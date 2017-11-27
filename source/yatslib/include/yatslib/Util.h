
#pragma once


#include <yatslib/yatslib_api.h>
#include <memory>


template<typename T>
struct has_options
{
	//TODO: use the real option
	using Option = int;

	template<typename U, Option(*)()> struct SFINAE {};
	template<typename U> static char test_function(SFINAE<U, &U::options>*);
	template<typename U> static int test_function(...);
	static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template<typename T>
constexpr bool has_options_v = has_options<T>::value;

template<typename T>
struct has_run
{
	template<typename U> static char test_function(decltype(&U::run));
	template<typename U> static int test_function(...);
	static constexpr bool value = sizeof(test_function<T>(0)) == sizeof(char);
};

template<typename T>
constexpr bool has_run_v = has_run<T>::value;

template<typename T>
struct is_unique_ptr
{
	template<typename U> static char test_function(const std::unique_ptr<U> &);
	static int test_function(...);
	static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template<typename T>
constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

template<typename T>
struct is_shared_ptr
{
	template<typename U> static char test_function(const std::shared_ptr<U> &);
	static int test_function(...);
	static constexpr bool value = sizeof(test_function(std::declval<T>())) == sizeof(char);
};

template<typename T>
constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;