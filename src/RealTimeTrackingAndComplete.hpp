#pragma once
#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include "Shell.hpp"

#if defined(_WIN32)
    #include<conio.h>
#else
    #include<termios.h>
#endif

int get_char_raw();

// we will first implement the solution using binary search and then use 

void handle_tab_completion(std::string &input);
