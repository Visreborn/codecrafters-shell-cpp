#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<algorithm>
#include "RealTimeTrackingAndComplete.hpp"

#if defined(_WIN32)
    #include<conio.h>
#else
    #include<termios.h>
#endif

using std :: string;
using std :: cout;

int get_char_raw() {
    #if defined(_WIN32)
        return _getch();
    #else 
        struct termios old_attr, new_attr;
        int ch;
        
        // gets the current configurations of the terminal
        tcgetattr(STDIN_FILENO, &old_attr);
        new_attr = old_attr;
        
        // turn off the icanon flag and echo flag
        new_attr.c_lflag &= ~(ICANON | ECHO);
        
        // apply the new configurations
        tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);
        
        
        ch = getchar();
        
        // giving back the orignal configurations
        tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
        
        return ch;
    #endif
}

// Note : I will soon optimize this using Trie
void handle_tab_completion(string &input) {
    auto it = lower_bound(MOST_USED_PHRASES.begin(), MOST_USED_PHRASES.end(), input);

    if(it != MOST_USED_PHRASES.end() && it -> find(input) == 0) {
            string match = *it;

            string remainder = match.substr(input.size());

            cout << remainder << " ";
            cout.flush();

            input += remainder + " ";
    } 
    
    else {
        cout << '\a';
        cout.flush();
    }
}