SWEET (Sweet and Weird Educational EdiTor)
==========================================

This project is set to make a text editor with educational purpose, i.e, to known how one generally works.
You may want to see our [main][] site before begin.

Goals
=====

Besides its educational purpose, we want to make a fully functional editor, so we want:

- Ability to open, close, edit them; 
- Ability to work with different text encodings, to convert among them and to reasonably detect them;
- Support to syntax highlight and section folding through some extension mechanism;
- Support for text completion, also through some extension mechanism.
- Ability to *scriptize* (through macro or something like that);

We will focus on the inner working of the editor (their data structures and stuff like that), so our main 
artifact produced will mostly be a library. We also produce the lessons itself, explaining its particular focus.
Finally we will have at least one *reference implementation* editor, who implements the library into a somewhat
useful editor. 

Development Strategy
====================

Our development strategy is to develop both code and the current lesson directly on 
branch master. When a lesson is ready, it is then moved to its own branch.  

Lesson 1 - File Target
======================

Where do we begin? The first thing to do is a *target* class, which 
represents the text we are editing. It could also be called Buffer, 
File etc, but these names already represent something on the system, so... 
we'll call it target. 

We start simple and for the first lesson we'll have a target that just wraps
a file. This class needs to present methods to do just these:

- to view the file content;
- to know our current cursor position;
- to be able to move the cursor position;
- be able to write on it;   
- to save.

For now we will treat the file as an unidimensional array of chars. We also will aim to support 
only the basic ASCII for now, as unicode is complicated and using anything else would be madness.
Our class would look like this:

```cpp
class FileTarget {
public:
	FileTarget(std::string const& filename);
	~FileTarget();

	std::string view(long count) const;
	long tell() const;
	void toStart();
	void toEnd();
	void go(long value);
	void write( std::string const& value);
	void flush();

private:
	FILE *file;
};
``` 

We called it FileTarget because we may have some new target types in the future, 
so we save ourselves the trouble of renaming this file. We will use FILEs instead 
of fstreams because their interface is simpler. Our constructor only needs to do 
a fopen and our destructor need to close it. Pretty simple. 

The view method is const as it just lets us to see (some of) the file contents.
It starts on the character under our cursor and shows n characters. We will have to
revisit it in the future, but for now we could define it as:

```cpp
inline std::string FileTarget::view(long count) const {
	std::string buffer;
	int ch = getc(file);
	while (ch != EOF && count > 0) {
		buffer += ch;
		--count;
		ch = getc(file);
	}
	return buffer;
}
```

We just need to check against EOF, to avoid some weird stuff in case we pass it.
Otherwise we just call getc for *count* times. It **has** an obvious undesirable 
effect, can you spot it?

Another very important method is the write, where the user input is put on our
file.

```cpp
inline void FileTarget::write(std::string const& value) {
	auto r = fwrite(value.data(), 1, value.size(), file);
	if (r != value.size()) {
		std::cerr << "Some error occurred, can't write." << std::endl;
	}
}
```

It is important to  notice that **it just replaces the text**. If it gets to
the end of file it then appends to the file, but otherwise it can't insert text.
It makes it less useful of an editor for now, but we could use it to make a sort 
of binary editor. 

The method, tell(), is very straightforward, it just calls `ftell(file)` and 
return its value.

The methods toStart(), toEnd() and go() let us to move our cursor. all of them just 
use fseek(). The first two call `fseek(file, 0, SEEK_SET)` and 
`fseek(file, 0, SEEK_END)`, respectively. The third accept a *value* as parameter 
and so it calls `fseek(file, value, SEEK_CUR)`.

The flush() saves the content to the disc. It is not very important for now, as we
are already writing directly on the file and so it is flushed automatically on the 
fclose() call we do on our destructor.

Our "Editor"
------------

We don't have support for lines or text insertion by now, so we can't even do much 
stuff by now. So our editor will just show the capabilities we have now: 

1. Load the file name passed as parameter and initialize our Target with it.
2. Then, read a line, identify its command:
	- 'q', quits the program;
	- 'v', Shows `number` of character of text, starting at current cursor. 
	- 't', then tells the current cursor position;
	- 'f', go to the start position;
	- 'l', go to the last position;
	- 'g', advances cursor position `number` characters; 
	- 'w', writes the remaining of the line on the target;
	- 's', flushes the content to disc ("save");
	- Anything else prints an error.
3. Repeat until we get the 'q' command.


So, our first app would be like this:

```cpp
int main(){
	/* ... */
	for(;;) {
		string line;
		getline(cin, line);
		char command = line.front();
		switch (command) {
		case 'q':
			return 0;
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
			target.overwrite(line.substr(1));
			break;
		case 's':
			target.flush();
			break;
		default:
			cerr << "Command Unknown" << endl;
			break;
		}
	}
}
```

We could use some more error checking, but our app will be completely
rewrote in near future, so better not feel to much attached to it.

Final thoughts
--------------

We have a very very basic Target now, but it have some very basic flaws:

1. it can't insert text, just overwrite; 
2. It can't discard changes; 
3. As hinted above, when we view the text, we move the cursor.

The 3 is easily solvable, we could just save the current position before
the view happens and then go back to it, and doing so is a good exercise 
to do. But our [next lesson][lesson2] will show that is possible to get a better
solution, which also solves 1 and 2; 

See the code at [code], put comments, couts, edit... thinker with it a 
little, it is the best way to learn.


[main]: TODO
[lesson2]: TODO
[code]: src/
