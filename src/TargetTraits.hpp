/**
 * @file writeable_target_tag.h
 *
 * @date 2016-12-05
 * @author talesm
 */

#ifndef SRC_TARGETTRAITS_HPP_
#define SRC_TARGETTRAITS_HPP_

struct appendable_target_tag {
};
struct insertable_target_tag: public appendable_target_tag {
};

template<typename TARGET>
struct TargetTrait{
	using category = typename TARGET::category;
};

#endif /* SRC_TARGETTRAITS_HPP_ */
