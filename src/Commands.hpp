#pragma once
#include<string>
#include<vector>
#include "Tokenizer.hpp"
#include<fcntl.h>
#include<iostream>

#if defined(_WIN32)
    #include<io.h>
#else 
    #include<unistd.h>
#endif

struct Redirector {
    int saved_fd = -1;

    // by defaut should be stdout
    int target_fd = 1;
    
    bool active = 0;

    void setup(const std :: string &filename, int fd_to_redirect) {    
        if(filename.empty()) return;

        if(fd_to_redirect == 1) std :: cout.flush();
        else std :: cerr.flush();

        target_fd = fd_to_redirect;

        #if defined(_WIN32)
            // _O_TRUNC helps overwrite existed files
            int fd = _open(filename.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC, 0666);

            if (fd != -1) {
                saved_fd = _dup(target_fd);   // save the current standard output
                _dup2(fd, target_fd);             // points to the file
                _close(fd);
                active = true;
            }

        #else
            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

            if (fd != -1) {
                saved_fd = dup(target_fd);
                dup2(fd, target_fd);
                close(fd);
                active = true;
            }

        #endif
        
        if (!active) {
            std :: cerr << "shell: " << filename << ": No such file or directory\n";
        }
    }

    void restore() {
        if (active && saved_fd != -1) {
            std :: cout.flush(); 

            #if defined(_WIN32)
                _dup2(saved_fd, target_fd);   // Points the pipeline back to the intended 
                _close(saved_fd);
            #else
                dup2(saved_fd, target_fd);
                close(saved_fd);
            #endif
            active = false;
        }
    }
};

void print(Tokenizer &tokenizer);
void cat(std :: vector<std :: string> &args);

void pwd();
void changeCWD(Tokenizer &tokenizer);