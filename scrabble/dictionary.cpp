#include "dictionary.h"

#include "exceptions.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

string lower(string str) {
    transform(str.cbegin(), str.cend(), str.begin(), ::tolower);
    return str;
}

// Implemented for you to read dictionary file and
// construct dictionary trie graph for you
Dictionary Dictionary::read(const std::string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open dictionary file!");
    }
    std::string word;
    Dictionary dictionary;
    dictionary.root = make_shared<TrieNode>();

    while (!file.eof()) {
        file >> word;
        if (word.empty()) {
            break;
        }
        dictionary.add_word(lower(word));
    }

    return dictionary;
}

bool Dictionary::is_word(const string& word) const {
    // goes to the prefix of the word
    shared_ptr<TrieNode> cur = find_prefix(word);
    // returns false if cur is null
    if (cur == nullptr) {
        return false;
    }
    // returns true if it is final
    return cur->is_final;
}

shared_ptr<Dictionary::TrieNode> Dictionary::find_prefix(const string& prefix) const {
    shared_ptr<TrieNode> cur = root;
    // This is a for each loop in C++. It is equivalent to the following:
    // for (int i = 0; i < prefix.length(); i++) {
    //      char letter = prefix[i];
    for (char letter : prefix) {
        if (cur == nullptr || cur->nexts.find(letter) == cur->nexts.end()) {
            return nullptr;
        }
        // keeps iterating to nexts nexts until the end of the word is is reached
        // or nullptr is returned
        cur = cur->nexts.find(letter)->second;
        // HW5: IMPLEMENT HERE
        // if there is no child of cur using `letter`
        //     return nullptr
        // set cur to the child of cur that uses `letter`
    }
    return cur;
}

// Implemented for you to build the dictionary trie
void Dictionary::add_word(const string& word) {
    shared_ptr<TrieNode> cur = root;
    for (char letter : word) {
        if (cur->nexts.find(letter) == cur->nexts.end()) {
            cur->nexts.insert({letter, make_shared<TrieNode>()});
        }
        cur = cur->nexts.find(letter)->second;
    }
    cur->is_final = true;
}

vector<char> Dictionary::next_letters(const std::string& prefix) const {
    shared_ptr<TrieNode> cur = find_prefix(prefix);
    vector<char> nexts;
    if (cur == nullptr) {
        return nexts;
    }
    // creates an iterator to the map in dictionary
    map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator it;
    // pushes back all of the next letters that cur has
    for (it = cur->nexts.begin(); it != cur->nexts.end(); it++) {
        nexts.push_back(it->first);
    }
    return nexts;
}
