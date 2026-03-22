#include "Shell.hpp"
#include "Tokenizer.hpp"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <fcntl.h>

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
const std::vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type", "pwd"};
std::vector<string> FETCH_PATH;

// --- PRIVATE FUNCTIONS ---

struct Redirector {
    int saved_stdout = -1;
    bool active = false;

    void setup(const string& filename) {
        if (filename.empty()) return;
        
        // Đẩy hết dữ liệu cũ ra màn hình trước khi đổi ống dẫn
        cout.flush(); 

        #if defined(_WIN32)
            // _O_TRUNC giúp ghi đè file cũ nếu đã tồn tại
            int fd = _open(filename.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC, 0666);

            if (fd != -1) {
                saved_stdout = _dup(1);   // save the current standard output
                _dup2(fd, 1);             // points to the file
                _close(fd);
                active = true;
            }

        #else
            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);

            if (fd != -1) {
                saved_stdout = dup(1);
                dup2(fd, 1);
                close(fd);
                active = true;
            }

        #endif
        
        if (!active) {
            cerr << "shell: " << filename << ": No such file or directory\n";
        }
    }

    void restore() {
        if (active && saved_stdout != -1) {
            cout.flush(); 

            #if defined(_WIN32)
                _dup2(saved_stdout, 1);   // Points the pipeline back to the terminal
                _close(saved_stdout);
            #else
                dup2(saved_stdout, 1);
                close(saved_stdout);
            #endif
            active = false;
        }
    }
};

bool IsExecutable(const string &path) {
    return access(path.c_str(), X_OK) == 0;
}

void print(Tokenizer &tokenizer) {
    std :: vector<string> tokens;
    string redirect_file = "";

    while(1) { 
        string cur_token = tokenizer.next();    
        if(cur_token == "") break;

        if(cur_token == ">" || cur_token == "1>") {
            redirect_file = tokenizer.next();
            break;
        }

        tokens.push_back(cur_token);
    }

    Redirector redir;
    redir.setup(redirect_file);

    for(auto &token : tokens) {
        cout << token << ' ';
    }

    cout << endl;

    redir.restore();
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

bool run_external_programs(string &first_token, Tokenizer &tokenizer) {
    string cmd_path = find_directory(first_token, 1); 

    if(!cmd_path.empty()) {
        std :: vector<string> args_str;
        args_str.push_back(first_token); 
        string redirect_file = "";

        while(true) {
            string arg = tokenizer.next();
            if(arg == "") break;

            if(arg == ">" || arg == "1>") {
                redirect_file = tokenizer.next();
                break;
            }

            if (arg.find(' ') != string::npos) {
                arg = "\"" + arg + "\"";
            }

            args_str.push_back(arg); 
            // cerr << arg << endl;
        }

        std :: vector<const char*> argv;
        for (const auto& s : args_str) argv.push_back(s.c_str());
        argv.push_back(NULL); 

        // redirect standard ouput to a specific file
        Redirector redir;
        redir.setup(redirect_file);

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
        cerr << "cat: missing file operand" << endl;
        return;
    } 

    // check if there is a redirection operator inside the argument
    int n = args.size();
    string redirect_file = "";

    for(int i = 0; i < n; i ++) {
        if(args[i] == ">" || args[i] == "1>") {
            if(i + 1 < n) {
                redirect_file = args[i + 1];
            }

            n = i;
            break;
        }
    }

    Redirector redir;
    redir.setup(redirect_file);

    for(int i = 0; i < n; i ++) {
        std :: ifstream file(args[i]);

        if(!file.is_open()) {
            cerr << "cat: " << args[i] << ": No such file or directory" << endl;
            continue;
        }

        string line;

        while(getline(file, line)) {
            cout << line << '\n';
        }

        file.close();
    }

    redir.restore();
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