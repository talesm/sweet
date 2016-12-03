/**
 * @file writeable_target_tag.h
 *
 * @date 2016-12-05
 * @author talesm
 */

#ifndef SRC_TARGETTRAITS_HPP_
#define SRC_TARGETTRAITS_HPP_

struct writeable_target_tag {
};
struct insertable_target_tag: public writeable_target_tag {
};

template<typename TARGET>
struct TargetTrait;
// Causes an error if called upon unsupported

// Forward
class FileTarget;

template<>
struct TargetTrait<FileTarget> {
	using category = writeable_target_tag;
};

// Forward
class MemoryTarget;

template<>
struct TargetTrait<MemoryTarget> {
	using category = insertable_target_tag;
};

#endif /* SRC_TARGETTRAITS_HPP_ */
