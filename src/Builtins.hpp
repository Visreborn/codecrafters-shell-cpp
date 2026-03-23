#pragma once
#include "Tokenizer.hpp"
#include "Redirection.hpp"
#include<string>
#include<vector>
#include<fcntl.h>
#include<iostream>
#include<unistd.h>

#if defined(_WIN32)
    #include<io.h>
#endif

void print(Tokenizer &tokenizer);
void cat(std :: vector<std :: string> &args);

void pwd();
void changeCWD(Tokenizer &tokenizer);