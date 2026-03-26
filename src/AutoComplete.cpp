#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<algorithm>
#include "AutoComplete.hpp"

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

bool handle_tab_completion(string &input) {
    string last_word = get_last_word(input);

    if(last_word.empty()) {
        return 0;
    }

    for(int i = 0; i < 4; i ++) {
        Trie &MOST_USED_PHRASES = COMMAND_BUCKETS[i];

        string match = MOST_USED_PHRASES.get_longest_common_prefix(last_word);

        if(match.size() > last_word.size()) {
            string remainder = match.substr(last_word.size());

            std :: vector<string> matches = MOST_USED_PHRASES.get_all_words(match);
            if(matches.size() == 1) {
                remainder += " ";
            }

            cout << remainder;
            cout.flush();

            input += remainder;

            return 1;
        }
    } 
        
    return 0;
}

void listing(const string &input) {
    string last_word = get_last_word(input);
    std :: vector<string> ans = COMMAND_BUCKETS[3].get_all_words(last_word);

    if(!ans.empty()) {
        cout << '\n';

        for(auto &exe : ans) {
            cout << exe << "  ";
        }

        cout << "\n$ " << input;
        cout.flush();
    }
}