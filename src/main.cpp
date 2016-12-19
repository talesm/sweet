#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "ConsoleEditor.hpp"
#include "MemoryTarget.hpp"

using namespace std;
using namespace sweet;

template<typename TARGET>
void run(string const &fileName);

int main(int argc, char **argv) {
	namespace po = boost::program_options;
	po::options_description options("Options");
	options.add_options()
			("help", "This help message")
			("version", "The version")
			("direct-mode,d", "Write every modification directly to file."
					" In this mode the file is automatically saved when"
					" closed. The file can be also saved to disc using the"
					" save command any time. It can be also be saved without"
					" any requisition depending of your underline platform."
			)
			("file", po::value<string>(), "The file to edit. You can omit the"
					" --file")
			;
	po::positional_options_description positional;
	positional.add("file", 1);

	po::variables_map programOptions;
	po::store(po::command_line_parser(argc, argv)
		.options(options).positional(positional).run(), programOptions);
	po::notify(programOptions);

	cout << "SWEET (Sweet and Weird Educational EdiTor)" << endl;
	if(programOptions.count("help")){
		cout << "Usage:" << endl;
		cout << "\t" << argv[0] << " {Options} [--[file]] file" << endl;
		cout << "\t" << argv[0] << " --help" << endl;
		cout << "\t" << argv[0] << " --version" << endl;
		cout << options << endl;
	} else if(programOptions.count("version")) {
		cout << "Version 0.2" << endl;
	} else if(programOptions.count("file")) {
		auto fileName = programOptions["file"].as<string>();
		if(programOptions.count("direct-mode")){
			run<FileTarget>(fileName);
		} else {
			run<MemoryTarget>(fileName);
		}
	} else {
		cerr << "Expected file name" << endl;
	}
	return 0;
}

template<typename TARGET>
void run(string const &fileName) {
	ConsoleEditor<TARGET> editor { fileName };
	editor.registerCustomCommand('q', [](const std::string&) {
		cout << "Exited Successfully" << endl;
		exit(0);
	});
	string line;
	do {
		editor.render(cout);
		do {
			cout << ">";
			getline(cin, line);
		} while (line.size() == 0);
	} while (editor.update(line));
}
