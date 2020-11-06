Project: Scrabble

Description:
This project is a full implementation of a Scrabble game that can be played through the 
console with both human players and computer players. The game terminates if all human players pass
or when one of the players runs out of tiles.


Game Features:
Computer Player: The computer player was implemented using a 
recursive backtracking algorithm that scans the board and picks the best possible move based on
tiles already placed, and tiles in hand.

Blank Tile: The blank tile is a wildcard tile in which the user has the option
of choosing which letter they want this tile to represent. The computer player checks
this tile by going through all available letters that the tile could possibly represent.

Dictionary Trie Implementation: The paths of the tree in the trie represent prefix strings that are
stored in each node. Each node represents the series of prefix strings required to reach it, and its children are continuations of the string, which eventually forms words. This aids in the computer player backtracking.


