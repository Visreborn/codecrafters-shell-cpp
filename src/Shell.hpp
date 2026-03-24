#pragma once
#include <string>
#include<vector>

extern std :: vector <std :: vector<std :: string>> COMMAND_BUCKETS;
extern std :: vector<std :: string> FETCH_PATH;

// Only these two functions are public to be used in main.cpp
void init_path();
void exe(const std::string &input);