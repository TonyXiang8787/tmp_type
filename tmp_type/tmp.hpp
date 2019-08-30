#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>

namespace tmp {

template<size_t number, size_t position>
struct encode_position {
	static_assert(position < 16u, "Max position is 15!");
	static size_t constexpr value = 
		encode_position<number, position - 1u>::value >> 4u;
};
template<size_t number>
struct encode_position<number, 0u> {
	static size_t constexpr value = number << 60u;
};
template<size_t number, size_t position>
size_t constexpr encode_position_v = encode_position<number, position>::value;

template<size_t coding, size_t position>
struct decode_position {
	static_assert(position < 16u, "Max position is 15!");
	static size_t constexpr value =
		decode_position<coding << 4u, position - 1u>::value;
};
template<size_t coding>
struct decode_position<coding, 0u> {
	static size_t constexpr value = coding >> 60;
};
template<size_t coding, size_t position>
size_t constexpr decode_position_v = decode_position<coding, position>::value;



size_t constexpr a = encode_position_v<2, 15>;
size_t constexpr b = encode_position_v<2, 14>;
size_t constexpr c = encode_position_v<2, 13>;

size_t constexpr a1 = decode_position_v<a, 15>;
size_t constexpr b1 = decode_position_v<b, 14>;
size_t constexpr c1 = decode_position_v<c, 13>;

size_t constexpr aa = 0xAB10'0000'0000'A0F5ULL;
size_t constexpr aa1 = decode_position_v<aa, 15>;
size_t constexpr aa2 = decode_position_v<aa, 0>;
size_t constexpr aa3 = decode_position_v<aa, 1>;
size_t constexpr aa4 = decode_position_v<aa, 12>;

}