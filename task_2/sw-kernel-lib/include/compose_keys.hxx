#ifndef COMPOSE_KEYS_HXX_
#define COMPOSE_KEYS_HXX_
#include <stdint.h>

template <typename T>
struct alignas(uint64_t) _base_uint64_t{ //Без выравнивания на 8 байт gcc упорно не хочет размещать структуру в регистрах
  [[gnu::always_inline]] operator uint64_t() noexcept {
    static_assert(sizeof(T)==sizeof(uint64_t));
    return *(reinterpret_cast<uint64_t *>(this));
  };
  [[gnu::always_inline]] static T from_int(uint64_t i) noexcept {
    static_assert(sizeof(T)==sizeof(uint64_t));
    return *(reinterpret_cast<T *>(&i));
  };
};
#define STRUCT(name) struct name:_base_uint64_t<name>
#endif /*COMPOSE_KEYS_HXX_*/
