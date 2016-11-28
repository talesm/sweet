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
		string line;
		cout << ">";
		getline(cin, line);
		if (line.size() == 0) {
			continue;
		}
		char command = line.front();
		switch (command) {
		case 'q':
			break;
		case 'v':
			cout << target.view(stol(line.substr(1))) << endl;
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
			target.write(line.substr(1));
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
