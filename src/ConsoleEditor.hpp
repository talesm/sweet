/**
 * @file FileEditor.h
 *
 * @date 2016-12-03
 * @author talesm
 */

#ifndef SRC_CONSOLEEDITOR_HPP_
#define SRC_CONSOLEEDITOR_HPP_

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "TargetTraits.hpp"

template<class TARGET>
inline std::string textViewTarget(TARGET &target, long pos, long size){
	return textViewTarget(target, pos, size, typename TargetTrait<TARGET>::category{});
}
template<class TARGET>
inline std::string textViewTarget(TARGET &target, long pos, long size, appendable_target_tag){
	auto currentPosition = target.tell();
	target.toStart();
	target.go(pos);
	std::string content;
	target.view(size, std::back_inserter(content));
	target.toStart();
	target.go(currentPosition);
	return content;
}
template<class TARGET>
inline std::string textViewTarget(TARGET &target, long pos, long size, insertable_target_tag){
	std::string content;
	target.viewRange(pos, size, std::back_inserter(content));
	return content;
}

/**
 * A basic file editor
 */
template<typename TARGET>
class ConsoleEditor {
public:
	using Command = std::function<void(std::string const &)>;

	ConsoleEditor(const std::string& fileName);
	/**
	 * Updates the file according with the given command.
	 * @param line the command
	 * @return true if it can receive a new command, false
	 * otherwise.
	 */
	bool update(std::string const &line);
	void render(std::ostream& out);

	/**
	 * Adds a custom command.
	 */
	void registerCustomCommand(char key, Command command) {
		commands[key] = command;
	}
private:
	TARGET target;
	std::unordered_map<char, Command> commands;

	/**
	 * @brief register a target method as a command
	 * @param key the key to trigger the command
	 * @param method the method to be called.
	 * @{
	 */
	template<typename RETURN>
	void registerMethod(char key, RETURN (TARGET::*method)() const);
	void registerMethod(char key, void (TARGET::*method)());
	void registerMethod(char key, void (TARGET::*method)(const std::string&));
	template<typename PARAM>
	void registerMethod(char key, void (TARGET::*method)(PARAM));
	void registerMethod(char key, void (TARGET::*method)(std::string::const_iterator, std::string::const_iterator &&));
	/// @}

	/**
	 * @brief Initialize the default commands
	 * @param the tag to tag dispatch. Use like this:
	 * `initCommands(typename TargetTraits<TARGET>::category{})`.
	 * @{
	 */
	void initCommands(appendable_target_tag);
	void initCommands(insertable_target_tag);
	void renderTagged(std::ostream& out, appendable_target_tag);
	void renderTagged(std::ostream& out, insertable_target_tag);
	///@}
};

template<typename TARGET>
inline ConsoleEditor<TARGET>::ConsoleEditor(const std::string& fileName) :
		target(fileName) {
	initCommands(typename TargetTrait<TARGET>::category { });
}

template<typename TARGET>
inline bool ConsoleEditor<TARGET>::update(std::string const &line) {
	using namespace std;
	char command = line.front();
	if (commands.count(command)) {
		commands[command](line);
	} else {
		std::cerr << "Command '" << command << "' unknown" << std::endl;
	}
	return true;
}

template<typename TARGET>
inline void ConsoleEditor<TARGET>::render(std::ostream& out) {
	std::string content = textViewTarget(target, 0, 60);
	for (auto &ch : content) {
		if (ch < 0x20 || ch >= 0x7f) {
			ch = '?';
		}
	}
	out << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
	out << content << std::endl;
	out << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
}

/**
 * A basic file editor
 */
template<typename TARGET>
template<typename RETURN>
inline void ConsoleEditor<TARGET>::registerMethod(char key, RETURN (TARGET::*method)() const) {
	commands[key] = [this, method](const std::string&) {
		std::cout << (target.*method)() << std::endl;
	};
}

/**
 * A basic file editor
 */
template<typename TARGET>
inline void ConsoleEditor<TARGET>::registerMethod(char key, void (TARGET::*method)()) {
	commands[key] = [this, method](const std::string&) {
		(target.*method)();
	};
}

/**
 * A basic file editor
 */
template<typename TARGET>
inline void ConsoleEditor<TARGET>::registerMethod(char key, void (TARGET::*method)(const std::string&)) {
	commands[key] = [this, method](const std::string& cmd) {
		(target.*method)(cmd.substr(1));
	};
}

/**
 * A basic file editor
 */
template<typename TARGET>
template<typename PARAM>
inline void ConsoleEditor<TARGET>::registerMethod(char key, void (TARGET::*method)(PARAM)) {
	commands[key] = [this, method](const std::string& cmd) {
		std::stringstream ss(cmd.substr(1));
		PARAM value;
		ss >> value;
		if (ss.fail()) {
			std::cerr << "Number expected" << std::endl;
		}
		(target.*method)(value);
	};
}

template<typename TARGET>
void ConsoleEditor<TARGET>::registerMethod(char key, void (TARGET::*method)(std::string::const_iterator , std::string::const_iterator &&)){
	commands[key] = [this, method](const std::string& cmd) {
		(target.*method)(cmd.begin()+1, cmd.end());
	};
}

/**
 * A basic file editor
 */
template<typename TARGET>
inline void ConsoleEditor<TARGET>::initCommands(insertable_target_tag) {
	initCommands(appendable_target_tag { });
	registerMethod('i', &TARGET::insert);
	registerMethod('d', &TARGET::erase);
}

/**
 * A basic file editor
 */
template<typename TARGET>
inline void ConsoleEditor<TARGET>::initCommands(appendable_target_tag) {
	registerMethod('t', &TARGET::tell);
	registerMethod('f', &TARGET::toStart);
	registerMethod('l', &TARGET::toEnd);
	registerMethod('g', &TARGET::go);
	registerMethod('w', &TARGET::replace);
	registerMethod('s', &TARGET::flush);
}

#endif /* SRC_CONSOLEEDITOR_HPP_ */
