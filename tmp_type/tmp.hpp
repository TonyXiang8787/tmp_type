#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>
#include <array>

template<size_t len>
using FixStr = std::array<char32_t, len>;

namespace tmp {

template<size_t number, size_t position>
struct encode_position {
	static_assert(position < 16u, "Max position is 15!");
	static size_t constexpr value = 
		encode_position<number, position - 1>::value >> 4u;
};
template<size_t number>
struct encode_position<number, 0> {
	static size_t constexpr value = number << 60u;
};
template<size_t number, size_t position>
size_t constexpr encode_position_v = encode_position<number, position>::value;

template<size_t coding, size_t position>
struct decode_position {
	static_assert(position < 16u, "Max position is 15!");
	static size_t constexpr value =
		decode_position<coding << 4u, position - 1>::value;
};
template<size_t coding>
struct decode_position<coding, 0> {
	static size_t constexpr value = coding >> 60u;
};
template<size_t coding, size_t position>
size_t constexpr decode_position_v = decode_position<coding, position>::value;


template<size_t type_code, size_t l = 0> struct decode_type;
template<size_t l> struct decode_type<0, l> { using type = int8_t; };
template<size_t l> struct decode_type<1, l> { using type = uint8_t; };
template<size_t l> struct decode_type<2, l> { using type = uint16_t; };
template<size_t l> struct decode_type<3, l> { using type = uint32_t; };
template<size_t l> struct decode_type<4, l> { using type = uint64_t; };
template<size_t l> struct decode_type<5, l> { using type = int16_t; };
template<size_t l> struct decode_type<6, l> { using type = int32_t; };
template<size_t l> struct decode_type<7, l> { using type = int64_t; };
template<size_t l> struct decode_type<8, l> { using type = float; };
template<size_t l> struct decode_type<9, l> { using type = double; };
template<size_t l> struct decode_type<10, l> { using type = FixStr<1ull << l>; };
template<size_t type_code, size_t l = 0>
using decode_type_t = typename decode_type<type_code, l>::type;

using Ha = decode_type<10, 5>::type;
size_t constexpr a = sizeof(Ha);
}