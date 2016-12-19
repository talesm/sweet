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
#include <deque>
#include <iterator>
#include <memory>
#include <string>

#include "FileTarget.hpp"
#include "MemoryNode.hpp"
#include "TargetTraits.hpp"

/**
 * Represents a target in-memory.
 */
class MemoryTarget {
public:
	using category = insertable_target_tag;
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
	void view(size_t count, OUTPUT_ITERATOR out) const;

	/**
	 * @brief Returns a view.
	 * @param count the max number of characters.
	 */
	template<typename OUTPUT_ITERATOR>
	void viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out) const;

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
	template<typename FORWARD_ITERATOR>
	void replace(FORWARD_ITERATOR first, FORWARD_ITERATOR last);

	/**
	 * @brief Inserts a value on current position
	 * @param value what to insert.
	 *
	 * All contents after the current position are shifted
	 * to make space to the new content, so nothing is
	 * erased.
	 */
	template<typename FORWARD_ITERATOR>
	void insert(FORWARD_ITERATOR first, FORWARD_ITERATOR last);

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
	FileTarget internalTarget;
	size_t position, size_, originalSize;
	std::unique_ptr<MemoryNode> parent;
};

inline MemoryTarget::MemoryTarget(std::string const& filename) :
		internalTarget(filename), position(0) {
	internalTarget.toEnd();
	originalSize = size_ = internalTarget.tell();
	internalTarget.toStart();
	parent = std::make_unique<MemoryNode>(position, size_);
}

/**
 * @brief Returns a view.
 * @param count the max number of characters.
 */
template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::view(size_t count, OUTPUT_ITERATOR out) const {
	viewRange(position, count, out);
}

/**
 * @brief Returns a view.
 * @param count the max number of characters.
 */
template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out) const {
	parent->viewRange(pos, count, out, internalTarget);
}

/**
 * @brief Returns a view.
 * @param count the max number of characters.
 */
template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::viewAll(OUTPUT_ITERATOR&& out) const {
	viewRange(0, size_, out);
}

inline size_t MemoryTarget::size() const {
	return size_;
}

/**
 * @brief Replace the the content on current position.
 * @param value value to replace
 *
 * It starts at the current position and advances it
 * until it uses all value.size() characters.
 */
template<typename FORWARD_ITERATOR>
inline void MemoryTarget::replace(FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
	parent->replace(position, first, last);
	position += std::distance(first, last);
	if (position > size_) {
		size_ = position;
	}
}

/**
 * @brief Inserts a value on current position
 * @param value what to insert.
 *
 * All contents after the current position are shifted
 * to make space to the new content, so nothing is
 * erased.
 */
template<typename FORWARD_ITERATOR>
inline void MemoryTarget::insert(FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
	parent->insert(position, first, last);
	auto incr = std::distance(first, last);
	position += incr;
	size_ += incr;
}

inline void MemoryTarget::erase(size_t count) {
	parent->erase(position, count);
	size_ -= count;
}

inline void MemoryTarget::flush() {
	internalTarget.toStart();
	parent->flush(internalTarget);
	if(size() < originalSize){
		internalTarget.shrink();
	}
	internalTarget.flush();
}

inline size_t MemoryTarget::tell() const {
	return position;
}

inline void MemoryTarget::toEnd() {
	position = size_;
}

inline void MemoryTarget::toStart() {
	position = 0;
}

inline void MemoryTarget::go(ptrdiff_t offset) {
	position += offset;
}

#endif /* SWEET_MEMORYTARGET_HPP_ */

