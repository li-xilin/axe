#ifndef AXE_CHECK_H_
#define AXE_CHECK_H_
#include <axe/debug.h>

#define CHECK_PARAM_NULL(_param) ax_assert((_param), "parameter `%s' is NULL", #_param)

#define CHECK_PARAM_VALIDITY(_param, _cond) ax_assert((_cond), "parameter `%s' is invalid", #_param)

#define CHECK_ITERATOR_VALIDITY(_param, _cond) ax_assert((_cond), "iterator `%s' is invalid", #_param)

#define CHECK_TRAIT_VALIDITY(_trait, _fun) ax_assert((_cond), "trait `%s' is not specified", #_trait)

#define CHECK_ITER_COMPARABLE(_it1, _it2) \
{ \
	ax_assert((_it1)->owner == (_it2)->owner, "different owner for two iterators"); \
	ax_assert((_it1)->tr == (_it2)->tr, "different direction for two iterators"); \
}

#define CHECK_ITER_TYPE(_it, _type) ax_assert(ax_one_is(it->owner, _type), "it->owner is not " _type);

#define UNSUPPORTED() ax_assert(ax_false, "trait unsupported");

#endif
