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
#include "TargetTraits.hpp"

struct MemoryNode {
	enum Type {
		BRANCH,
		ORIGINAL_LEAF,
		MODIFIED_LEAF,
	} type;
	union {
		struct {
			std::unique_ptr<MemoryNode> left;
			std::unique_ptr<MemoryNode> right;
			size_t weight;
		} branch;
		struct {
			size_t offset;
			size_t size;
		} original;
		struct {
			std::deque<char> content;
		} modified;
	};

	MemoryNode(size_t offset, size_t size) {
		type = ORIGINAL_LEAF;
		original.offset = offset;
		original.size = size;
	}

	MemoryNode(std::deque<char> &&content) {
		type = MODIFIED_LEAF;
		new (&modified.content) std::deque<char>(move(content));
	}

	~MemoryNode() {
		switch (type) {
		case BRANCH:
			branch.left.~unique_ptr();
			branch.right.~unique_ptr();
			break;
		case ORIGINAL_LEAF:
			break;
		case MODIFIED_LEAF:
			modified.content.~deque();
			break;
		}
	}

	template<typename OUTPUT_ITERATOR>
	void viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out, FileTarget const &internalTarget) const {
		switch (type) {
		case BRANCH:
			if(pos < branch.weight){
				branch.left->viewRange(pos, std::min(branch.weight, count), out, internalTarget);
			}
			if(pos + count > branch.weight){
				branch.right->viewRange(0, count - branch.weight, out, internalTarget);
			}
			break;
		case ORIGINAL_LEAF:
			internalTarget.viewRange(pos+original.offset, count, out);
			break;
		case MODIFIED_LEAF:
			std::copy(modified.content.begin(), modified.content.end(), out);
			break;
		}
	}

	void split(size_t pos){
		using namespace std;
		switch (type) {
		case BRANCH:
			throw std::logic_error("Unimplemented");
		case ORIGINAL_LEAF:{
			auto first = original.offset;
			auto middle = pos;
			auto last = first + original.size;
			auto right = make_unique<MemoryNode>(middle, last - middle);
			type = BRANCH;
			new (&branch.left) unique_ptr<MemoryNode>(new MemoryNode(first, middle - first));
			new (&branch.right) unique_ptr<MemoryNode>(new MemoryNode(middle, last - middle));
			branch.weight = middle - first;
			break;
		}
		case MODIFIED_LEAF:{
			auto leftContent = move(modified.content);
			deque<char> rightContent(leftContent.begin()+pos, leftContent.end());
			leftContent.erase(leftContent.begin()+pos, leftContent.end());
			modified.content.~deque();

			type = BRANCH;
			new (&branch.left) unique_ptr<MemoryNode>(new MemoryNode(move(leftContent)));
			new (&branch.right) unique_ptr<MemoryNode>(new MemoryNode(move(rightContent)));
			branch.weight = branch.left->modified.content.size();
			break;
		}
		}
	}

	template<typename FORWARD_ITERATOR>
	void replace(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
		using namespace std;
		switch (type) {
		case BRANCH:
			if(pos < branch.weight){
				if(distance(first, last) <= ptrdiff_t(branch.weight)){
					branch.left->replace(pos, first, last);
				} else {
					auto middle = next(first, branch.weight);
					branch.left->replace(pos, first, middle);
					branch.right->replace(0, middle, last);
				}
			} else {
				branch.right->replace(pos-branch.weight, first, last);
			}
			break;
		case ORIGINAL_LEAF:
			if (pos > 0) {
				split(pos);
				branch.right->replace(0, first, last);
			} else if (distance(first, last) < ptrdiff_t(original.size)) {
				split(distance(first, last));
				branch.left->replace(pos, first, last);
			} else {
				type = MODIFIED_LEAF;
				new(&modified.content) deque<char>(first, last);
			}
			break;
		case MODIFIED_LEAF:
			if(distance(first, last) > ptrdiff_t(modified.content.size()-pos)){
				auto middle = next(first, modified.content.size()-pos);
				copy(first, middle, modified.content.begin()+pos);
				copy(middle, last, back_inserter(modified.content));
			} else {
				copy(first, last, modified.content.begin()+pos);
			}
			break;
		}
	}

	template<typename FORWARD_ITERATOR>
	void insert(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last){
		using namespace std;

		switch (type) {
		case BRANCH:
			if (pos <= branch.weight) {
				branch.left->insert(pos, first, last);
				branch.weight += distance(first, last);
			} else {
				branch.right->insert(pos - branch.weight, first, last);
			}
			break;
		case ORIGINAL_LEAF:
			if (pos == original.size) {
				replace(pos, first, last);
			} else {
				split(pos);
				branch.left->insert(pos, first, last);
				branch.weight += distance(first, last);
			}
			break;
		case MODIFIED_LEAF:
			if( pos == modified.content.size()){
				replace(pos, first, last);
			} else {
				split(pos);
				branch.left->insert(pos, first, last);
			}
			break;
		}
	}

	void erase(size_t position, size_t count) {
		switch (type) {
		case BRANCH:
			throw std::logic_error("Unimplemented");
		case ORIGINAL_LEAF:
			throw std::logic_error("Unimplemented");
		case MODIFIED_LEAF:
			throw std::logic_error("Unimplemented");
		}
	}
};

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
	void viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out) const {
		parent->viewRange(pos, count, out, internalTarget);
	}

	/**
	 * @brief Returns a view.
	 * @param count the max number of characters.
	 */
	template<typename OUTPUT_ITERATOR>
	void viewAll(OUTPUT_ITERATOR&& out) const {
		viewRange(0, size_, out);
	}

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
	void replace(FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
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
	void insert(FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
		parent->insert(position, first, last);
		auto incr = std::distance(first, last);
		position += incr;
		size_ += incr;
	}

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
	size_t position, size_;
	std::unique_ptr<MemoryNode> parent;
};

inline MemoryTarget::MemoryTarget(std::string const& filename) :
		internalTarget(filename), position(0) {
	internalTarget.toEnd();
	size_ = internalTarget.tell();
	internalTarget.toStart();
	parent = std::make_unique<MemoryNode>(position, size_);
}

template<typename OUTPUT_ITERATOR>
inline void MemoryTarget::view(size_t count, OUTPUT_ITERATOR out) const {
	static_assert(
			std::is_base_of<std::output_iterator_tag, typename std::iterator_traits<OUTPUT_ITERATOR>::iterator_category>::value,
			"parameter out needs to be an output iterator."
	);
	viewRange(position, count, out);
}

inline size_t MemoryTarget::size() const {
	return size_;
}

inline void MemoryTarget::erase(size_t count) {
	parent->erase(position, count);
}

inline void MemoryTarget::flush() {
//	internalTarget.toStart();
//	internalTarget.replace(content.begin(), content.end());
//	internalTarget.shrink();
//	internalTarget.flush();
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
//	position = std::min(position, content.size());
}

#endif /* SWEET_MEMORYTARGET_HPP_ */

