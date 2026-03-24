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

string get_last_word(const string &input) {
    if(input.empty()) {
        return "";
    }

    int last_pos = input.find_last_of(" ");

    if(last_pos == string :: npos) {
        return input;
    }

    return input.substr(last_pos + 1);
}

// Note : I will soon optimize this using Trie
void handle_tab_completion(string &input) {
    string last_word = get_last_word(input);

    for(int i = 0; i < 4; i ++) {
        const std :: vector<string> &MOST_USED_PHRASES = COMMAND_BUCKETS[i];

        auto it = lower_bound(MOST_USED_PHRASES.begin(), MOST_USED_PHRASES.end(), last_word);

        if(it != MOST_USED_PHRASES.end() && it -> find(last_word) == 0) {
            string match = *it;

            string remainder = match.substr(last_word.size());

            cout << remainder << " ";
            cout.flush();

            input += remainder + " ";

            return;
        } 
    }

    cout << '\a';
    cout.flush();
}