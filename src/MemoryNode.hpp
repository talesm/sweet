/**
 * @file MemoryNode.h
 *
 * @date 2016-12-19
 * @author talesm
 */

#ifndef SRC_MEMORYNODE_HPP_
#define SRC_MEMORYNODE_HPP_

/**
 * A rope based memory node.
 */
class MemoryNode {
public:
	/**
	 * Constructs a original content based node.
	 * @param offset
	 * @param size
	 */
	MemoryNode(size_t offset, size_t size);

	/**
	 * Constructs a modified content node.
	 * @param content
	 */
	MemoryNode(std::deque<char>&& content, size_t originalSize);

	//dtor
	~MemoryNode();

	/**
	 * View a range
	 * @param pos
	 * @param count
	 * @param out
	 * @param internalTarget
	 */
	template<typename OUTPUT_ITERATOR>
	void viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out, const FileTarget& internalTarget) const;

	/**
	 * Replace text starting at pos.
	 * @param pos
	 * @param first
	 * @param last
	 */
	template<typename FORWARD_ITERATOR>
	void replace(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last);

	/**
	 * Insert text at pos.
	 * @param pos
	 * @param first
	 * @param last
	 */
	template<typename FORWARD_ITERATOR>
	void insert(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last);

	/**
	 * Erase text at pos.
	 * @param pos
	 * @param count
	 */
	void erase(size_t pos, size_t count);

	void flush(FileTarget& target, ptrdiff_t offset = 0);
private:
	/**
	 * Split at pos.
	 * @param pos
	 */
	void split(size_t pos);

	/**
	 * Gets the offset.
	 * @return
	 */
	ptrdiff_t offset() const;

private:
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
			size_t originalSize;
		} modified;
	};
};

inline MemoryNode::MemoryNode(size_t offset, size_t size) {
	type = ORIGINAL_LEAF;
	original.offset = offset;
	original.size = size;
}

inline MemoryNode::MemoryNode(std::deque<char>&& content, size_t originalSize) {
	type = MODIFIED_LEAF;
	new (&modified.content) std::deque<char>(move(content));
	modified.originalSize = originalSize;
}

inline MemoryNode::~MemoryNode() {
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
inline void MemoryNode::viewRange(size_t pos, size_t count, OUTPUT_ITERATOR out, const FileTarget& internalTarget) const {
	switch (type) {
	case BRANCH:
		if (pos < branch.weight) {
			branch.left->viewRange(pos, std::min(branch.weight, count), out, internalTarget);
		}
		if (pos + count > branch.weight) {
			branch.right->viewRange(0, count - branch.weight, out, internalTarget);
		}
		break;
	case ORIGINAL_LEAF:
		internalTarget.viewRange(pos + original.offset, count, out);
		break;
	case MODIFIED_LEAF:
		std::copy(modified.content.begin(), modified.content.end(), out);
		break;
	}
}

template<typename FORWARD_ITERATOR>
inline void MemoryNode::replace(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
	using namespace std;
	switch (type) {
	case BRANCH:
		if (pos < branch.weight) {
			if (distance(first, last) <= ptrdiff_t(branch.weight)) {
				branch.left->replace(pos, first, last);
			} else {
				auto middle = next(first, branch.weight);
				branch.left->replace(pos, first, middle);
				branch.right->replace(0, middle, last);
			}
		} else {
			branch.right->replace(pos - branch.weight, first, last);
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
			size_t originalSize = original.size;
			type = MODIFIED_LEAF;
			new (&modified.content) deque<char>(first, last);
			modified.originalSize = originalSize;
		}

		break;
	case MODIFIED_LEAF:
		if (distance(first, last) > ptrdiff_t(modified.content.size() - pos)) {
			auto middle = next(first, modified.content.size() - pos);
			copy(first, middle, modified.content.begin() + pos);
			copy(middle, last, back_inserter(modified.content));
		} else {
			copy(first, last, modified.content.begin() + pos);
		}
		break;
	}
}

template<typename FORWARD_ITERATOR>
inline void MemoryNode::insert(size_t pos, FORWARD_ITERATOR first, FORWARD_ITERATOR last) {
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
		if (pos == modified.content.size()) {
			replace(pos, first, last);
		} else {
			split(pos);
			branch.left->insert(pos, first, last);
		}
		break;
	}
}

inline void MemoryNode::erase(size_t pos, size_t count) {
	//TODO Remove node if it is empty
	switch (type) {
	case BRANCH:
		if (pos < branch.weight) {
			if (pos + count <= branch.weight) {
				branch.left->erase(pos, count);
				branch.weight -= count;
			} else {
				auto leftCount = branch.weight - pos;
				branch.left->erase(pos, leftCount);
				branch.right->erase(0, count - leftCount);
				branch.weight -= leftCount;
			}
		} else {
			branch.right->erase(pos - branch.weight, count);
		}
		break;
	case ORIGINAL_LEAF:
		if (pos == 0) {
			auto diff = std::min(count, original.size);
			original.offset += diff;
			original.size -= diff;
		} else if (pos + count >= original.size) {
			original.size -= std::min(count, original.size);
		} else {
			split(pos);
			branch.right->erase(0, count);
		}

		break;
	case MODIFIED_LEAF:
		if (pos == 0) {
			modified.content.erase(modified.content.begin(), modified.content.begin() + count);
		} else if (pos + count >= modified.content.size()) {
			modified.content.erase(modified.content.begin() + pos, modified.content.end());
		} else {
			split(pos);
			branch.right->erase(0, count);
		}
	}
}

inline void MemoryNode::flush(FileTarget& target, ptrdiff_t offset) {
	using namespace std;
	switch (type) {
	case BRANCH: {
		ptrdiff_t leftOffset = branch.left->offset();
		if (leftOffset > 0) {
			target.go(branch.weight);
			branch.right->flush(target, offset + leftOffset);
			target.go(-branch.weight - branch.right->original.size - offset);
			branch.left->flush(target, offset);
		} else {
			branch.left->flush(target, offset);
			branch.right->flush(target, offset + leftOffset);
		}
		size_t foffset = branch.left->original.offset;
		size_t size = branch.left->original.size + branch.right->original.size;
		branch.left.~unique_ptr();
		branch.right.~unique_ptr();
		type = ORIGINAL_LEAF;
		original.offset = foffset;
		original.size = size;
		break;
	}
	case ORIGINAL_LEAF: {
		ptrdiff_t cPos = target.tell();
		ptrdiff_t realOffset = target.tell() - original.offset;
		if (realOffset != 0) {
			string buffer;
			buffer.reserve(original.size); //TODO make it in chunks...
			target.viewRange(original.offset, original.size, back_inserter(buffer));
			target.go(cPos - target.tell());
			target.replace(buffer.begin(), buffer.end());
			original.offset += realOffset - offset;
		} else {
			target.go(original.size);
		}
		break;
	}
	case MODIFIED_LEAF: {
		size_t foffset = target.tell();
		size_t size = modified.content.size();
		target.replace(modified.content.begin(), modified.content.end());
		modified.content.~deque();
		type = ORIGINAL_LEAF;
		original.offset = foffset;
		original.size = size;
		break;
	}
	}
}

inline void MemoryNode::split(size_t pos) {
	using namespace std;
	switch (type) {
	case BRANCH:
		throw std::logic_error("Unimplemented");
	case ORIGINAL_LEAF: {
		auto first = original.offset;
		auto middle = first + pos;
		auto last = first + original.size;
		type = BRANCH;
		new (&branch.left) unique_ptr<MemoryNode>(new MemoryNode(first, middle - first));
		new (&branch.right) unique_ptr<MemoryNode>(new MemoryNode(middle, last - middle));
		branch.weight = middle - first;
		break;
	}
	case MODIFIED_LEAF: {
		auto leftContent = move(modified.content);
		deque<char> rightContent(leftContent.begin() + pos, leftContent.end());
		leftContent.erase(leftContent.begin() + pos, leftContent.end());
		size_t leftOriginalSize, rightOriginalSize;
		if (pos > modified.originalSize) {
			leftOriginalSize = modified.originalSize;
			rightOriginalSize = 0;
		} else {
			leftOriginalSize = pos;
			rightOriginalSize = modified.originalSize - pos;
		}
		modified.content.~deque();
		type = BRANCH;
		new (&branch.left) unique_ptr<MemoryNode>(new MemoryNode(move(leftContent), leftOriginalSize));
		new (&branch.right) unique_ptr<MemoryNode>(new MemoryNode(move(rightContent), rightOriginalSize));
		branch.weight = branch.left->modified.content.size();
		break;
	}
	}
}

inline ptrdiff_t MemoryNode::offset() const {
	switch (type) {
	case BRANCH:
		return branch.left->offset() + branch.right->offset();
	case ORIGINAL_LEAF:
		return 0;
	case MODIFIED_LEAF:
		return modified.content.size() - modified.originalSize;
	}
	throw std::logic_error("It should never happen");
}

#endif /* SRC_MEMORYNODE_HPP_ */
