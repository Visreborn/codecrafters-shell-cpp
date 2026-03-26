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
        // can only be activated after calling tab on a command prefix
        bool tab = 0;

        //real time reading
        while(1) {
            int ch = get_char_raw();

            if(ch == 9 || ch == '\t') {
                if(tab) {
                    std :: vector<string> ans = COMMAND_BUCKETS[3].get_all_words(input);

                    cout << '\n';

                    for(auto &exe : ans) {
                        cout << exe << ' ';
                    }

                    cout << "\n$ " << input;
                    cout.flush();
                    tab = 0;
                }

                else {
                    handle_tab_completion(input);
                }

                continue;
            }

            // we did not press tab
            tab = 0;    

            if (ch == '\n' || ch == '\r') { 
                cout << '\n';
                break;
            } 

            if (ch == 127 || ch == '\b') { 
                if (!input.empty()) {
                    input.pop_back();
                    cout << "\b \b";
                    cout.flush(); 
                }
            } 

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