#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)),
          num_human_players(0) {}

// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {

    bool game_continue = true;
    while (game_continue) {
        // checks for consecutive passes that could end game
        int pass_count = 0;
        // loops until all the players have made their turns
        for (unsigned int i = 0; i < players.size(); i++) {
            cout << players[i]->get_name() << " time to make your move" << endl;
            cout << "Scoreboard:" << endl;
            for (unsigned int k = 0; k < players.size(); k++) {
                cout << players[k]->get_name() << ": " << SCORE_COLOR << players[k]->get_points() << rang::style::reset
                     << endl;
            }
            // reads the moves in get move which does all of the work that
            // deals with input entered by the user
            Move player_move = players[i]->get_move(board, dictionary);
            // keeps track of new tiles added
            vector<TileKind> tiles_added;
            // keeps track of new words formed
            vector<string> words_formed;
            if (player_move.kind == MoveKind::PASS) {
                if (players[i]->is_human()) {
                    pass_count++;
                }
            } else if (player_move.kind == MoveKind::EXCHANGE) {
                // when an exchange is made the tiles are first put back into the tile bag
                for (unsigned int i = 0; i < player_move.tiles.size(); i++) {
                    tile_bag.add_tile(player_move.tiles[i]);
                }
                // removes the tiles that were exchanged
                players[i]->remove_tiles(player_move.tiles);
                // removes random tiles from the tile bag
                // adds new tiles that are the same size of the tiles removed
                tiles_added = tile_bag.remove_random_tiles(player_move.tiles.size());
                players[i]->add_tiles(tiles_added);
                cout << "New Letters Picked Up: " << endl;
                for (unsigned int i = 0; i < tiles_added.size(); i++) {
                    cout << tiles_added[i].letter << " " << endl;
                }
            } else if (player_move.kind == MoveKind::PLACE) {
                // place result allows access to new words formed
                // and points gained by the player. also places the player move
                PlaceResult res = board.place(player_move);
                int bonus_points = 0;
                // player gets bonus points if they place all of
                // their tiles on the board
                if (player_move.tiles.size() == hand_size) {
                    cout << "You earned 50 bonus points!!" << endl;
                    bonus_points += 50;
                    players[i]->add_points(bonus_points);
                }
                // same logic as above with using and removing from the tile bag
                players[i]->remove_tiles(player_move.tiles);
                tiles_added = tile_bag.remove_random_tiles(player_move.tiles.size());
                players[i]->add_tiles(tiles_added);
                // adds points returned from place to the player
                players[i]->add_points(res.points);

                cout << "Words formed: " << endl;
                for (unsigned int i = 0; i < res.words.size(); i++) {
                    cout << res.words[i] << " ";
                }
                cout << endl;
                cout << "Points Gained: " << (res.points + bonus_points) << endl;
                cout << "New Letters Picked Up: " << endl;
                for (unsigned int i = 0; i < tiles_added.size(); i++) {
                    cout << tiles_added[i].letter << " " << endl;
                }
            }

            // ends game if a player has no more tiles to use or if all players
            // have passed their turn
            if ((pass_count == num_human_players && num_human_players >= 1) || players[i]->count_tiles() == 0) {
                game_continue = false;
                break;
            }
            cout << "Press enter to continue" << endl;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    // TODO: implement this.

    // Useful cout expressions with fancy colors. Expressions in curly braces, indicate values you supply.
    // cout << "You gained " << SCORE_COLOR << {points} << rang::style::reset << " points!" << endl;
    // cout << "Your current score: " << SCORE_COLOR << {points} << rang::style::reset << endl;
    // cout << endl << "Press [enter] to continue.";
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    int total_lost = 0;
    int winner_index;
    int all_tiles = false;
    // loop runs until all players have been accounted for
    for (unsigned int i = 0; i < plrs.size(); i++) {
        // if a player has no more tiles they are the winner
        if (plrs[i]->count_tiles() == 0) {
            all_tiles = true;
            winner_index = i;
        } else {
            // total lost keeps track of the collective hand tiles of players
            // that still have tiles in their hands
            total_lost += plrs[i]->get_hand_value();
            int play_points = plrs[i]->get_points();
            // performs subtraction arithmetic with an integer so no overflow error
            // occurs with an unsigned int
            play_points -= plrs[i]->get_hand_value();
            // since points cannot be negative players scores are set to zero
            // if their hand points exceed their total points
            if (play_points < 0) {
                plrs[i]->subtract_points(plrs[i]->get_points());
                continue;
            }

            plrs[i]->subtract_points(plrs[i]->get_hand_value());
        }
    }
    // if a player has no tiles left
    // they are the winner and score of total points lost from
    // other players is added to their score
    if (all_tiles) {
        plrs[winner_index]->add_points(total_lost);
    }
}

void Scrabble::add_players() {

    int player_amount = 0;
    bool amt = false;
    string num;
    string computer;
    cout << "How many players in this game?? Enter a number between 1 and 8" << endl;
    // try catch statement ensures that the user enters a correct player amount
    // or else they are prompted to enter another player_amount that is correct
    while (!amt) {
        try {
            getline(cin, num);
            stringstream ss(num);
            ss >> player_amount;
            if (player_amount < 1 || player_amount > 8) {
                throw CommandException("invalid player amount");
            }
            amt = true;
        } catch (...) {
            cout << "Please enter a number between 1 and 8" << endl;
        }
    }
    // loop runs to add the correct amount of players to the game
    for (int i = 0; i < player_amount; i++) {
        string name;
        // make sure names can have whitespace
        cout << "Enter a name" << endl;
        getline(cin, name);
        cout << "Is this player a computer, y or n?" << endl;
        getline(cin, computer);
        // creates the new player
        if (computer == "y") {
            std::shared_ptr<ComputerPlayer> cp = make_shared<ComputerPlayer>(name, hand_size);
            players.push_back(cp);
        } else {
            std::shared_ptr<HumanPlayer> p2 = make_shared<HumanPlayer>(name, hand_size);
            players.push_back(p2);
            num_human_players++;
        }
        cout << name << " has been added to the game!" << endl;
    }
    // gives the players their initial tiles to start the game
    for (int i = 0; i < player_amount; i++) {
        vector<TileKind> give = tile_bag.remove_random_tiles(hand_size);
        players[i]->add_tiles(give);
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
