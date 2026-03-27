#include<iostream>
#include<stdio.h>
#include<unistd.h>
#include<algorithm>
#include "AutoComplete.hpp"
#include<filesystem>

#if defined(_WIN32)
    #include<conio.h>
#else
    #include<termios.h>
#endif

using std :: string;
using std :: cout;

namespace fs = std :: filesystem;

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

        auto [match, ended] = MOST_USED_PHRASES.get_longest_common_prefix(last_word);

        if(match.size() > last_word.size()) {
            string remainder = match.substr(last_word.size());

            if(ended == 1) {
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

bool get_filename(string &input) {
    string prefix = get_last_word(input);

    if(prefix.empty()) return 0;

    fs :: path CurrentPath = fs :: current_path();

    for(const auto &entry : fs :: directory_iterator(CurrentPath)) {
        string filename = entry.path().filename().string();
        
        if(filename.find(prefix) == 0) {
            string remainder = filename.substr(prefix.size()) + " ";
            cout << remainder;
            cout.flush();
            input += remainder;
            return 1;
        }
    }

    return 0;
}

bool get_absolute_path(string &input) {
    string tot = get_last_word(input);

    #if defined(_WIN32) 
        size_t pos = tot.find_last_of('\\');
    #else 
        size_t pos = tot.find_last_of('/');
    #endif

    if(pos == string :: npos) return 0;

    string dir = tot.substr(0, pos + 1);
    string prefix = tot.substr(pos + 1);

    fs :: path CurrentPath(dir);

    std :: error_code ec;
    if (!fs :: exists(CurrentPath, ec) || !fs :: is_directory(CurrentPath, ec)) {
        return 0;
    }
    
    for(const auto &entry : fs :: directory_iterator(CurrentPath, ec)) {
        string filename = entry.path().filename().string();
        
        if(filename.find(prefix) == 0) {
            string remainder = filename.substr(prefix.size()) + " ";
            cout << remainder;
            cout.flush();
            input += remainder;
            return 1;
        }
    }

    return 0;
}

bool get_directory(string &input) {
    string tot = get_last_word(input);
    
    // Mặc định tìm ở thư mục hiện tại
    string dir = "."; 
    string prefix = tot;

    // Tìm dấu gạch chéo cuối cùng để xem có phải đường dẫn không
    #if defined(_WIN32) 
        size_t pos = tot.find_last_of('\\');
    #else 
        size_t pos = tot.find_last_of('/');
    #endif

    // Nếu có dấu phân cách, tách riêng thư mục gốc và từ khóa cần tìm
    if(pos != string :: npos) {
        dir = tot.substr(0, pos + 1);
        prefix = tot.substr(pos + 1);
    }

    fs :: path CurrentPath(dir);
    std :: error_code ec;

    // Kiểm tra an toàn: đường dẫn có tồn tại và đúng là thư mục không?
    if (!fs :: exists(CurrentPath, ec) || !fs :: is_directory(CurrentPath, ec)) {
        return 0;
    }
    
    // Bắt đầu quét
    for(const auto &entry : fs :: directory_iterator(CurrentPath, ec)) {
        // QUAN TRỌNG: Bỏ qua ngay lập tức nếu đây là file thường
        if (!entry.is_directory(ec)) {
            continue; 
        }

        string dirname = entry.path().filename().string();
        
        // Khớp từ khóa (nếu prefix = "" thì nó sẽ tự khớp thư mục đầu tiên thấy)
        if(dirname.find(prefix) == 0) {
            string remainder = dirname.substr(prefix.size());
            
            // Nối thêm gạch chéo thay vì dấu cách
            #if defined(_WIN32)
                remainder += "\\";
            #else
                remainder += "/";
            #endif

            cout << remainder;
            cout.flush();
            input += remainder;
            return 1;
        }
    }

    return 0; // Không tìm thấy gì
}