#include "Shell.hpp"
#include "RealTimeTrackingAndComplete.hpp"
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

        //real time reading
        while(1) {
            int ch = get_char_raw();

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

            else if (ch == 9 || ch == '\t') { 
                handle_tab_completion(input);
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