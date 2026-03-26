#include "Shell.hpp"
#include "AutoComplete.hpp"
#include <iostream>
#include <string>

using std::cin;
using std::cout;
using std::cerr;
using std::string;

int main() {
    init_path();

    while(1) {
        cout << "$ ";
        cout.flush(); 

        string input = "";
        // can only be activated after calling tab one
        bool tab = 0;
        bool second_token_onwards = 0;

        while(1) {
            int ch = get_char_raw();

            if(ch == 9 || ch == '\t') {
                if(!second_token_onwards) {
                    int pos = input.find_last_of(" ");

                    if(pos != string :: npos) {
                        second_token_onwards = 1;
                    }
                }

                if(second_token_onwards) {
                    bool successful = get_filename(input);

                    if(!successful) {
                        get_directory(input);
                    }
                    
                    continue;
                }

                if(tab) {
                    // get all the possible words
                    listing(input);
                    tab = 0;
                }
                else {
                    if(!handle_tab_completion(input)) {
                        cout << '\a';
                        cout.flush();
                        tab = 1;
                    }
                }

                continue;
            }

            tab = 0;    

            // endline
            if (ch == '\n' || ch == '\r') { 
                cout << '\n';
                break;
            } 

            // delete(backspace)
            if (ch == 127 || ch == '\b') { 
                if (!input.empty()) {
                    input.pop_back();
                    cout << "\b \b";
                    cout.flush(); 

                    if(second_token_onwards) {
                        if(input.find_last_of(' ') == string :: npos) {
                            second_token_onwards = 0;
                        }
                    }
                }
            } 

            // normal character
            else { 
                input += char(ch);
                cout << char(ch);
                cout.flush(); 
            }
        }

        exe(input);
    }
    
    return 0;
}