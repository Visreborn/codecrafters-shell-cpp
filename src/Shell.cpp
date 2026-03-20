#include "Shell.hpp"
#include "Tokenizer.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include<process.h>

using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

namespace fs = std :: filesystem;

#if defined(_WIN32)
    const char PATH_DELIMETER = ';';
    const char DIRECTORY_DELIMETER = '\\';
    const string HOME = "USERPROFILE";
#else
    const char PATH_DELIMETER = ':';
    const char DIRECTORY_DELIMETER = '/';
    const string HOME = "HOME";
#endif

// hidden global variables
const std::vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type", "pwd"};
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

        if(!fs :: exists(path)) continue;

        for(const auto &entry : fs :: directory_iterator(path)) {
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

    // skip all the white spaces
    pos = skip_ws(pos, input);
    string rem = input.substr(pos);

    string token = tokenizer.next();
    int cnt = tokenizer.get_count();

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

#include <process.h> // Đối với Windows (_spawnvp)
#include <vector>

bool run_external_programs(string &first_token, Tokenizer &tokenizer) {
    string cmd_path = find_directory(first_token, 0); // Lấy đường dẫn tuyệt đối

    if(!cmd_path.empty()) {
        // Tạo danh sách argument dạng mảng (argv style)
        std::vector<string> args_str;
        args_str.push_back(first_token); // argv[0] thường là tên lệnh

        while(true) {
            string arg = tokenizer.next();
            if(arg == "") break;
            args_str.push_back(arg); // Add thẳng, KHÔNG CẦN BỌC NHÁI
        }

        // Chuyển vector<string> sang char* array (kiểu C cũ)
        std::vector<const char*> argv;
        for (const auto& s : args_str) argv.push_back(s.c_str());
        argv.push_back(NULL); // Kết thúc mảng bằng NULL

        // CHẠY TRỰC TIẾP (Bỏ qua system() và cmd.exe)
        #if defined(_WIN32)
            _spawnvp(_P_WAIT, cmd_path.c_str(), (char* const*)argv.data());
        #else
            // Trên Linux bạn dùng fork + execvp hoặc posix_spawn
            // Đây là ví dụ đơn giản với spawnvp tương đương trên nhiều nền tảng
            _spawnvp(_P_WAIT, cmd_path.c_str(), (char* const*)argv.data());
        #endif

        return true;
    }
    return false;
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

void cat(std :: vector<string> &args) {
    if(args.empty()) {
        cout << "cat: missing file operand" << endl;
        return;
    } 

    for(const auto &filename : args) {
        std :: ifstream file(filename);

        if(!file.is_open()) {
            cout << "cat: " << filename << ": No such file or directory" << endl;
            continue;  
        }

        string line;

        while(getline(file, line)) {
            cout << line;
        }

        file.close();
    }

    cout << endl;
}

// --- PUBLIC FUNCTIONS ---

void init_path() {
    const char *path_env = std::getenv("PATH");
    if(path_env == nullptr) return;

    string path_str(path_env);
    std :: stringstream ss(path_str);
    string dir;

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

    if(first_token == "pwd" && tokenizer.get_count() == 1) {
        pwd();
        return;
    } 

    if(first_token == "cd") {
        changeCWD(tokenizer);
        return;
    }

    if(first_token == "cat") {
        std :: vector<string> args;
        
        while(1) {
            string arg = tokenizer.next();
            if (arg == "") break;
            args.push_back(arg);
        }
        
        cat(args);
        return;
    }

    if(run_external_programs(first_token, tokenizer)) {
        return;
    }

    cout << input << ": command not found" << endl;
}