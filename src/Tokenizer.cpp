#include "Tokenizer.hpp"
#include<iostream>

using std :: string;
using std :: cerr;
using std :: endl;

int skip_ws(int i, const string &input) {
    while(i < input.size() && input[i] == ' ') {
        i ++;
    }

    return i;
}

std :: pair<int, string> get_token(int i, const string &input) {
    bool in_single_quotes = 0;
    bool in_double_quotes = 0;
    bool literal = 0;
    string tmp;

    while(i < input.size()) {
        // cerr << input[i] << ' ' << in_double_quotes << ' ' << in_single_quotes << endl;

        if(input[i] == '\'') {
            if(literal == 1) {
                literal = 0;
                tmp += input[i];
            }

            else if(!in_double_quotes) {
                in_single_quotes ^= 1;
            } 
            
            else {
                tmp += input[i];
            }
        } 

        else if(input[i] == '\"') {
            if(in_single_quotes) {
                tmp += input[i];
            } 

            else if(literal == 1) {
                tmp += input[i];
                literal = 0;
            }

            else {
                in_double_quotes ^= 1;
            }
        }
        
        else if(input[i] == ' ') {
            if(in_single_quotes || in_double_quotes) {
                tmp += ' ';
            }
             
            else if(literal == 1) {
                tmp += ' ';    
                literal = 0;
            }
            
            else {
                break;
            }
        }

        else if(input[i] == '\\') {
            // if not in single quotes then it is either in double quotes or outside 
            if(in_single_quotes) { 
                tmp += input[i];
            }

            else if(!literal) {
                literal = 1;
            }
            
            else if(literal == 1) {
                literal = 0;
                tmp += input[i];
            } 
        }

        else {
            if(literal == 1) {
                literal = 0;
            }
            
            tmp += input[i];
        }

        i ++;
    }

    return {i, tmp};
}

int count_tokens(const string& input) { // this is for counting tokens that are separated by commas
    int count = 0;
    bool in_token = 0;
    bool in_single_quotes = 0;
    bool in_double_quotes = 0;

    for (char c : input) {
        if(c == '\'') {
            if(!in_double_quotes) {
                in_single_quotes ^= 1;
            }

            if(!in_token) {
                in_token = 1;
                count ++;
            }
        } 

        else if(c == '\"') {
            in_double_quotes ^= 1;

            if(!in_token) {
                in_token = 1;
                count ++;
            }
        }

        else if(c == ' ' && !in_single_quotes && !in_double_quotes) {
            in_token = 0;
        }

        else if(!in_token) {
            count ++;
            in_token = 1;
        }
    }

    return count;
}

// constructor
Tokenizer :: Tokenizer(string in) : input(in), token_counts(count_tokens(in)) {}

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

int Tokenizer :: get_count() const { // token counts
    return token_counts;
}