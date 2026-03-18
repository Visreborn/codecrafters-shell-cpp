#include "Shell.hpp"
#include "Tokenizer.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>
#include <sstream>

using std::cout;
using std::cerr;
using std::string;
using std::endl;

#if defined(_WIN32)
    const char PATH_DELIMETER = ';';
#else
    const char PATH_DELIMETER = ':';
#endif

// hidden global variables
const std::vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type"};
std::vector<string> FETCH_PATH;

// --- PRIVATE FUNCTIONS ---

bool IsExecutable(const string &path) {
    return access(path.c_str(), X_OK) == 0;
}

void print(Tokenizer &tokenizer) {
    std :: vector<string> tokens;

    while(1) { 
        string cur_token = tokenizer.next();    
        if(cur_token == "") break;
        tokens.push_back(cur_token);
    }

    for(auto &token : tokens) {
        cout << token << ' ';
    }

    cout << endl;
}

string find_directory(string &token, int type) {
    for(const auto &path : FETCH_PATH) {

        if(!std::filesystem::exists(path)) continue;

        for(const auto &entry : std::filesystem::directory_iterator(path)) {
            string absolute_path = entry.path().string();
            string file_name = entry.path().filename().string();

            if((file_name == token || file_name == token + ".exe") && IsExecutable(absolute_path)) {
                if(type == 0) {
                    return absolute_path;
                } else {
                    return file_name;
                }
            }
        }
    }

    return "";
}

void handle_type(Tokenizer &tokenizer) {
    int pos = tokenizer.get_pos();
    string input = tokenizer.get_input();
    string rem = input.substr(pos);
    string token = tokenizer.next();
    int cnt = count_words(input);

    if(cnt == 2) {
        for(auto &builtin : BUILTIN_COMMANDS) {
            if(builtin == token) {
                cout << token << " is a shell builtin" << endl;
                return;
            }
        }
    }

    // get the remainings to see if they form an executable file
    token = rem;
    
    string res = find_directory(token, 0);

    if(res.size() > 0) {
        cout << token << " is " << res << endl;
        return;
    }

    cout << token << ": not found" << endl;
}

bool run_external_programs(string &first_token, Tokenizer &tokenizer) {
    string res = find_directory(first_token, 1); 

    if(res.size() > 0) {    
        string full_command = "\"" + res + "\"";

        while(1) {
            string arg = tokenizer.next();
            if(arg == "") break;
            full_command += " " + arg;
        }

        // full_command = "\"" + full_command  + "\"";

        std :: system(full_command.c_str());
        return 1;
    }

    return 0;
}

// --- PUBLIC FUNCTIONS ---

void init_path() {
    const char *path_env = std::getenv("PATH");
    if(path_env == nullptr) return;

    std::string path_str(path_env);
    std::stringstream ss(path_str);
    std::string dir;

    while(getline(ss, dir, PATH_DELIMETER)) {
        if(dir.size() > 0) {
            FETCH_PATH.push_back(dir);
        }
    }
}

void exe(const string &input) {
    // initializer Tokenizer
    Tokenizer tokenizer(input);
    string first_token = tokenizer.next();

    if(first_token == "") { // escape
        return; 
    }
    
    if(first_token == "exit") {
        string second_token = tokenizer.next();
        
        if (second_token == "") {
            exit(0);
        } else {
            cout << input << ": command not found" << endl;
            return;
        }
    }

    // test for builtint commands
    if(first_token == "echo") {
        print(tokenizer);
        return;
    } 
    
    if(first_token == "type") {
        handle_type(tokenizer);
        return;
    } 
    
    if(run_external_programs(first_token, tokenizer)) {
        // cout << endl;
        return;
    }

    cout << input << ": command not found" << endl;
}