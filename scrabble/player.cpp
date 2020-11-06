#include "player.h"

#include "exceptions.h"
#include <iostream>
#include <set>

using namespace std;

// Note to self: finish destructor and virtual move function??
// TODO: implement member functions

// Adds points to player's score
void Player::add_points(size_t points) { this->points += points; }

// Subtracts points from player's score
void Player::subtract_points(size_t points) { this->points -= points; }

// returns player points
size_t Player::get_points() const { return this->points; }

// returns player name
const std::string& Player::get_name() const { return this->name; }

// Returns the number of tiles in a player's hand.
size_t Player::count_tiles() const {
    int tile_num = 0;
    for (unsigned int i = 0; i < player_tiles.size(); i++) {
        tile_num++;
    }
    return tile_num;
}

// Removes tiles from player's hand.
// makes sure that the player has the tiles that
// they want to remove
void Player::remove_tiles(const std::vector<TileKind>& tiles) {

    bool match = false;
    for (unsigned int i = 0; i < tiles.size(); i++) {
        for (unsigned int j = 0; j < player_tiles.size(); j++) {
            if (tiles[i] == player_tiles[j]) {
                player_tiles.erase(player_tiles.begin() + j);
                match = true;
                break;
            }
            match = false;
        }
    }
    if (!match) {
        throw CommandException("Cannot remove unexisting tile");
    }
}

// Adds tiles to player's hand.
void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (unsigned int i = 0; i < tiles.size(); i++) {
        player_tiles.push_back(tiles[i]);
    }
}

// Checks if player has a matching tile.
bool Player::has_tile(TileKind tile) {
    for (unsigned int i = 0; i < player_tiles.size(); i++) {
        if (player_tiles[i] == tile) {
            return true;
        }
    }
    return false;
}

// Returns the total points of all tiles in the players hand.
unsigned int Player::get_hand_value() const {
    unsigned int sum = 0;
    for (unsigned int i = 0; i < this->player_tiles.size(); i++) {
        sum += player_tiles[i].points;
    }
    return sum;
}
// returns hand size
size_t Player::get_hand_size() const { return this->hand_size; }
