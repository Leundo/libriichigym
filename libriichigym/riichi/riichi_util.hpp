//
//  riichi_util.hpp
//  libriichigym
//
//  Created by Leundo on 2024/04/23.
//

#ifndef riichi_util_hpp
#define riichi_util_hpp

#include <string_view>
#include <utility>
#include <bitset>
#include <concepts>
#include <cstdint>


namespace riichi {

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define likely(x) __builtin_expect(!!(x), 1)

#define unlikely(x) __builtin_expect(!!(x), 0)


template<class T>
using Offset = T;

template<typename T, typename... Types>
concept AllMatching = (... && std::is_same<T, Types>::value);

template<typename T, typename... Types>
concept AnyMatching = (... || std::is_same<T, Types>::value);

template<typename T>
concept UnsignedInteger = std::unsigned_integral<T>;



// MARK: - Bit Tool

#define uoptset_insert(set, value)\
do { *(set) |= (1u << (value)); } while(0)

#define uoptset_remove(set, value)\
do { *(set) &= ~(1u << (value)); } while(0)

#define uoptset_contain(set, value)\
((set & (1u << (value))) != 0)

#define uoptset_mask(size, trailing)\
(~(~0u << (size)) << (trailing))

#define uoptset_extract(set, size, trailing)\
(((~(~0u << (size)) << (trailing)) & set) >> (trailing))

#define underlie(enum_value)\
(std::to_underlying(enum_value))

constexpr uint64_t u64_make_containing() {
    return 0;
}

template <UnsignedInteger T, UnsignedInteger ... Args>
constexpr uint64_t u64_make_containing(T first, Args ... args) {
    return (1u << first) | u64_make_containing(args...);
}


template<unsigned int N>
constexpr std::bitset<N> bitset_make_containing() {
    return 0;
}

template<unsigned int N, UnsignedInteger T, UnsignedInteger ... Args>
constexpr std::bitset<N> bitset_make_containing(T first, Args ... args) {
    std::bitset<N> value = 1u << first;
    return value | bitset_make_containing<N>(args...);
}



// MARK: - ConstexprString
template<typename T, std::size_t N>
struct ConstexprStringContainer {
    using ContainerChar = T;
    constexpr static std::size_t length = N;
    ContainerChar data[length + 1] = {};
    
    template<std::size_t M = N, std::enable_if_t<M == 0, std::size_t> = 0>
    constexpr ConstexprStringContainer(): data{0} {}
    
    template<std::size_t M = N, std::enable_if_t<M == 1, std::size_t> = 0>
    constexpr ConstexprStringContainer(ContainerChar c): data{c, 0} {}
    
    constexpr ConstexprStringContainer(std::basic_string_view<ContainerChar> view) {
        for(std::size_t i = 0; i < length; i++) {
            data[i] = view[i];
        }
        data[N] = 0;
    }
    
    constexpr ConstexprStringContainer(const ContainerChar (&str)[N + 1]) {
        for (std::size_t i = 0; i < length; i += 1) {
            data[i] = str[i];
        }
        data[N] = 0;
    }
    
    template<typename...C>
    constexpr ConstexprStringContainer(std::in_place_t, C...c): data{static_cast<ContainerChar>(c)..., 0} {}
};

template<typename T, std::size_t N>
ConstexprStringContainer(const T(&)[N])->ConstexprStringContainer<T, N - 1>;

template<ConstexprStringContainer Str>
struct ConstexprString {
public:
    using ContainerChar = typename decltype(Str)::ContainerChar;
    static constexpr const ContainerChar* data() { return Str.data; }
    static constexpr std::size_t length() { return Str.length; }
    static constexpr std::basic_string_view<ContainerChar> view() { return Str.data; }
    constexpr operator std::basic_string_view<ContainerChar>() { return view(); }
};

#define CONSTEXPR_STR(str) \
([]{constexpr std::basic_string_view s{str};\
    return ::riichi::ConstexprString<::riichi::ConstexprStringContainer<typename decltype(s)::value_type, s.size()> {str}> {};}\
())


int strncmpci(const char * lhs, const char * rhs, size_t num);


template<class... Ts> struct Visitor: Ts... { using Ts::operator()...; };
template<class... Ts> Visitor(Ts...) -> Visitor<Ts...>;

}

#endif /* riichi_util_hpp */
