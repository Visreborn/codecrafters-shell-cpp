#pragma once
#include "Tokenizer.hpp"
#include <iostream>
#include <fcntl.h>
#include <string>
#include <vector>
#include<unistd.h>

#if defined(_WIN32)
    #include<io.h>
#endif

struct Redirector {
    int saved_fd = -1;

    // the standard format is stdout and stderr outo 
    // the given terminal
    int target_fd = 1;

    // to determine if there is a switch to the buffers in general
    bool active = 0;

    void setup(const std :: string &filename, int fd_to_redirect, int mode) {
        if(filename.empty()) {
            return;
        }

        target_fd = fd_to_redirect;

        //clear the waste or stucked outputs
        if(target_fd == 1) std :: cout.flush();
        else std :: cerr.flush();

        #if defined(_WIN32) 

            // flag[0] -> overwritten
            // flag[1] -> append
            int flag[2] = {_O_TRUNC, _O_APPEND};

            int fd = _open(filename.c_str(), _O_WRONLY | _O_CREAT | flag[mode], 0666);

            // if fd = -1 -> we couldn't open the file
            if(fd != -1) {
                saved_fd = _dup(target_fd);
                _dup2(fd, target_fd); // points to the file as the new intended buffer
                _close(fd);
                active = 1;
            }

        #else 

            // flag[0] -> overwritten
            // flag[1] -> append
            int flag[2] = {O_TRUNC, O_APPEND};

            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | flag[mode], 0666);

            // if fd = -1 -> we couldn't open the file
            if(fd != -1) {
                saved_fd = _dup(target_fd);
                dup2(fd, target_fd); // points to the file as the new intended buffer
                close(fd);
                active = 1;
            }

        #endif

        if (!active) {
            std :: cerr << "shell: " << filename << ": No such file or directory\n";
        }
    }

    void restore() {
        if(active && saved_fd != -1) {
            if(target_fd == 1) std :: cout.flush();
            else std :: cerr.flush();

            #if defined(_WIN32)
                _dup2(saved_fd, target_fd);
                _close(saved_fd);
            #else
                dup2(saved_fd, target_fd);
                close(saved_fd);
            #endif

            active = 0;
        }
    }
};