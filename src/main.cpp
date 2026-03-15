#include <iostream>
#include <string>

using std :: cin;
using std :: cout;
using std :: cerr;
using std :: string;
using std :: endl;

int main() {
	// Flush after every std::cout / std:cerr
	cout << std::unitbuf;
	cerr << std::unitbuf;

	// TODO: Uncomment the code below to pass the first stage
	cout << "$ ";

	string input;
	getline(std :: cin, input);

	cout << input << ": command not found" << endl;
}
