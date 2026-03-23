#include "Shell.hpp"
#include "Tokenizer.hpp"
#include "Builtins.hpp"
#include<iostream>
#include<vector>
#include<unistd.h>
#include<filesystem>
#include<cstdlib>
#include<sstream>
#include<fstream>
#include<fcntl.h>
#include<algorithm>

using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

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

// hidden global variables
const std :: vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type", "pwd"};
std :: vector<string> FETCH_PATH;
std :: vector<string> MOST_USED_PHRASES;

// --- PRIVATE FUNCTIONS ---

bool IsExecutable(const string &path) {
    return access(path.c_str(), X_OK) == 0;
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
                cout << token << " is a shell builtin" << '\n';
                return;
            }
        }
    }

    // get the remainings to see if they form an executable file
    token = rem;
    
    string res = find_directory(token, 0);

    if(res.size() > 0) {
        cout << token << " is " << res << '\n';
        return;
    }

    cout << token << ": not found" << '\n';
}

bool run_external_programs(string &first_token, Tokenizer &tokenizer) {
    string cmd_path = find_directory(first_token, 1); 

    if(!cmd_path.empty()) {
        std :: vector<string> args_str;
        args_str.push_back(first_token); 

        string redirect_file = "";

        // default : stdout
        int type = 1;
        // default : overwritten
        int mode = 0;

        while(true) {
            string arg = tokenizer.next();
            if(arg == "") break;

            if(arg == ">" || arg == "1>") {
                redirect_file = tokenizer.next();
                type = 1;
                mode = 0;
                continue;
            } 
            else if(arg == "2>") {
                redirect_file = tokenizer.next();
                type = 2;
                mode = 0;
                continue;
            }
            else if(arg == ">>" || arg == "1>>") {
                redirect_file = tokenizer.next();
                type = 1;
                mode = 1;
                continue;
            }
            else if(arg == "2>>") {
                redirect_file = tokenizer.next();
                type = 2;
                mode = 1;
                continue;
            }

            if (arg.find(' ') != string :: npos) {
                arg = "\"" + arg + "\"";
            }
            args_str.push_back(arg); 
        }

        std :: vector<const char*> argv;
        for (const auto& s : args_str) argv.push_back(s.c_str());
        argv.push_back(NULL); 

        // redirect standard ouput to a specific file
        Redirector redir;
        redir.setup(redirect_file, type, mode);

        #if defined(_WIN32)
            _spawnvp(_P_WAIT, cmd_path.c_str(), (char* const*)argv.data());
        #else
            pid_t pid = fork();

            if (pid == 0) {
                execvp(argv[0], (char* const*)argv.data());
                
                perror("execvp"); 
                exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("fork");
            }
        #endif

        //switch back to terminal
        redir.restore();

        return 1;
    }

    return 0;
}

// --- PUBLIC FUNCTIONS ---

void init_path() {
    const char *path_env = std :: getenv("PATH");
    if(path_env == nullptr) return;

    string path_str(path_env);
    std :: stringstream ss(path_str);
    string dir;

    while(getline(ss, dir, PATH_DELIMETER)) {
        if(dir.size() > 0) {
            FETCH_PATH.push_back(dir);
        }
    }

    
    for(auto &token : BUILTIN_COMMANDS) {
        MOST_USED_PHRASES.push_back(token);
    }

    // Code sửa lại:
    for(auto &path : FETCH_PATH) {
        if(!fs::exists(path)) continue;
        
        // Bắt lỗi permission denied khi đọc thư mục hệ thống
        try {
            for(const auto &entry : fs::directory_iterator(path)) {
                // Lấy tên file
                string file_name = entry.path().filename().string();
                string absolute_path = entry.path().string();
                
                // Chỉ thêm vào danh sách gợi ý nếu nó là file thực thi
                if(IsExecutable(absolute_path)) {
                    MOST_USED_PHRASES.push_back(file_name);
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            // Bỏ qua các thư mục không có quyền truy cập
            continue;
        }
    }

    // for(int i = 0; i < 5; i ++) {
    //     cout << MOST_USED_PHRASES[i] << '\n';
    // }

    std :: sort(MOST_USED_PHRASES.begin(), MOST_USED_PHRASES.end());

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
            cout << input << ": command not found" << '\n';
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

    cout << input << ": command not found" << '\n';
}