#include<iostream>
#include<string>
#include<vector>
#include<utility>
#include<unistd.h>
#include<filesystem>
#include<cstdlib>
#include<sstream>

using std :: cin;
using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

#if defined(_WIN32)
	const char PATH_DELIMETER = ';'; // this is for Windows
#else
	const char PATH_DELIMETER = ':'; // this is for Linux
#endif

const std :: vector<string> BUILTIN_COMMANDS = {"echo", "exit", "type"};
const std :: vector<string> LOCAL_PATH = {"D:\\Programming\\CP", "D:\\tro-choi-o-chu-ve-moi-truong-va-hoa-hoc\\phuong phap1"};
std :: vector<string> FETCH_PATH;

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

bool IsExecutable(const string &path) {
	return access(path.c_str(), X_OK) == 0;
}

void init_path() {
	const char *path_env = std :: getenv("PATH");

	// check if somehow the PATH does not exist
	if(path_env == nullptr) return;

	std :: string path_str(path_env); // this is equal to string path_str = path_env.string()
	std :: stringstream ss(path_str);
	std :: string dir;

	while(getline(ss, dir, PATH_DELIMETER)) {
		if(dir.size() > 0) {
			FETCH_PATH.push_back(dir);
		}
	}
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

	for(const auto &path : FETCH_PATH) {
		// check if this directory actually exist on the online disk
		if(!std :: filesystem :: exists(path)) {
			continue;
		}

		for(const auto &entry : std :: filesystem :: directory_iterator(path)) {
			string absolute_path = entry.path().string();
			string file_name = entry.path().filename().string();

			if(file_name == token && IsExecutable(absolute_path)) {
				cout << token << " is " << absolute_path << endl;
				return;
			}
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

	init_path();

	while(1) {
		cout << "$ ";

		string input;
		getline(std :: cin, input);

		exe(input);
	}
}
