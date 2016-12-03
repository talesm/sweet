#include <cstdlib>
#include <iostream>
#include <string>

#include "ConsoleEditor.hpp"
#include "MemoryTarget.hpp"

using namespace std;

int main(int argc, char **argv) {
	if (argc < 2) {
		cerr << "Expected file name" << endl;
		return 1;
	}
	ConsoleEditor<MemoryTarget> editor {argv[1] };
	editor.registerCustomCommand('q', [](std::string const &){
		cout << "Exited Successfully" << endl;
		exit(0);
	});
	string line;
	do{
		editor.render(cout);
		do {
			cout << ">";
			getline(cin, line);
		}
		while (line.size() == 0);
	} while(editor.update(line));
	return 0;
}
