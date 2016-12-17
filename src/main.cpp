#include <iostream>
#include <string>

#include "FileTarget.hpp"

using namespace std;

int main(int argc, char **argv) {
	if (argc < 2) {
		cerr << "Expected file name" << endl;
		return 1;
	}
	FileTarget target { argv[1] };

	for(;;) {
		string line, buffer;
		cout << ">";
		getline(cin, line);
		if (line.size() == 0) {
			continue;
		}
		char command = line.front();
		switch (command) {
		case 'q':
			return 0;
		case 'v':
			target.view(stol(line.substr(1)), std::back_inserter(buffer));
			cout << buffer << endl;
			break;
		case 't':
			cout << target.tell() << endl;
			break;
		case 'f':
			target.toStart();
			break;
		case 'l':
			target.toEnd();
			break;
		case 'g':
			target.go(stol(line.substr(1)));
			break;
		case 'w':
			target.replace(line.begin()+1, line.end());
			break;
		case 's':
			target.flush();
			break;
		default:
			cerr << "Command Unknown" << endl;
			break;
		}
	}
	return 0;
}
