#include "Tokenizer.hpp"

using std :: string;

int skip_ws(int i, const string &input) {
    while(i < input.size() && input[i] == ' ') {
        i ++;
    }
    return i;
}

std :: pair<int, string> get_token(int i, const string &input) {
    string tmp;

    while(i < input.size() && input[i] != ' ') {
        tmp += input[i];
        i ++;
    }

    return {i, tmp};
}

int count_words(const string& input) {
    int count = 0;
    bool in_word = false; 

    for (char c : input) {
        if (c == ' ') {
            in_word = false; 
        } else if (!in_word) {
            in_word = true;  
            count ++;         
        }
    }

    return count;
}

// constructor
Tokenizer :: Tokenizer(string in) : input(in) {}

string Tokenizer :: next() {
    pos = skip_ws(pos, input);
    if (pos == input.size()) return "";
    auto [next_pos, token] = get_token(pos, input);
    pos = next_pos;
    return token;
}

string Tokenizer :: get_input() const {
    return input;
}

int Tokenizer :: get_pos() const {
    return pos;
}