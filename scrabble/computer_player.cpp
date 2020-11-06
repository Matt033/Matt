#include "computer_player.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>

bool ComputerPlayer::is_human() const { return false; }
void ComputerPlayer::left_part(
        Board::Position anchor_pos,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        size_t limit,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {

    // HW5: IMPLEMENT THIS

    // calls extend right with every call to left part
    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);

    // if the word formed is equal to the size of the limit then
    // the function returns
    if (partial_word.size() == limit || partial_word.size() == this->get_hand_size() - 1) {
        return;
    }
    // base case if the dictionary node is null
    if (node == nullptr) {
        return;
    }

    // makes a copy of remaining tiles
    TileCollection rem_t = remaining_tiles;
    TileCollection::const_iterator it(remaining_tiles.cbegin());
    // iterator goes through remaining tiles and loops until
    // it reaches the end
    for (; it != remaining_tiles.cend(); it++) {
        // updates the row and column depending on direction
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column--;
        } else {
            partial_move.row--;
        }

        if (it->letter == TileKind::BLANK_LETTER) {
            rem_t.remove_tile(*it);
            // creates an iterator to the dictionary since a blank tile can be any letter
            std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator map_it;
            for (map_it = node->nexts.begin(); map_it != node->nexts.end(); map_it++) {
                partial_word.push_back(map_it->first);
                // creates and initializes a tilekind blank letter to push back into move
                TileKind blank_letter = TileKind(TileKind::BLANK_LETTER, it->points, map_it->first);
                partial_move.tiles.push_back(blank_letter);
                // calls left part with updated word, move, and dictionary node
                left_part(
                        anchor_pos,
                        partial_word,
                        partial_move,
                        node->nexts[map_it->first],
                        limit,
                        rem_t,
                        legal_moves,
                        board);
                // undoes previous additions after function returns
                partial_word.pop_back();
                partial_move.tiles.pop_back();
            }
            rem_t.add_tile(*it);
        }
        // does the same as above but in the case that the tile is not blank
        else if (node->nexts.find(it->letter) != node->nexts.end()) {
            partial_word.push_back(it->letter);
            rem_t.remove_tile(*it);
            partial_move.tiles.push_back(*it);
            // updates the dictionary node with the current letter
            left_part(
                    anchor_pos, partial_word, partial_move, node->nexts[it->letter], limit, rem_t, legal_moves, board);
            partial_word.pop_back();
            partial_move.tiles.pop_back();
            rem_t.add_tile(*it);
        }

        // undoes the partial_move row and column change
        // from the beginning of the loop
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column++;
        } else {
            partial_move.row++;
        }
    }
}

void ComputerPlayer::extend_right(
        Board::Position square,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {
    // HW5: IMPLEMENT THIS

    // base case that returns when node is null
    if (node == nullptr) {
        return;
    }
    // if the current node is a real word, this pushes the move into legal moves
    if (node->is_final) {
        legal_moves.push_back(partial_move);
    }
    // returns if the current square is out of bounds
    if (!board.is_in_bounds(square)) {
        return;
    }
    if(remaining_tiles.count_tiles() == 0 && !board.in_bounds_and_has_tile(square)){
        return;
    }

    if (partial_move.direction == Direction::ACROSS) {
        // if the current spot on the board is vacant
        if (!board.in_bounds_and_has_tile(square)) {
            TileCollection rem_t = remaining_tiles;
            // creates an iterator to the remaining tiles and loops through until
            // the iterator reaches the end
            TileCollection::const_iterator it(remaining_tiles.cbegin());
            for (; it != remaining_tiles.cend(); ++it) {
                // same logic as checking for a blank tile in left part
                if (it->letter == TileKind::BLANK_LETTER) {
                    std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator map_it;
                    rem_t.remove_tile(*it);
                    for (map_it = node->nexts.begin(); map_it != node->nexts.end(); map_it++) {
                        partial_word.push_back(map_it->first);
                        TileKind blank_letter = TileKind(TileKind::BLANK_LETTER, it->points, map_it->first);
                        partial_move.tiles.push_back(blank_letter);
                        // increments the square column when going across to move to the next position
                        square.column++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[map_it->first],
                                rem_t,
                                legal_moves,
                                board);
                        // undoes previous changes when this call to extend right returns to try new values
                        partial_word.pop_back();
                        partial_move.tiles.pop_back();
                        square.column--;
                    }
                    rem_t.add_tile(*it);
                }
                // same logic as left part when the current tile is not a blank tile
                // and the letter is found in the nexts node of dictionary
                else if (node->nexts.find(it->letter) != node->nexts.end()) {
                    partial_word.push_back(it->letter);
                    rem_t.remove_tile(*it);
                    partial_move.tiles.push_back(*it);
                    square.column++;
                    extend_right(
                            square, partial_word, partial_move, node->nexts[it->letter], rem_t, legal_moves, board);
                    partial_move.tiles.pop_back();
                    partial_word.pop_back();
                    square.column--;
                    rem_t.add_tile(*it);
                }
            }
        }
        // goes in here when the square is not vacant
        else if (board.in_bounds_and_has_tile(square)) {
            // checks if the square is found in the nexts of current dictionary node
            if (node->nexts.find(board.letter_at(square)) != node->nexts.end()) {
                // copy of current letter at
                char copy = board.letter_at(square);
                partial_word.push_back(board.letter_at(square));
                square.column++;
                extend_right(
                        square, partial_word, partial_move, node->nexts[copy], remaining_tiles, legal_moves, board);
                // does not have to undo since the letter on the board would have to become apart of the move
                // if placed at the current move position
            }
        }
    }
    // same logic as across except when the move direction is down
    else if (partial_move.direction == Direction::DOWN) {
        if (!board.in_bounds_and_has_tile(square)) {
            TileCollection rem_t = remaining_tiles;
            TileCollection::const_iterator it(remaining_tiles.cbegin());
            for (; it != remaining_tiles.cend(); ++it) {
                if (it->letter == TileKind::BLANK_LETTER) {
                    std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator map_it;
                    rem_t.remove_tile(*it);
                    for (map_it = node->nexts.begin(); map_it != node->nexts.end(); map_it++) {
                        partial_word.push_back(map_it->first);
                        TileKind blank_letter = TileKind(TileKind::BLANK_LETTER, it->points, map_it->first);
                        partial_move.tiles.push_back(blank_letter);
                        square.row++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[map_it->first],
                                rem_t,
                                legal_moves,
                                board);
                        partial_word.pop_back();
                        partial_move.tiles.pop_back();
                        square.row--;
                    }
                    rem_t.add_tile(*it);
                } else if (node->nexts.find(it->letter) != node->nexts.end()) {
                    partial_word.push_back(it->letter);
                    rem_t.remove_tile(*it);
                    partial_move.tiles.push_back(*it);
                    square.row++;
                    extend_right(
                            square, partial_word, partial_move, node->nexts[it->letter], rem_t, legal_moves, board);
                    partial_move.tiles.pop_back();
                    partial_word.pop_back();
                    square.row--;
                    rem_t.add_tile(*it);
                }
            }
        } else if (board.in_bounds_and_has_tile(square)) {
            if (node->nexts.find(board.letter_at(square)) != node->nexts.end()) {
                char copy = board.letter_at(square);
                partial_word.push_back(board.letter_at(square));
                square.row++;
                extend_right(
                        square, partial_word, partial_move, node->nexts[copy], remaining_tiles, legal_moves, board);
            }
        }
    }
}

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    std::vector<Move> legal_moves;
    // updates anchors positions with each call to get move
    std::vector<Board::Anchor> anchors = board.get_anchors();
    // HW5: IMPLEMENT THIS
    // loops through all of the anchor positions
    for (unsigned int i = 0; i < anchors.size(); i++) {
        TileCollection rem_tiles;
        std::string partial;
        Move partial_m;
        // adds the players current tiles into a new tile collection
        for (size_t j = 0; j < player_tiles.size(); j++) {
            rem_tiles.add_tile(player_tiles[j]);
        }
        if (anchors[i].limit == 0) {
            Board::Position temp = anchors[i].position;
            // initializes the partial move row and column
            partial_m.row = temp.row;
            partial_m.column = temp.column;
            // sets direction and gets words based on direction
            if (anchors[i].direction == Direction::ACROSS) {
                partial_m.direction = Direction::ACROSS;
                temp.column--;
                // goes until the end of the word is reached
                while (board.in_bounds_and_has_tile(temp)) {
                    temp.column--;
                }
                temp.column++;
                // pushes back current words until back at anchor position
                while (temp != anchors[i].position) {
                    partial.push_back(board.letter_at(temp));
                    temp.column++;
                }
                // calls extend right with the updated values
                extend_right(
                        anchors[i].position,
                        partial,
                        partial_m,
                        dictionary.find_prefix(partial),
                        rem_tiles,
                        legal_moves,
                        board);
            }
            // same logic as above but with a direction going down
            else if (anchors[i].direction == Direction::DOWN) {
                Board::Position temp = anchors[i].position;
                partial_m.direction = Direction::DOWN;
                temp.row--;
                while (board.in_bounds_and_has_tile(temp)) {
                    temp.row--;
                }
                temp.row++;
                while (temp != anchors[i].position) {
                    partial.push_back(board.letter_at(temp));
                    temp.row++;
                }
                extend_right(
                        anchors[i].position,
                        partial,
                        partial_m,
                        dictionary.find_prefix(partial),
                        rem_tiles,
                        legal_moves,
                        board);
            }
        }
        // calls left part with an empty string and partial move initialized with
        // a row and column and direction
        else if (anchors[i].limit > 0) {
            Move partial_move;
            partial_move.direction = anchors[i].direction;
            partial_move.row = anchors[i].position.row;
            partial_move.column = anchors[i].position.column;
            left_part(
                    anchors[i].position,
                    "",
                    partial_move,
                    dictionary.get_root(),
                    anchors[i].limit,
                    rem_tiles,
                    legal_moves,
                    board);
        }
    }

    return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(
        std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {

    Move best_move = Move();
    // computer passes if there are no legal moves
    if (legal_moves.empty()) {
        best_move.kind = MoveKind::PASS;
        return best_move;
    }
    // Pass if no move found
    // HW5: IMPLEMENT THIS
    int best_idx = 0;
    unsigned int points = 0;
    bool dictionary_check = false;
    // loops through all of the legal moves
    for (size_t i = 0; i < legal_moves.size(); i++) {
        try {
            // calls test place to get points and words formed for each move
            PlaceResult temp = board.test_place(legal_moves[i]);
            // loops through all of the words created by a move and makes
            // sure all of those are valid words in the dictionary
            for (size_t j = 0; j < temp.words.size(); j++) {
                if (dictionary.is_word(temp.words[j])) {
                    dictionary_check = true;
                } else {
                    dictionary_check = false;
                    break;
                }
            }
            if (dictionary_check) {
                // if the legal move places all tiles in hand, the move
                // with the bonus is checked against the current highest point value
                if (legal_moves[i].tiles.size() == get_hand_size()) {
                    if (temp.points + 50 > points) {
                        best_idx = i;
                        points = temp.points + 50;
                    }
                }
                // if the points returned by test place is higher than the current highest point
                // value, the best index and highest point value is updated
                else if (temp.points > points) {
                    best_idx = i;
                    points = temp.points;
                }
            }
        } catch (...) {
        }
    }
    // uses best idx to get best index in legal moves
    best_move = legal_moves[best_idx];
    best_move.kind = MoveKind::PLACE;

    return best_move;
}
