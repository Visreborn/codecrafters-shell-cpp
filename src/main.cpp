#include "Shell.hpp"
#include <iostream>
#include <string>

using std::cin;
using std::cout;
using std::cerr;
using std::string;

int main() {
    // Flush after every cout and cerr
    cout << std :: unitbuf;
    cerr << std :: unitbuf;

	// called this from shell.hpp
    init_path();

    while(1) {
        cout << "$ ";

        string input;
        getline(cin, input);

        // also from shell.hpp
        exe(input);
    }
    
    return 0;
}