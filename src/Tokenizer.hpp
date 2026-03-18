#pragma once
#include <string>
#include <utility>

// define utility functions
int skip_ws(int i, const std :: string &input);
std :: pair<int, std :: string> get_token(int i, const std :: string &input);
int count_words(const std :: string& input);

// Tokenizer class
class Tokenizer {
    private:
        std :: string input;
        int pos = 0;
        int token_counts = 0;

    public:
        Tokenizer(std :: string in);
        std :: string next();
        std :: string get_input() const;
        int get_pos() const;
        int get_count() const;
};