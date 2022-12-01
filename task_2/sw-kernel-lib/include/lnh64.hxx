#ifndef LNH64_HXX_
#define LNH64_HXX_

#include "lnh64.h"

template <typename T>
__attribute__((always_inline)) static inline T get_result_key(){
	return T::from_int(lnh_core.result.key);
}
template <typename T>
__attribute__((always_inline)) static inline T get_result_value(){
	return T::from_int(lnh_core.result.value);
}

#endif /*LNH64_HXX_*/
