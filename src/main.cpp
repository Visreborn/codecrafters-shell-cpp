#include<iostream>
#include<string>
#include<vector>
#include<utility>

using std :: cin;
using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

const std :: vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type"};

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

void handle_type(int i, const string &input) {
	i = skip_ws(i, input);
	auto [_, token] = get_token(i, input);

	for(auto &builtin : BUILTIN_COMMANDS) {
		if(builtin == token) {
			cout << token << " is a shell builtin" << endl;
			return;
		}
	}

	cout << token << ": not found" << endl;
}

void exe(const string &input) {
	if(status(input) == 0) {
		exit(0);
	}

	int i = 0;
	string first_token;
	
	i = skip_ws(i, input);
	auto [a, b] = get_token(i, input);

	i = a;
	first_token = b;

	// doing the corresponded function
	if(first_token == "echo") {
		print(i, input);
		return;
	} else if(first_token == "type") {
		handle_type(i, input);
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
