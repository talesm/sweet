/**
 * @file MemoryTarget..hpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#ifndef SWEET_MEMORYTARGET_HPP_
#define SWEET_MEMORYTARGET_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>

#include "FileTarget.hpp"

/**
 * Represents a target in-memory.
 */
class MemoryTarget {
public:
	/**
	 * @brief Ctor
	 * @param filename
	 */
	MemoryTarget(std::string const& filename);

	/**
	 * @brief Returns a view.
	 * @param count the max number of characters.
	 */
	template<typename OUTPUT_ITERATOR>
	void view(size_t count, OUTPUT_ITERATOR &&out) const;

	/**
	 * @brief Returns a view.
	 * @param count the max number of characters.
	 */
	template<typename OUTPUT_ITERATOR>
	void viewRange(size_t pos, size_t count, OUTPUT_ITERATOR&& out) const;

	/**
	 * @brief Returns a view.
	 * @param count the max number of characters.
	 */
	template<typename OUTPUT_ITERATOR>
	void viewAll(OUTPUT_ITERATOR&& out) const;

	/**
	 * @brief Tells the number of characters.
	 * @return the size
	 */
	size_t size() const;

	/**
	 * @brief Replace the the content on current position.
	 * @param value value to replace
	 *
	 * It starts at the current position and advances it
	 * until it uses all value.size() characters.
	 */
	template <typename FORWARD_ITERATOR>
	void replace(FORWARD_ITERATOR &&first, FORWARD_ITERATOR &&last);

	/**
	 * @brief Inserts a value on current position
	 * @param value what to insert.
	 *
	 * All contents after the current position are shifted
	 * to make space to the new content, so nothing is
	 * erased.
	 */
	template <typename FORWARD_ITERATOR>
	void insert(FORWARD_ITERATOR &&first, FORWARD_ITERATOR &&last);

	/**
	 * Erase characters
	 * @param count the quantity to erase.
	 *
	 */
	void erase(size_t count);

	void flush();

	/**
	 * Tells the current position
	 * @return the current position
	 */
	size_t tell() const;

	/**
	 * @brief Goes to the end of the file
	 */
	void toEnd();

	/**
	 * @brief Goes to the begin of the file.
	 */
	void toStart();

	/**
	 * @brief Advances the position
	 * @param offset the number of character to advance. May be negative.
	 */
	void go(ptrdiff_t offset);
private:
	std::string content;
	FileTarget internalTarget;
	size_t position;
};

inline MemoryTarget::MemoryTarget(std::string const& filename) :
		internalTarget(filename), position(0) {
	internalTarget.toEnd();
	long count = internalTarget.tell();
	internalTarget.toStart();
	content.reserve(count);
	internalTarget.view(count, std::back_inserter(content));
}

template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::view(size_t count, OUTPUT_ITERATOR &&out) const {
	viewRange(position, count, out);
}

/**
 * @brief Returns a view.
 * @param count the max number of characters.
 */
template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::viewRange(size_t pos, size_t count, OUTPUT_ITERATOR&& out) const {
	auto first = content.begin() + std::min(pos, content.size());
	if (first == content.end()) {
		return;
	}
	auto last = first + std::min(count, content.size() - pos);
	std::copy(first, last, out);
}

/**
 * @brief Returns a view.
 * @param count the max number of characters.
 */
template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::viewAll(OUTPUT_ITERATOR&& out) const {
	std::copy(content.begin(), content.end(), out);
}

inline size_t MemoryTarget::size() const {
	return content.size();
}

template <typename FORWARD_ITERATOR>
inline void MemoryTarget::replace(FORWARD_ITERATOR &&first, FORWARD_ITERATOR &&last) {
	using namespace std;
	size_t remainingSize = content.size() - position;
	if (std::distance(first, last) <= remainingSize) {
		auto result = copy(first, last, content.begin() + position);
		position = result - content.begin();
	} else {
		string::const_iterator transition_point;
		transition_point = first + remainingSize;
		copy(first, transition_point, content.begin() + position);
		content.append(transition_point, last);
		position = content.size();
	}
}

template <typename FORWARD_ITERATOR>
inline void MemoryTarget::insert(FORWARD_ITERATOR &&first, FORWARD_ITERATOR &&last) {
	auto incr = std::distance(first, last);
	content.insert(content.begin() + position, first, last);
	position += incr;
}

inline void MemoryTarget::erase(size_t count) {
	auto first = content.begin() + position;
	auto last = first + count;
	content.erase(first, last);
}

inline void MemoryTarget::flush() {
	internalTarget.toStart();
	internalTarget.replace(content.begin(), content.end());
	internalTarget.shrink();
	internalTarget.flush();
}

inline size_t MemoryTarget::tell() const {
	return position;
}

inline void MemoryTarget::toEnd() {
	position = content.size();
}

inline void MemoryTarget::toStart() {
	position = 0;
}

inline void MemoryTarget::go(ptrdiff_t offset) {
	position += offset;
	position = std::min(position, content.size());
}

#endif /* SWEET_MEMORYTARGET_HPP_ */

