#pragma once
#include <string>
#include<vector>
#include "Trie.hpp"

extern std :: vector<Trie> COMMAND_BUCKETS;
extern std :: vector<std :: string> COMMAND_BUILTINS;
extern std :: vector<std :: string> FETCH_PATH;

// Only these two functions are public to be used in main.cpp
void init_path();
void exe(const std::string &input);