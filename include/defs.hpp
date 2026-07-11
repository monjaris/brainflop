#pragma once
#include <cstdint>
#include <cstdlib>

#define NAMESPACE_START(_name) namespace _name {
#define NAMESPACE_END(_name) }


using int32 = int32_t;
using int64 = int64_t;
using uint32 = uint32_t;

using isize = ssize_t;
using usize = size_t;

template<class T> using inilist = std::initializer_list<T>;
using strview = std::basic_string_view<char>;
