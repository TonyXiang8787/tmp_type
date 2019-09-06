#pragma once

#include <type_traits>
#include <cstddef>
#include <utility>
#include <array>

template<size_t len>
using FixStr = std::array<char32_t, len>;
using FixString = FixStr<1>;

// convert length to rank of 2, pick larger exponetial of 2
inline constexpr size_t get_pow2_len_internal(size_t len) {
	if (len == 0) return 0u;
	return get_pow2_len_internal(len / 2) + 1u;
}
inline constexpr size_t get_pow2_len(size_t len) {
	return get_pow2_len_internal(len - 1u);
}

namespace data_type_encoding {

// max and min string length by power 2
constexpr size_t min_str_len_pow2 = 3;
constexpr size_t max_str_len_pow2 = 10;
// encode string using pow2
template<size_t l>
struct fix_str_pow2 {
	using type = FixStr<1ull << l>;
};
template<size_t l>
using fix_str_pow2_t = typename fix_str_pow2<l>::type;

// encode one digit to position
template<size_t number, size_t position>
struct encode_position {
	static_assert(position < 16u, "Max position is 15!");
	static constexpr uint64_t value = 
		encode_position<number, position - 1>::value >> 4u;
};
template<size_t number>
struct encode_position<number, 0> {
	static constexpr uint64_t value = number << 60u;
};
template<size_t number, size_t position>
constexpr uint64_t encode_position_v = encode_position<number, position>::value;

// decode one digit from position
template<uint64_t coding, size_t position>
struct decode_position {
	static_assert(position < 16u, "Max position is 15!");
	static constexpr size_t value =
		decode_position<coding << 4u, position - 1>::value;
};
template<uint64_t coding>
struct decode_position<coding, 0> {
	static constexpr size_t value = coding >> 60u;
};
template<uint64_t coding, size_t position>
constexpr size_t decode_position_v = decode_position<coding, position>::value;

// get scalar data type
template<size_t type_code, size_t l = 0> struct num_scalar_type;
template<size_t l> struct num_scalar_type<0, l> { using type = int8_t; };
template<size_t l> struct num_scalar_type<1, l> { using type = uint8_t; };
template<size_t l> struct num_scalar_type<2, l> { using type = uint16_t; };
template<size_t l> struct num_scalar_type<3, l> { using type = uint32_t; };
template<size_t l> struct num_scalar_type<4, l> { using type = uint64_t; };
template<size_t l> struct num_scalar_type<5, l> { using type = int16_t; };
template<size_t l> struct num_scalar_type<6, l> { using type = int32_t; };
template<size_t l> struct num_scalar_type<7, l> { using type = int64_t; };
template<size_t l> struct num_scalar_type<8, l> { using type = float; };
template<size_t l> struct num_scalar_type<9, l> { using type = double; };
template<size_t l> struct num_scalar_type<10, l> { 
	static_assert(l >= min_str_len_pow2);
	static_assert(l <= max_str_len_pow2);
	using type = fix_str_pow2_t<l>;
};
// retrieve type in one go
template<uint64_t coding>
using decode_scalar_type_t = typename num_scalar_type<
	decode_position_v<coding, 0>,
	decode_position_v<coding, 2>
>::type;

// scalar data type to encoding
template<class T> struct encode_scalar_type;
template<> struct encode_scalar_type<int8_t> {
	static constexpr uint64_t value = encode_position_v<0, 0>; };
template<> struct encode_scalar_type<uint8_t> {
	static constexpr uint64_t value = encode_position_v<1, 0>; };
template<> struct encode_scalar_type<uint16_t> {
	static constexpr uint64_t value = encode_position_v<2, 0>; };
template<> struct encode_scalar_type<uint32_t> {
	static constexpr uint64_t value = encode_position_v<3, 0>; };
template<> struct encode_scalar_type<uint64_t> {
	static constexpr uint64_t value = encode_position_v<4, 0>; };
template<> struct encode_scalar_type<int16_t> {
	static constexpr uint64_t value = encode_position_v<5, 0>; };
template<> struct encode_scalar_type<int32_t> {
	static constexpr uint64_t value = encode_position_v<6, 0>; };
template<> struct encode_scalar_type<int64_t> {
	static constexpr uint64_t value = encode_position_v<7, 0>; };
template<> struct encode_scalar_type<float> {
	static constexpr uint64_t value = encode_position_v<8, 0>; };
template<> struct encode_scalar_type<double> {
	static constexpr uint64_t value = encode_position_v<9, 0>; };
template<size_t len> struct encode_scalar_type<FixStr<len>> {
	static constexpr uint64_t value = encode_position_v<10, 0> +
		encode_position_v<get_pow2_len(len), 2>;
};
template<class T>
constexpr uint64_t encode_scalar_type_v = encode_scalar_type<T>::value;

// data type struct
template<class T, size_t... D>
struct data_type_struct {};

// decode data type
template<uint64_t coding>
struct decode_data_type {
	using scalar_type = decode_scalar_type_t<coding>;  // digit 0 and 2
	static constexpr size_t ndim = decode_position_v<coding, 1>;
	static_assert(ndim <= 13);
	// make type struct using index sequence
	template<class T> struct make_type_struct;
	template<size_t... I>  // sequence of 0, 1, 2, ..., ndim - 1
	struct make_type_struct<std::index_sequence<I...>> {
		using type = data_type_struct<scalar_type,
			decode_position_v<coding, I + 3>...>;   // dimension size start from 3
	};
	// assign type to index sequence
	using type = typename make_type_struct<
		std::make_index_sequence<ndim>>::type;
};
template<uint64_t coding>
using decode_data_type_t = typename decode_data_type<coding>::type;

// encode data type
template<class T> struct encode_data_type;
template<class T, size_t... D>
struct encode_data_type<data_type_struct<T, D...>> {
	static constexpr uint64_t coding_scalar_type = encode_scalar_type_v<T>;
	static constexpr size_t ndim = sizeof...(D);
	static constexpr uint64_t coding_ndim = encode_position_v<ndim, 1>;
	// make coding struct using index sequence
	template<class T> struct make_coding_struct;
	template<size_t... I>  // sequence of 0, 1, 2, ..., ndim - 1
	struct make_coding_struct<std::index_sequence<I...>> {
		static constexpr uint64_t value =
			(encode_position_v<D, I + 3> +... +  // dimension size start from 3
			(coding_scalar_type + coding_ndim));
	};
	// assign value
	static constexpr uint64_t value = 
		make_coding_struct<std::make_index_sequence<ndim>>::value;
};
template<class T>
constexpr uint64_t encode_data_type_v = encode_data_type<T>::value;

// convert raw list of data type
// containing FixString (0 2pow)
// FixString to all possible combinations of FixStr<min> to FixStr<max>


constexpr uint64_t enha = encode_data_type_v<data_type_struct<double, 0, 2, 5>>;
using Ha = decode_data_type_t<enha>;


static_assert(std::is_same_v<Ha, data_type_struct<double, 0, 2, 5>>);
static_assert(enha == encode_data_type_v<Ha>);
}