SWEET (Sweet and Weird Educational EdiTor)
==========================================

This project is set to make a text editor with educational purpose, i.e, to known how one generally works.
You may want to see our [main][] site before begin.

Lesson 2 - How to Inserting content
==================================

On the [previous lesson](lesson1), we created a basic file target. but
we have some issues hanging there, as we can't insert text, just 
overwrite it or  append at the end. In this lesson we still treat the
text as a single giant line, but we will see how to insert content 
anywhere there.

Until now we are writing direct to disc. It has the problems of 
1) performance related to disc access and 2) the fact that a file 
grows only in one direction (on the end). The first one is not a great
deal nowadays, as your OS is made by very smart people that reads the
file to the memory and sync it with your disc regularly. But the second
one... it is the one we have to deal. The solution is to put the file
content (or some large chunk of it. More about it later) on memory.

The simplest solution is using our `view()` method to get our contents
and then operate on them, so lets start with that. we create our class
MemoryTarget as follows:

```cpp
class Target {
public:
	MemoryTarget(std::string const &filename);
	std::string view(size_t pos, size_t count) const;
	std::string view(size_t count) const;
	std::string viewAll() const;
	/* ... */
	void insert(std::string const &value);
	void replace(std::string const &value);
	void erase(std::ptrdiff_t count);
	/* ... */	
private:
	FileTarget target;
	std::string content;
	size_t position;
};
```

Our constructor will copy the entire content, more or less as follows:

```cpp
inline MemoryTarget::MemoryTarget(const std::string& filename):
		internalTarget(filename), position(0){
	internalTarget.toEnd();
	long count = internalTarget.tell();
	internalTarget.toStart();
	content = internalTarget.view(count);
}
```

We use the old jump-to-last-to-see-the-size trick here, so we don't have to rely 
on os-dependent solution to see the size (jump to end, get position, that is the
current size, then jump back to the start). We use view() with the size to get
the entire content. Copying it to content is not as bad as it seems, as it will
just use move semantics in that case. 

View
----

The viewing is very alike the FileTarget. But it does not move the pointer
and we added a new overload that allows totally arbitrary values:
```cpp
inline std::string MemoryTarget::view(size_t count) const {
	return view(position, count);
}

inline std::string MemoryTarget::view(size_t pos, size_t count) const {
	return content.substr(pos, count);
}

inline std::string MemoryTarget::viewAll() const {
	return content;
}
```

Replacing
---------

The replace method shouldn't be that complicated. But we should factor that it
should respect the position and if the given value is larger than what we have, 
it should enlarge the content, so we have this

```cpp
inline void MemoryTarget::replace(const std::string& value) {
	using namespace std;
	size_t remainingSize = content.size() - position;
	if (value.size() <= remainingSize) {
		auto result = copy(value.begin(), value.end(), content.begin() + position);
		position = result - content.begin();
	} else {
		string::const_iterator first = value.begin();
		string::const_iterator last;
		last = first + remainingSize;
		copy(first, last, content.begin() + position);
		content.append(last, value.end());
		position = content.size();
	}
}
```

The common case, when the file is not increased is fairly simple, just
`copy()` the value (through `begin()` to `end()`) to the current 
position (`content.begin() + position`). 

Otherwise, we check the how many chars we can replace and get an iterator 
to the exact position where we have to start appending (`transition_point`).
Then we copy from `begin()` to `transition_point` and then append from 
`transition_point` to the `end()`. 

Inserting
---------
Well this will be ugly... Or will it? The implementation can be as easy as follows:

```cpp
inline void MemoryTarget::insert(std::string const& value) {
	content.insert(content.begin() + position, value.begin(), value.end());
	position += value.size();
}
```

This implementation *works*, and is the correct structure for what we have now.
But there is a problem with it, logical and performance one. Strings are just the 
wrong tool for the job because the string are contiguous data, we have to copy
all characters after the insertion point to make up space to the inserted text.

There are better solutions than that and we will cover some of them, but for now
we will finish the interface and get to test it.

Erasing
-------

Similarly to insert, our erase is as follows:

```
inline void MemoryTarget::erase(size_t count) {
	auto first = content.begin() + position;
	auto last = first + count;
	content.erase(first, last);
}
```

Simple. But it has a analogous problem to `insert()`, all the characters right
to the erased have to be shifted to fill the space left by the erased ones.

Other Stuff
-----------

The methods `toStart()`, `toEnd()`  and `tell()` are very simple, the first just 
sets position to 0, the second sets it to `content.size()` and the last one
return the position. The `go()` is easy, but we must remember to check if
it is within the bounds of content.

The flush is also simple by itself, but requires a small modification on the
FileTarget.

```cpp
inline void MemoryTarget::flush() {
	internalTarget.toStart();
	internalTarget.replace(content);
	internalTarget.shrink();
	internalTarget.flush();
}
```

The modification, is the shrink() command, that should shrink the file to the 
current position and... It is *impossible* to do so on current c spec. There 
are some platform-dependent ways to solve that, like Windows' `SetEndOfFile()` 
or posix's `ftruncate()` and we will talk more about them later, but for now we
can do following:

```cpp
inline void FileTarget::shrink() {
	auto pos = tell();
	toStart();
	std::string content = view(pos);
	if(!freopen(nullptr, "w+", file)){
		throw new std::runtime_error("Can not shrink");
	}
	replace(content);
} 
```

In that code, we load the content up to the current position and store on
`content`. Then we reopen it on "w" mode, which sets its size to zero. 
Then we can write our `content` back to the file. 

We also renamed the method `write()` to `toEnd()`, respectively, 
what will be handy in next section.

Testing Program
---------------

To test, we update the program from the last lesson with a twist to allow us
to still test our last version and then compare both. At first we need to move 
the code to a class, so we do our ConsoleEditor:

```cpp
class ConsoleEditor {
public:
	ConsoleEditor(std::string const& fileName);
	void update(std::string const &line);
	void render();
private:
	FileTarget target;
};
```

The constructor will just forward to target. The update could be basically the 
old switch, minus the `v` command:

```cpp
inline void ConsoleEditor::update(std::string const &line) {
	using namespace std;
	char command = line.front();
	switch (command) {
	case 'q':
		exit(0);
		return;
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
		target.replace(line.substr(1));
		break;
	case 's':
		target.flush();
		break;
	default:
		cerr << "Command Unknown" << endl;
		break;
	}
	return true;
}
```

The `q` command now return false to signal the end of the program.

Why not the `v` command? Because now we do (almost) like any editor and show
the content just after each command. It is not perfect yet, but is better:

```cpp
inline void ConsoleEditor::render(std::ostream& out) {
	auto currentPosition = target.tell();
	target.toStart();
	std::string content = target.view(60);
	for(auto &ch: content){
		if(ch < 0x20 || ch >= 0x7f){
			ch = '?';
		}
	}
	out << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
	out << content << std::endl;
	out << "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" << std::endl;
	target.toStart();
	target.go(currentPosition);
}
```

This not only prints the chars, as it also ensures it is entirely on the 
allowed range \[0x20, 0x7f\). It replaces it for a '?' otherwise. The good 
thing is that this transformation only occurs on the view. If you save it still
have the original character. We only show the first 60 characters, as it is a
good guess on terminal length (Most of them have at least 80).

I don't like to use switch that much, it is easy to get something wrong and
very repetitive. Also, there is no guarantee it will optimize it. So we could
use something like a map or unordered_map here. Ideally we would map from the
char to the methods directly. But while most of our methods has the same 
signature, some of them, like `go()` and `replace()` are different. So instead
We will wrap them. We need just modify our class like this:

```cpp
class ConsoleEditor {
public:
	using Command = std::function<void(std::string)>;
	/* ... */
private:
	/* ... */
	std::unordered_map<char, Command> commands;
	template<typename RETURN>
	void registerMethod(char key, RETURN (TARGET::*method)() const);

	void registerMethod(char key, void (TARGET::*method)());

	void registerMethod(char key, void (TARGET::*method)(const std::string&));

	template<typename PARAM>
	void registerMethod(char key, void (TARGET::*method)(PARAM));
};
```

The register* methods help us by wrapping each signature in a lambda to fit
the Command interface.

Supporting MemoryTarget
-----------------------

Ok, so now we have the ConsoleEditor supporting FileTarget, we must create the
one to support MemoryTarget, right? We could instead make it an abstract base
class and create derived ones like FileEditor and MemoryEditor. But as both 
classes would be very similar (and I feel like it), we will just make 
ConsoleEditor a template.

One template parameter is very obvious, TARGET which represents the type of
our target, so let start with it and  add `template <typename TARGET>` on top
of ConsoleEditor, replace FileTarget to TARGET on the body and adjust our method 
definitions accordingly. 

Now we can run both *targets*. But we don't have access yet to our methods `insert()` and
`erase()`. Now we have to do a template trick called [tag dispatch](). First we
create (before the TargetTraits) these structs:

```cpp
struct writeable_target_tag {};
struct insertable_target_tag: public writeable_target_tag {};
```

Then we implement a TargetTrait class, as the following:

```cpp
template<typename TARGET>
struct TargetTrait; // Causes an error if called upon unsupported

template<>
struct TargetTrait<FileTarget>{
	using category = writeable_target_tag;
};

template<>
struct TargetTrait<MemoryTarget>{
	using category = insertable_target_tag;
};
```

If you are not familiar with this idiom, it enable us to select different methods
at compile time. In our example, we will declare the initCommands() methods at
our ConsoleEditor:

```cpp
template <typename TARGET>
class ConsoleEditor {
	/*...*/
	void initCommands(writeable_target_tag);
	void initCommands(insertable_target_tag);
};
```

Our ctor now doesn't add commands anymore, but just call initCommands().

```cpp
template<typename TARGET>
inline ConsoleEditor<TARGET>::ConsoleEditor(const std::string& fileName) :
		target(fileName) {
	initCommands(typename TargetTrait<TARGET>::category { });
}
```

Because the way we specialized the TargetTrait, if the TARGET is a FileTarget
the TargetTrait<TARGET>::category will be an alias to writeable_target_tag, so
our code basically will call the first version. If TARGET is a MemoryTarget, 
TargetTrait<TARGET>::category will be an alias to insertable_target_tag and thus
it will call the second version. So we are abusing the template  and the
overloading systems to fun and profit. So cool!

Now we just have to re-write our `main()` to be able to use both versions. We do
that by using a command line switch: if the user passes -d it will use the
FileTarget, otherwise we use the MemoryTarget. Parsing the command line is not
difficult right now, but will probably get more complex in the future, so we 
will use the [Boost.Program_options][]. Our main is now as follows:

```cpp
int main(int argc, char **argv) {
	namespace po = boost::program_options;
	
	/*Declaring options*/
	po::options_description options("Options");
	options.add_options()
			("help", "This help message")
			("version", "The version")
			("direct-mode,d", "Write every modification directly to file.")
			("file", po::value<string>(), "The file to edit. You can omit the --file")
			;
	po::positional_options_description positional;
	positional.add("file", 1);
	po::variables_map programOptions;
	po::store(po::command_line_parser(argc, argv)
		.options(options).positional(positional).run(), programOptions);
	po::notify(programOptions);
	
	/*Parsing options*/
	if(programOptions.count("help")){
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
```

And our run method would be like the following:
```cpp
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
```

Final Thoughts
==============

We now have a much more featured editor in action. We can edit it as we please
and it will only persist when (and if) we deliberately push the save command.
We do not warn when the user exits the program, it could be a useful exercise
to implement it yourself. Our own implementation will only occurs in a future 
lesson after we solve some more pressing problems. Another challenge for you 
is that we just show up to 60 first characters. We should have at least a way 
to offset it and it should be a good exercise to do it.

In the [next lesson][lesson3] we will learn what are [rope][]s and why they're 
indicated to hold our format better than contiguous strings we've been using.
Until then you can check the new [benchmark](./benchmark) directory to see some
performance comparisons between our implemented targets up to now.

[main]: https://talesm.github.io/sweet/
[lesson1]: ../lesson1/README.md
[lesson3]: ../lesson2/README.md
[code]: src/
[cmake]: https://cmake.org/
[cmakelists]: ./CMakeLists.txt
[rope]: https://en.wikipedia.org/wiki/Rope_(data_structure)
[Boost.Program_options]: http://www.boost.org/doc/libs/1_62_0/doc/html/program_options.html
