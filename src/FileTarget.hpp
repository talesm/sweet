/**
 * @file FileTarget.hpp
 *
 * @date 2016-11-25
 * @author talesm
 */

#ifndef SRC_FILETARGET_HPP_
#define SRC_FILETARGET_HPP_

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>

/**
 * A file target that can be read and wrote.
 */
class FileTarget {
public:
	FileTarget(std::string const& filename);
	~FileTarget();
	FileTarget(FileTarget const&) = delete;
	FileTarget(FileTarget &&) = delete;
	FileTarget &operator=(FileTarget const&) = delete;
	FileTarget &operator=(FileTarget &&) = delete;

	std::string view(long count) const;
	long tell() const;
	void toStart();
	void toEnd();
	void go(long value);
	void write(std::string const& value);
	void flush();

private:
	FILE *file;
};

inline FileTarget::FileTarget(std::string const& filename) {
	file = fopen(filename.c_str(), "rb+");
	if (!file) { //Maybe it does not exist, so we try to create a new
		file = fopen(filename.c_str(), "wb+");
		if (!file) { //If it still doesn't exist, we quit.
			throw std::runtime_error("Error opening '" + filename + "': " + std::strerror(errno));
		}
	}
}

inline FileTarget::~FileTarget() {
	fclose(file);
}

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

inline long FileTarget::tell() const {
	return ftell(file);
}

inline void FileTarget::toStart() {
	fseek(file, 0, SEEK_SET);
}

inline void FileTarget::toEnd() {
	fseek(file, 0, SEEK_END);
}

inline void FileTarget::go(long value) {
	fseek(file, value, SEEK_CUR);
}

inline void FileTarget::write(std::string const& value) {
	auto r = fwrite(value.data(), 1, value.size(), file);
	if (r != value.size()) {
		throw std::runtime_error("Some error occurred, can't write.");
	}
}

inline void FileTarget::flush() {
	fflush(file);
}

#endif /* SRC_FILETARGET_HPP_ */
