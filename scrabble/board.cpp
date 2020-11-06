#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}
// if the tile is blank, gets the assigned value
// otherwise the letter value is returned
char Board::letter_at(Position p) const {
    if (this->at(p).get_tile_kind().letter == TileKind::BLANK_LETTER) {
        return this->at(p).get_tile_kind().assigned;
    }
    return this->at(p).get_tile_kind().letter;
}

bool Board::is_anchor_spot(Position p) const {
    // only anchor spot on empty board is the start position
    if (!this->at(start).has_tile() && p == start) {
        return true;
    }
    // places anchor spot at adjacent positions
    if (this->is_in_bounds(p) && !this->at(p).has_tile()) {
        Position left = Position(p.row, p.column - 1);
        Position right = Position(p.row, p.column + 1);
        Position up = Position(p.row - 1, p.column);
        Position down = Position(p.row + 1, p.column);
        if (this->in_bounds_and_has_tile(left)) {
            return true;
        } else if (this->in_bounds_and_has_tile(right)) {
            return true;
        } else if (this->in_bounds_and_has_tile(up)) {
            return true;
        } else if (this->in_bounds_and_has_tile(down)) {
            return true;
        }
        // returns false if nothing above returns true
        else {
            return false;
        }
    }
    return false;
}

vector<Board::Anchor> Board::get_anchors() const {
    vector<Anchor> anchors;
    Direction a = Direction::ACROSS;
    Direction d = Direction::DOWN;
    // iterates through rows and columns of the board starting at first position
    for (size_t i = 0; i < this->rows; i++) {
        for (size_t j = 0; j < this->columns; j++) {
            Position temp = Position(i, j);
            size_t limit = 0;
            // sets the limit value for an anchor spot by iterating left while it is in bounds
            // and it doesnt reach another anchor spot, and there it doesnt reach a tile
            if (this->is_anchor_spot(temp)) {
                Position left = Position(i, j - 1);
                while (this->is_in_bounds(left) && !this->is_anchor_spot(left) && !this->at(left).has_tile()) {
                    left.column--;
                    limit++;
                }
                // initializes the across anchor for a given position
                Anchor across = Anchor(temp, a, limit);
                anchors.push_back(across);
                limit = 0;
                Position up = Position(i - 1, j);
                // same as above but sets the down anchor
                while (this->is_in_bounds(up) && !this->is_anchor_spot(up) && !this->at(up).has_tile()) {
                    up.row--;
                    limit++;
                }
                // pushes back down anchor into anchor vector
                Anchor down = Anchor(temp, d, limit);  // Go Commodores
                anchors.push_back(down);
            }
        }
    }
    return anchors;
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // TODO: complete implementation of reading in board from file here.

    // initializes board squares for square possibilities
    BoardSquare t_word(1, 3);
    BoardSquare d_word(1, 2);
    BoardSquare t_let(3, 1);
    BoardSquare d_let(2, 1);
    BoardSquare norm(1, 1);
    int count = 0;

    // initializes a temp vector of boardsquares for the rows
    // these vectors get pushed back into the squares vector of vectors
    // as the strings of the board rows are read in the temp vector pushes
    // back the respective board squares
    for (size_t i = 0; i < rows; i++) {
        count++;
        vector<BoardSquare> temp;
        string reader;
        file >> reader;
        for (size_t j = 0; j < columns; j++) {
            if (reader[j] == 't') {
                temp.push_back(t_word);
            } else if (reader[j] == '.') {
                temp.push_back(norm);
            } else if (reader[j] == '2') {
                temp.push_back(d_let);
            } else if (reader[j] == '3') {
                temp.push_back(t_let);
            } else if (reader[j] == 'd') {
                temp.push_back(d_word);
            }
        }
        board.squares.push_back(temp);
    }

    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    unsigned int word_mult = 1;
    string error = "error with placement";
    // holds all of the new words formed by a user placement
    vector<string> full_words;
    int range = move.tiles.size();
    bool adjacent = false;
    bool start_pos = false;

    Position current = Position(move.row, move.column);
    // checks if the user is placing on a square that
    // already has a tile
    if (this->at(current).has_tile()) {
        return PlaceResult(error);
    }

    if (move.direction == Direction::ACROSS) {
        for (int i = 0; i < range; i++) {
            Position temp = Position(move.row, move.column + i);

            Position left = Position(temp.row, temp.column - 1);
            Position right = Position(temp.row, temp.column + 1);
            Position up = Position(temp.row - 1, temp.column);
            Position down = Position(temp.row + 1, temp.column);

            // if the square already has a tile, the loop terminating
            // condition needs to be incremented
            if (this->in_bounds_and_has_tile(temp)) {
                range++;
                continue;
            }
            // returns an error if the current square
            // being checked is out of bounds
            if (!this->is_in_bounds(temp)) {
                return PlaceResult(error);
            }
            // checks if the start tile has been placed yet
            if (!this->at(start).has_tile()) {
                if (temp == this->start) {
                    start_pos = true;
                }
            }
            // For tiles that are about to be placed, the word
            // multiplier is set for later use
            if (!this->at(temp).has_tile()) {
                word_mult *= this->at(temp).word_multiplier;
            }

            // The next 4 if statements check for adjacency. This
            // makes sure that placements on the board adhere to
            // the scrabble placement rules
            if (this->in_bounds_and_has_tile(left)) {
                adjacent = true;
            }

            if (this->in_bounds_and_has_tile(right)) {
                adjacent = true;
            }

            if (this->in_bounds_and_has_tile(up)) {
                adjacent = true;
            }
            if (this->in_bounds_and_has_tile(down)) {
                adjacent = true;
            }
        }
        // if the placement fails the adjacency check when
        // a start tile exists, an error is thrown
        if (!adjacent && this->at(start).has_tile()) {
            // cout << "getting in this one" << endl;
            return PlaceResult(error);
        }

        // If there is no start tile, but nothing went over the start position,
        // an error is thrown
        if (!this->at(start).has_tile() && !start_pos) {
            // cout << "nah getting in this one" << endl;
            return PlaceResult(error);
        }

        Position current = Position(move.row, move.column - 1);
        // points for overall points from all words formed
        unsigned int points = 0;
        // across points for points just from word by the user
        unsigned int across_points = 0;
        string word;
        // this if statement checks if the tile being placed
        // appended a previous word
        if (this->in_bounds_and_has_tile(current)) {
            // loop increments to get back to the start
            // of the original word
            while (this->in_bounds_and_has_tile(current)) {
                current.column--;
            }
            current.column += 1;
            // loop increments until the user-defined starting position
            // is reached
            while (current != Position(move.row, move.column)) {
                // makes sure the the string pushes back the assigned character for
                // blank tiles. Increments column to move up a spot on the board
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    across_points += this->at(current).get_tile_kind().points;
                    current.column++;
                    continue;
                }
                // if not a blank tile, the letter is pushed back
                word.push_back(this->at(current).get_tile_kind().letter);
                across_points += this->at(current).get_tile_kind().points;
                current.column++;
            }
        }
        range = move.tiles.size();
        current = Position(move.row, move.column);
        int tile_pos = 0;
        for (int i = 0; i < range; i++) {
            int letter_mult = 1;
            // checks if the boardsquare already has a tile
            if (this->in_bounds_and_has_tile(current)) {
                // for blank tiles the assigned value is pushed back instead of letter
                // range is incremented since none of the user tiles have been checked yet4
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    across_points += this->at(current).get_tile_kind().points;
                    current.column++;
                    range++;
                    continue;
                }
                // if its not a blank tile, the letter is pushed back
                word.push_back(this->at(current).get_tile_kind().letter);
                across_points += this->at(current).get_tile_kind().points;
                range++;
                current.column++;
                continue;
            }
            // if a user tile is a blank tile, the same checks are applied
            // since its a user tile, the letter multiplier is used
            // assigned value pushed back
            // tile_pos incremented after a user tile is checked
            if (move.tiles[tile_pos].letter == TileKind::BLANK_LETTER) {
                letter_mult *= this->at(current).letter_multiplier;
                across_points += (move.tiles[tile_pos].points * letter_mult);
                word.push_back(move.tiles[tile_pos].assigned);
                current.column++;
                tile_pos++;
                continue;
            }
            // multiplies letter by the multipler when its greater than 1
            // pushes back the regular letter variable
            if (this->at(current).letter_multiplier > 1) {
                letter_mult *= this->at(current).letter_multiplier;
                across_points += (move.tiles[tile_pos].points * letter_mult);
                word.push_back(move.tiles[tile_pos].letter);
                current.column++;
                tile_pos++;
                continue;
            }
            // same as above without multiplier
            word.push_back(move.tiles[tile_pos].letter);
            across_points += move.tiles[tile_pos].points;
            current.column++;
            tile_pos++;
        }

        // checks if the users word preceded existing tiles
        // follows the same logic as above when checking tiles
        // that have already been placed.
        if (this->in_bounds_and_has_tile(current)) {
            while (this->in_bounds_and_has_tile(current)) {
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    across_points += this->at(current).get_tile_kind().points;
                    current.column++;
                    continue;
                }
                word.push_back(this->at(current).get_tile_kind().letter);
                across_points += this->at(current).get_tile_kind().points;
                current.column++;
            }
        }
        // multiplies the full across words, placed tiles included,
        // and multiplies it by the word multipler defined at the beginning
        across_points *= word_mult;
        // word created should only be pushed back if its more than
        // one letter
        if (word.size() > 1) {
            full_words.push_back(word);
        } else {
            across_points = 0;
        }
        current = Position(move.row, move.column);
        int it = 0;
        int cur_pos = 0;
        // resets range value
        range = move.tiles.size();

        // loop checks for new words created above and below user
        // defined tiles
        while (it < range) {
            int extra_points = 0;
            current = Position(move.row, move.column + it);
            // skips over tiles that already exist
            if (this->at(current).has_tile()) {
                it++;
                range++;
                continue;
            }
            Position above = Position(current.row - 1, current.column);
            Position below = Position(current.row + 1, current.column);
            string vertical = "";
            int extra_word = this->at(current).word_multiplier;
            // checks if there is a tile above a user tile
            if (this->in_bounds_and_has_tile(above)) {
                while (this->in_bounds_and_has_tile(above)) {
                    above.row--;
                }
                above.row++;
                // applies the same logic as above when pushing back
                // tiles that already exist
                while (above.row != current.row) {
                    if (this->at(above).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                        vertical.push_back(this->at(above).get_tile_kind().assigned);
                        extra_points += this->at(above).get_tile_kind().points;
                        above.row++;
                        continue;
                    }
                    vertical.push_back(this->at(above).get_tile_kind().letter);
                    extra_points += this->at(above).get_tile_kind().points;
                    above.row++;
                }
                // applies same logic when dealing with user-defined tiles
                // pushes back letters into a new string called vertical
                if (move.tiles[cur_pos].letter == TileKind::BLANK_LETTER) {
                    vertical.push_back(move.tiles[cur_pos].assigned);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    extra_points *= extra_word;
                    points += extra_points;
                    cur_pos++;
                    full_words.push_back(vertical);
                    it++;
                    continue;
                }
                vertical.push_back(move.tiles[cur_pos].letter);
                extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                cur_pos++;
                full_words.push_back(vertical);
                extra_points *= extra_word;
                points += extra_points;
                it++;
                continue;
            }
            // does the same thing as the above check but for tiles that
            // exist below the user defined tiles
            // its the same except the user tiles are pushed back first before
            // going down further
            if (this->in_bounds_and_has_tile(below)) {
                if (move.tiles[cur_pos].letter == TileKind::BLANK_LETTER) {
                    vertical.push_back(move.tiles[cur_pos].assigned);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    cur_pos++;
                } else {
                    vertical.push_back(move.tiles[cur_pos].letter);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    cur_pos++;
                }
                while (this->in_bounds_and_has_tile(below)) {
                    if (this->at(below).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                        vertical.push_back(this->at(below).get_tile_kind().assigned);
                        extra_points += this->at(below).get_tile_kind().points;
                        below.row++;
                        continue;
                    }
                    vertical.push_back(this->at(below).get_tile_kind().letter);
                    extra_points += this->at(below).get_tile_kind().points;
                    below.row++;
                }
                extra_points *= extra_word;
                points += extra_points;
                full_words.push_back(vertical);
                it++;

            } else {
                it++;
                cur_pos++;
            }
        }
        // adds across points and returns vector of strings
        // created from words that were formed
        points += across_points;
        return PlaceResult(full_words, points);

    }
    // follows the exact same logic as when the user places a word across
    // changes the incrementation of rows and columns in various spots to account
    // for the fact that the direction is DOWN
    else if (move.direction == Direction::DOWN) {
        for (int i = 0; i < range; i++) {
            Position temp = Position(move.row + i, move.column);

            Position left = Position(temp.row, temp.column - 1);
            Position right = Position(temp.row, temp.column + 1);
            Position up = Position(temp.row - 1, temp.column);
            Position down = Position(temp.row + 1, temp.column);

            if (!this->is_in_bounds(temp)) {
                return PlaceResult(error);
            }

            if (!this->at(start).has_tile()) {
                if (temp == this->start) {
                    start_pos = true;
                }
            }

            if (!this->at(temp).has_tile()) {
                word_mult *= this->at(temp).word_multiplier;
            }

            if (this->in_bounds_and_has_tile(left)) {
                adjacent = true;
            }
            if (this->in_bounds_and_has_tile(right)) {
                adjacent = true;
            }

            if (this->in_bounds_and_has_tile(up)) {
                adjacent = true;
            }

            if (this->in_bounds_and_has_tile(down)) {
                adjacent = true;
            }
            if (this->in_bounds_and_has_tile(temp)) {
                range++;
                continue;
            }
        }
        if (!adjacent && this->at(start).has_tile()) {
            return PlaceResult(error);
        }
        if (!this->at(start).has_tile() && !start_pos) {
            return PlaceResult(error);
        }

        Position current = Position(move.row - 1, move.column);
        unsigned int points = 0;
        unsigned int down_points = 0;
        string word;
        if (this->in_bounds_and_has_tile(current)) {
            while (this->in_bounds_and_has_tile(current)) {
                current.row--;
            }
            current.row += 1;
            while (current != Position(move.row, move.column)) {
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    down_points += this->at(current).get_tile_kind().points;
                    current.row++;
                    continue;
                }
                word.push_back(this->at(current).get_tile_kind().letter);
                down_points += this->at(current).get_tile_kind().points;
                current.row++;
            }
        }
        range = move.tiles.size();
        current = Position(move.row, move.column);

        int tile_pos = 0;
        for (int i = 0; i < range; i++) {
            int letter_mult = 1;
            if (this->in_bounds_and_has_tile(current)) {
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    down_points += this->at(current).get_tile_kind().points;
                    current.row++;
                    range++;
                    continue;
                }
                word.push_back(this->at(current).get_tile_kind().letter);
                down_points += this->at(current).get_tile_kind().points;
                range++;
                current.row++;
                continue;
            }
            if (move.tiles[tile_pos].letter == TileKind::BLANK_LETTER) {
                word.push_back(move.tiles[tile_pos].assigned);
                letter_mult *= this->at(current).letter_multiplier;
                down_points += (move.tiles[tile_pos].points * letter_mult);
                current.row++;
                tile_pos++;
                continue;
            }
            if (this->at(current).letter_multiplier > 1) {
                letter_mult *= this->at(current).letter_multiplier;
                down_points += (move.tiles[tile_pos].points * letter_mult);
                word.push_back(move.tiles[tile_pos].letter);
                current.row++;
                tile_pos++;
                continue;
            }
            word.push_back(move.tiles[tile_pos].letter);
            down_points += move.tiles[tile_pos].points;
            current.row++;
            tile_pos++;
        }
        if (this->in_bounds_and_has_tile(current)) {
            while (this->in_bounds_and_has_tile(current)) {
                if (this->at(current).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                    word.push_back(this->at(current).get_tile_kind().assigned);
                    down_points += this->at(current).get_tile_kind().points;
                    current.row++;
                    continue;
                }
                word.push_back(this->at(current).get_tile_kind().letter);
                down_points += this->at(current).get_tile_kind().points;
                current.row++;
            }
        }
        down_points *= word_mult;
        if (word.size() > 1) {
            full_words.push_back(word);
        } else {
            down_points = 0;
        }
        current = Position(move.row, move.column);
        int it = 0;
        int cur_pos = 0;
        range = move.tiles.size();
        while (it < range) {
            int extra_points = 0;
            current = Position(move.row + it, move.column);
            if (this->at(current).has_tile()) {
                it++;
                range++;
                continue;
            }
            Position left = Position(current.row, current.column - 1);
            Position right = Position(current.row, current.column + 1);
            string horizontal = "";
            int extra_word = this->at(current).word_multiplier;
            if (this->in_bounds_and_has_tile(left)) {
                while (this->in_bounds_and_has_tile(left)) {
                    left.column--;
                }
                left.column++;
                while (left.column != current.column) {
                    if (this->at(left).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                        horizontal.push_back(this->at(left).get_tile_kind().assigned);
                        extra_points += this->at(left).get_tile_kind().points;
                        left.column++;
                        continue;
                    }
                    horizontal.push_back(this->at(left).get_tile_kind().letter);
                    extra_points += this->at(left).get_tile_kind().points;
                    left.column++;
                }
                if (move.tiles[cur_pos].letter == TileKind::BLANK_LETTER) {
                    horizontal.push_back(move.tiles[cur_pos].assigned);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    extra_points *= extra_word;
                    points += extra_points;
                    cur_pos++;
                    full_words.push_back(horizontal);
                    it++;
                    continue;
                }
                horizontal.push_back(move.tiles[cur_pos].letter);
                extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                extra_points *= extra_word;
                points += extra_points;
                cur_pos++;
                full_words.push_back(horizontal);
                it++;
                continue;
            }
            if (this->in_bounds_and_has_tile(right)) {
                if (move.tiles[cur_pos].letter == TileKind::BLANK_LETTER) {
                    horizontal.push_back(move.tiles[cur_pos].assigned);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    cur_pos++;
                } else {
                    horizontal.push_back(move.tiles[cur_pos].letter);
                    extra_points += (move.tiles[cur_pos].points * this->at(current).letter_multiplier);
                    cur_pos++;
                }
                while (this->in_bounds_and_has_tile(right)) {
                    if (this->at(right).get_tile_kind().letter == TileKind::BLANK_LETTER) {
                        horizontal.push_back(this->at(right).get_tile_kind().assigned);
                        extra_points += this->at(right).get_tile_kind().points;
                        right.column++;
                        continue;
                    }
                    horizontal.push_back(this->at(right).get_tile_kind().letter);
                    extra_points += this->at(right).get_tile_kind().points;
                    right.column++;
                }
                extra_points *= extra_word;
                points += extra_points;
                full_words.push_back(horizontal);
                it++;
            } else {
                it++;
                cur_pos++;
            }
        }
        points += down_points;
        return PlaceResult(full_words, points);
    }
    return PlaceResult(error);
}

PlaceResult Board::place(const Move& move) {

    // calls test place to obtain the correct vector and points
    // to return
    PlaceResult result = test_place(move);

    int range = move.tiles.size();
    int tile_pos = 0;
    for (int i = 0; i < range; i++) {
        if (move.direction == Direction::ACROSS) {
            Position current = Position(move.row, move.column + i);
            // makes sure that word being placed is not out of bounds
            if (!this->is_in_bounds(current)) {
                return PlaceResult("out of bounds");
            }
            // skips over squares that already have placed tiles
            if (this->at(current).has_tile()) {
                range++;
                continue;
            }
            // sets tiles on the board
            this->at(current).set_tile_kind(move.tiles[tile_pos]);
            tile_pos++;
        }
        // same logic as above but for placements that
        // go down
        else if (move.direction == Direction::DOWN) {
            Position current = Position(move.row + i, move.column);
            if (!this->is_in_bounds(current)) {
                return PlaceResult("out of bounds");
            }
            if (this->at(current).has_tile()) {
                range++;
                continue;
            }

            this->at(current).set_tile_kind(move.tiles[tile_pos]);
            tile_pos++;
        }
    }
    // returns result from test place
    return result;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}
