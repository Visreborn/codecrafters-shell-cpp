#pragma once
#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include "Shell.hpp"
#include "Trie.hpp"

#if defined(_WIN32)
    #include<conio.h>
#else
    #include<termios.h>
#endif

int get_char_raw();

// we will first implement the solution using binary search and then use 

bool handle_tab_completion(std :: string &input);

void listing(const std :: string &input);

bool get_filename(std :: string &input);

bool get_directory(std :: string &input);