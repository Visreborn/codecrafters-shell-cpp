#pragma once
#include <string>
#include<vector>

extern const std :: vector<std :: string> BUILTIN_COMMANDS;
extern std :: vector<std :: string> FETCH_PATH;
extern std :: vector<std :: string> MOST_USED_PHRASES;

// Only these two functions are public to be used in main.cpp
void init_path();
void exe(const std::string &input);