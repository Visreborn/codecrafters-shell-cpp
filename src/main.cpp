#include <iostream>
#include <string>
#include <vector>

using std :: cin;
using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

bool status(string input) {
	std :: vector<string> tokens;

	int i = 0; 

	while(i < input.size()) {
		string tmp;

		while(i < input.size() && input[i] != ' ') {
			tmp += input[i];
			i ++;
		}

		if(tmp.size() > 0) {
			tokens.push_back(tmp);
		}

		i ++;
	}

	if(tokens[0] == "exit" && tokens.size() == 1) {
		return 0;
	} else {
		return 1;
	}
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

		if(status(input) == 0) {
			return 0;
		}

		cout << input << ": command not found" << endl;
	}
}
