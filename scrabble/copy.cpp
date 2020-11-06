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

    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);

    if (partial_word.size() == limit || partial_move.column == 0 || partial_move.row == 0) {
        return;
    }

    if (node == nullptr) {
        return;
    }

    // if(limit > 0){
    TileCollection rem_t = remaining_tiles;
    TileCollection::const_iterator it(remaining_tiles.cbegin());
    for (; it != remaining_tiles.cend(); it++) {
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column--;
        } else {
            partial_move.row--;
        }
        if (it->letter == TileKind::BLANK_LETTER) {
            rem_t.remove_tile(*it);
            std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator map_it;
            for (map_it = node->nexts.begin(); map_it != node->nexts.end(); map_it++) {
                partial_word.push_back(map_it->first);
                TileKind blank_letter = TileKind(TileKind::BLANK_LETTER, it->points, map_it->first);
                partial_move.tiles.push_back(blank_letter);
                left_part(
                        anchor_pos,
                        partial_word,
                        partial_move,
                        node->nexts[map_it->first],
                        limit--,
                        rem_t,
                        legal_moves,
                        board);
                partial_word.pop_back();
                partial_move.tiles.pop_back();
            }
            rem_t.add_tile(*it);
        } else if (node->nexts.find(it->letter) != node->nexts.end()) {
            partial_word.push_back(it->letter);
            rem_t.remove_tile(*it);
            partial_move.tiles.push_back(*it);
            left_part(
                    anchor_pos,
                    partial_word,
                    partial_move,
                    node->nexts[it->letter],
                    limit--,
                    rem_t,
                    legal_moves,
                    board);
            partial_word.pop_back();
            partial_move.tiles.pop_back();
            rem_t.add_tile(*it);
        }

        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column++;
        } else {
            partial_move.row++;
        }
    }
    // }
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

    if (node == nullptr) {
        return;
    }
    if (node->is_final) {
        legal_moves.push_back(partial_move);
    }

    if (!board.is_in_bounds(square) || remaining_tiles.count_tiles() == 0) {
        return;
    }

    if (partial_move.direction == Direction::ACROSS) {
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
                        square.column++;
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
                        square.column--;
                    }
                    rem_t.add_tile(*it);
                }

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
        } else if (board.in_bounds_and_has_tile(square)) {
            if (node->nexts.find(board.letter_at(square)) != node->nexts.end()) {
                char copy = board.letter_at(square);
                partial_word.push_back(board.letter_at(square));
                square.column++;
                extend_right(
                        square, partial_word, partial_move, node->nexts[copy], remaining_tiles, legal_moves, board);
            }
        }
    } else if (partial_move.direction == Direction::DOWN) {
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
    std::vector<Board::Anchor> anchors = board.get_anchors();
    // HW5: IMPLEMENT THIS
    for (unsigned int i = 0; i < anchors.size(); i++) {
        TileCollection rem_tiles;
        std::string partial;
        Move partial_m;
        for (size_t j = 0; j < player_tiles.size(); j++) {
            rem_tiles.add_tile(player_tiles[j]);
        }
        if (anchors[i].limit == 0) {
            Board::Position temp = anchors[i].position;
            partial_m.row = temp.row;
            partial_m.column = temp.column;
            if (anchors[i].direction == Direction::ACROSS) {
                partial_m.direction = Direction::ACROSS;
                temp.column--;
                while (board.in_bounds_and_has_tile(temp)) {
                    temp.column--;
                }
                temp.column++;
                while (temp != anchors[i].position) {
                    partial.push_back(board.letter_at(temp));
                    temp.column++;
                }
                extend_right(
                        anchors[i].position,
                        partial,
                        partial_m,
                        dictionary.find_prefix(partial),
                        rem_tiles,
                        legal_moves,
                        board);
            } else if (anchors[i].direction == Direction::DOWN) {
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
        } else if (anchors[i].limit > 0) {
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

    // how is dictionary used in this function??
    Move best_move = Move();
    if (legal_moves.empty()) {
        best_move.kind = MoveKind::PASS;
        return best_move;
    }
    // Pass if no move found
    // HW5: IMPLEMENT THIS
    size_t bonus = 7;
    unsigned int bonus_val = 50;
    int best_idx = 0;
    unsigned int points = 0;
    bool dictionary_check = false;
    for (size_t i = 0; i < legal_moves.size(); i++) {
        try {
            PlaceResult temp = board.test_place(legal_moves[i]);
            for (size_t j = 0; j < temp.words.size(); j++) {
                if (dictionary.is_word(temp.words[j])) {
                    dictionary_check = true;
                } else {
                    dictionary_check = false;
                    break;
                }
            }
            if (dictionary_check) {
                if (temp.points > points) {
                    best_idx = i;
                    points = temp.points;
                }
            }
        } catch (...) {
            // std::cout <<"Move Error" << std::endl;
        }
    }
    best_move = legal_moves[best_idx];
    best_move.kind = MoveKind::PLACE;

    return best_move;
}
