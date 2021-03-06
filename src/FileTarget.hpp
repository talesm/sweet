/**
 * @file FileTarget.hpp
 *
 * @date 2016-11-25
 * @author talesm
 */

#ifndef SWEET_FILETARGET_HPP_
#define SWEET_FILETARGET_HPP_

#include <cerrno>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "TargetTraits.hpp"

namespace sweet {

/**
 * A file target that can be read and written.
 */
class FileTarget {
public:
	using category = appendable_target_tag;
public:
	/**
	 * @brief Load the target from a file.
	 * @param filename
	 */
	FileTarget(std::string const& filename);

	/**
	 * Dtor. Closes the file
	 */
	~FileTarget();

	//We cant have these.
	FileTarget(FileTarget const&) = delete;
	FileTarget(FileTarget &&) = delete;
	FileTarget &operator=(FileTarget const&) = delete;
	FileTarget &operator=(FileTarget &&) = delete;

	/**
	 * @brief Copies `count` characters to an output iterator.
	 * @param count
	 * @param out
	 */
	template<typename OUTPUT_ITERATOR>
	void view(long count, OUTPUT_ITERATOR &&out) const;


	template<typename OUTPUT_ITERATOR>
	void viewRange(long pos, long count, OUTPUT_ITERATOR &&out) const;

	/**
	 * @brief  Return our current position
	 */
	long tell() const;

	/**
	 * @brief Goes to first position
	 */
	void toStart();

	/**
	 * @brief Goes to last position
	 */
	void toEnd();

	/**
	 * @brief Offsets position
	 * @param value
	 */
	void go(long value);

	/**
	 * @brief Writes the given content.
	 *
	 * It overwrites the current content and appends to end if necessary.
	 * @param first
	 * @param last
	 */
	template<typename INPUT_ITERATOR>
	void replace(INPUT_ITERATOR first, INPUT_ITERATOR last);
	void flush();

	void shrink();

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

template<typename OUTPUT_ITERATOR>
inline void FileTarget::view(long count, OUTPUT_ITERATOR &&out) const {
	int ch;
	while ((ch = getc(file)) != EOF && count-- > 0) {
		*out++ = ch;
	}
}

template<typename OUTPUT_ITERATOR>
inline void FileTarget::viewRange(long pos, long count, OUTPUT_ITERATOR &&out) const {
	fseek(file, pos, SEEK_SET);
	int ch;
	while ((ch = getc(file)) != EOF && count-- > 0) {
		*out++ = ch;
	}
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

template<typename INPUT_ITERATOR>
inline void FileTarget::replace(INPUT_ITERATOR first, INPUT_ITERATOR last) {
	static_assert(
			std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<INPUT_ITERATOR>::iterator_category>::value,
			"first and last parameters must be input iterators"
	);
	while(first != last){
		auto result = fputc(*first++, file);
		if(result == EOF){
			throw std::runtime_error("Some error occurred, can't write.");
		}
	}
}

inline void FileTarget::flush() {
	fflush(file);
}

inline void FileTarget::shrink() {
	auto pos = tell();
	toStart();
	std::string content;
	view(pos, std::back_inserter(content));
	if(!freopen(nullptr, "w+", file)){
		throw new std::runtime_error("Can not shrink");
	}
	replace(content.begin(), content.end());
}

}

#endif /* SWEET_FILETARGET_HPP_ */
