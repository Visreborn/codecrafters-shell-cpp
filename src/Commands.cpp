#include "Commands.hpp"
#include <iostream>
#include <fstream>
#include<filesystem>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

namespace fs = std :: filesystem;

#if defined(_WIN32)
    const char PATH_DELIMETER = ';';
    const char DIRECTORY_DELIMETER = '\\';
    const string HOME = "USERPROFILE";
    #include<io.h>
    #include <process.h>
#else
    const char PATH_DELIMETER = ':';
    const char DIRECTORY_DELIMETER = '/';
    const string HOME = "HOME";
    #include <sys/wait.h>  // Cho waitpid
#endif

void print(Tokenizer &tokenizer) {
    std :: vector<string> tokens;
    string redirect_file = "";
    int type = 1;

    while(1) { 
        string cur_token = tokenizer.next();    
        if(cur_token == "") break;

        if(cur_token == ">" || cur_token == "1>") {
            redirect_file = tokenizer.next();
            type = 1;
            break;
        } 
        
        if(cur_token == "2>") {
            redirect_file = tokenizer.next();
            type = 2;
            break;
        }

        tokens.push_back(cur_token);
    }

    Redirector redir;
    redir.setup(redirect_file, type);

    for(auto &token : tokens) {
        cout << token << ' ';
    }

    cout << endl;

    redir.restore();
}

void cat(std :: vector<string> &args) {
    if(args.empty()) {
        cerr << "cat: missing file operand" << endl;
        return;
    } 

    // check if there is a redirection operator inside the argument
    int n = args.size();
    string redirect_file = "";
    int type = 1;

    for(int i = 0; i < n; i ++) {
        if(args[i] == ">" || args[i] == "1>") {
            if(i + 1 < n) {
                redirect_file = args[i + 1];
                type = 1;
            }

            n = i;
            break;
        }

        else if(args[i] == "2>") {
            if(i + 1 < n) {
                redirect_file = args[i + 1];
                type = 2;
            }

            n = i;
            break;
        }
    }

    Redirector redir;
    redir.setup(redirect_file, type);

    for(int i = 0; i < n; i ++) {
        std :: ifstream file(args[i]);

        if(!file.is_open()) {
            cerr << "cat: " << args[i] << ": No such file or directory" << endl;
            continue;
        }

        // read directly from the buffer so that the content 
        // is exacly what it is inside the file
        cout << file.rdbuf();

        file.close();
    }

    redir.restore();
}

void pwd() {
    try {
        fs :: path current_path = fs :: current_path();
        cout << current_path.string() << endl;
    } catch(const fs :: filesystem_error& e) {
        cerr << e.what() << endl;
    }
}

void changeCWD(Tokenizer &tokenizer) { // change Current Working Directory
    string tmp = tokenizer.next();

    // we convert tmp to a be a path
    fs :: path CurrentPath(tmp);

    std :: stringstream ss(tmp);
    std :: vector<string> store;
    string ans;

    // get the CWD
    fs :: path CWD = fs :: current_path();

    while(getline(ss, ans, DIRECTORY_DELIMETER)) {
        store.push_back(ans);
    }   

    if(store[0] == "~" && store.size() == 1) {
        string finalHomePath = "";

    #if defined(_WIN32)
        const char* homeDrive = std :: getenv("HOMEDRIVE");
        const char* homePath = std :: getenv("HOMEPATH");
        
        if (homeDrive != nullptr && homePath != nullptr) {
            finalHomePath = string(homeDrive) + string(homePath);
        }
    #else
        const char* homeEnv = std :: getenv("HOME");

        if (homeEnv != nullptr) {
            finalHomePath = homeEnv;
        }
    #endif

        if (finalHomePath.empty()) {
            std :: cout << "cd: could not determine home directory" << std::endl;
            return;
        }

        fs :: current_path(finalHomePath);
        return;
    }

    if(store[0] + DIRECTORY_DELIMETER == CWD.root_path().string()) { // change the CWD
        if(!fs :: exists(CurrentPath)) {
            cout << "cd: " << tmp << ": No such file or directory" << endl;
            return;
        }

        fs :: current_path(CurrentPath);
        return;
    }

    if(store[0] == "..") {
        fs :: path new_path = CWD;

        for(int i = 0; i < store.size(); i ++) {
            new_path = new_path.parent_path();
        }

        fs :: current_path(new_path);
        return;
    }

    string tot_path = CWD.string();

    for(int i = 0; i < store.size(); i ++) {
        if(store[i] == ".") continue;
        tot_path += DIRECTORY_DELIMETER + store[i];
    }

    fs :: path new_path(tot_path);

    if(!fs :: exists(new_path)) {
        cout << "cd: " << tmp << ": No such file or directory" << endl;
        return;
    }

    fs :: current_path(new_path);
}
