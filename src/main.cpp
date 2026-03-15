#include<iostream>
#include<string>
#include<vector>
#include<utility>

using std :: cin;
using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

int skip_ws(int i, const string &input) { // skip white spaces
	while(i < input.size() && input[i] == ' ') {
		i ++;
	}

	return i;
}

std :: pair<int, string> get_token(int i, const string &input) { // get the token and return the final index after getting it
	string tmp;

	while(i < input.size() && input[i] != ' ') {
		tmp += input[i];
		i ++;
	}

	return {i, tmp};
}

bool status(const string &input) {
	std :: vector<string> tokens;

	int i = 0; 

	while(1) {
		// skip all whitespaces
		i = skip_ws(i, input);

		if(i == input.size()) {
			break;
		}

		std :: pair<int, string> ans = get_token(i, input);
		i = ans.first;

		if(ans.second.size() > 0) {
			tokens.push_back(ans.second);
		}
	}

	if(tokens[0] == "exit" && tokens.size() == 1) {
		return 0;
	} else {
		return 1;
	}
}

void print(int i, const string &input) {
    std::vector<string> tokens;

    while(i < input.size()) { 

		i = skip_ws(i, input);
        if(i == input.size()) break;

        std::pair<int, string> ans = get_token(i, input);
        tokens.push_back(ans.second);
        
        i = ans.first;
    }

    for(int j = 0; j < tokens.size(); j++) {
        cout << tokens[j] << ' ';
    }

    cout << endl;
}

void exe(const string &input) {
	if(status(input) == 0) {
		exit(0);
	}

	string first_token;

	int i = 0;
	
	i = skip_ws(i, input);
	std :: pair<int, string> ans = get_token(i, input);
	i = ans.first;

	// doing the corresponded function
	if(ans.second == "echo") {
		print(i, input);
		return;
	}

	cout << input << ": command not found" << endl;
}

int main() {
	// Flush after every std::cout / std:cerr
	cout << std::unitbuf;
	cerr << std::unitbuf;

	// TODO: Uncomment the code below to pass the first stage
	while(1) {
		cout << "$ ";

		string input;
		getline(std :: cin, input);

		exe(input);
	}
}
