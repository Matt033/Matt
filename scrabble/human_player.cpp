#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

bool HumanPlayer::is_human() const { return true; }

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {

    // while loop for try catch statement
    // loops until valid input is entered, and a valid
    // placement can be made on the board
    while (1) {
        try {
            cout << "Here is the state of the board" << endl;
            board.print(cout);
            print_hand(cout);
            string move_s;
            cout << "Enter your Move" << endl;
            // input received into string move_s
            getline(cin, move_s);

            // player_move equal to value returned from parse_move
            Move player_move = parse_move(move_s);

            if (player_move.kind == MoveKind::PLACE) {

                // tests word placement in test_place
                // throws an exception if invalid and user enters new input
                PlaceResult test = board.test_place(player_move);
                if (!test.valid) {
                    throw MoveException("invalid move");
                }
                // with the vector of new words returned from test place,
                // each word is checked against the dictionary to make sure
                // that the words formed are valid
                for (unsigned int i = 0; i < test.words.size(); i++) {
                    if (!dictionary.is_word(test.words[i])) {
                        throw CommandException("word not in dictionary");
                    }
                }
            }

            return player_move;
        }

        catch (...) {
            cout << "error with placement, try again" << endl;
        }
    }
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters, MoveKind kind) const {
    vector<TileKind> play_tiles;
    bool match = false;
    set<int> used_spots;

    for (unsigned int i = 0; i < letters.size(); i++) {
        for (unsigned int j = 0; j < player_tiles.size(); j++) {
            // if the user wants to place a blank tile on the board
            // a new tile kind is created with the correct letter,
            // point value and assigned value
            if (letters[i] == player_tiles[j].BLANK_LETTER && kind == MoveKind::PLACE) {
                TileKind temp = player_tiles[j];
                temp.letter = TileKind::BLANK_LETTER;
                temp.points = 0;
                temp.assigned = letters[i + 1];
                play_tiles.push_back(temp);
                i++;
                match = true;
                break;
            }
            // makes sure the the user-entered letters
            // are equal to a letter they have in their hand
            if (letters[i] == player_tiles[j].letter) {
                // the set stores indexes of player tiles that have already been checked
                // ensures that an index that has already been checked doesnt get
                // checked again
                if (used_spots.find(j) != used_spots.end()) {
                    match = false;
                    continue;
                }
                // inserts a used index
                used_spots.insert(j);
                // pushes back full tilekind to be used in move
                play_tiles.push_back(player_tiles[j]);
                match = true;
                break;
            } else {
                match = false;
            }
        }
        // throws an exception if the above checks dont pass
        // makes sure that user has tiles that they enter and
        // also makes sure that any given player_tile is not used multiple times
        // during equality checks
        if (!match) {
            throw CommandException("play tiles don't match hand tiles");
        }
    }
    // returns the vector of tilekinds to be used in the move
    return play_tiles;
}

Move HumanPlayer::parse_move(string& move_string) const {
    Move player_move;
    // ss parses through user input
    stringstream ss(move_string);
    string kind;
    ss >> kind;

    // passes dont have tiles so nothing else needs
    // to be parsed
    if (kind == "PASS" || kind == "pass") {
        player_move.kind = MoveKind::PASS;
        return player_move;
    }

    else if (kind == "EXCHANGE" || kind == "exchange") {
        player_move.kind = MoveKind::EXCHANGE;
        string tiles;
        ss >> tiles;
        // makes the input tiles lowercase to avoid issues when checking
        // against player_tile letters
        for (unsigned int i = 0; i < tiles.size(); i++) {
            tiles[i] = tolower(tiles[i]);
        }
        // tiles are parsed separately
        player_move.tiles = parse_tiles(tiles, player_move.kind);
    } else if (kind == "PLACE" || kind == "place") {
        char direction;
        player_move.kind = MoveKind::PLACE;
        size_t row, column;
        string tiles;
        // reads in all of the info necessary for a move
        ss >> direction >> row >> column >> tiles;
        // checks for down or across directions
        if (direction == '-') {
            player_move.direction = Direction::ACROSS;
        } else if (direction == '|') {
            player_move.direction = Direction::DOWN;
        }
        // decrements rows and columns to revert back
        // to zero-based indexing
        player_move.row = row - 1;
        player_move.column = column - 1;

        // changes user input letters back to lowercase
        // to avoid issues when checking against player tiles
        for (unsigned int i = 0; i < tiles.size(); i++) {
            tiles[i] = tolower(tiles[i]);
        }

        player_move.tiles = parse_tiles(tiles, player_move.kind);
    }
    return player_move;
}

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = this->count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = player_tiles.cbegin(); it != player_tiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (this->count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
