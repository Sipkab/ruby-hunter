/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * utility.h
 *
 *  Created on: 2015 nov. 7
 *      Author: sipka
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <gen/configuration.h>

namespace rhfw {
namespace util {

template<class T>
class remove_reference {
public:
	typedef T type;
};
template<class T>
class remove_reference<T&> {
public:
	typedef T type;
};
template<class T>
class remove_reference<T&&> {
public:
	typedef T type;
};
template<class T> struct remove_const {
	typedef T type;
};
template<class T> struct remove_const<const T> {
	typedef T type;
};

template<class T>
struct is_rvalue_reference {
	static const bool value = false;
};
template<class T>
struct is_rvalue_reference<T&&> {
	static const bool value = true;
};

template<class T>
struct is_lvalue_reference {
	static const bool value = false;
};
template<class T>
struct is_lvalue_reference<T&> {
	static const bool value = true;
};

template<typename T>
inline T&& move(T& m) {
	return static_cast<T&&>(m);
}

template<typename T>
inline void swap(T & a, T & b) {
	T temp { util::move(a) };
	a = util::move(b);
	b = util::move(temp);
}
template<class T, unsigned int N>
inline void swap(T (&a)[N], T (&b)[N]) {
	for (unsigned int i = 0; i < N; ++i) {
		swap(a[i], b[i]);
	}
}

template<class T>
T&& forward(typename remove_reference<T>::type& t) {
	return static_cast<T&&>(t);
}

template<class T>
T&& forward(typename remove_reference<T>::type&& t) {
	return static_cast<T&&>(t);
}

template<bool B, class T = void>
struct enable_if {
};

template<class T>
struct enable_if<true, T> {
	typedef T type;
};

template<typename T, T ... Vals>
struct is_any;

template<typename T>
struct is_any<T> {
	static bool check(T val) {
		return false;
	}
};
template<typename T, T Head, T ... Tail>
struct is_any<T, Head, Tail...> {
	static bool check(T val) {
		return val == Head || is_any<T, Tail...>::check(val);
	}
};

template<class T, class U>
struct is_same {
	static const bool value = false;
};

template<class T>
struct is_same<T, T> {
	static const bool value = true;
};

template<typename T, typename ... Args>
struct is_constructible {
private:
	typedef long long VALUE_YES;
	typedef char VALUE_NO;
	template<typename U, typename ... UArgs,
			decltype(U(static_cast<UArgs>(*static_cast<typename util::remove_reference<UArgs>::type*>(nullptr))...))* = nullptr>
	static VALUE_YES test(int*);
	template<typename U, typename ... UArgs>
	static VALUE_NO test(void*);
public:
	static const bool value = sizeof(test<T, Args...>(static_cast<int*>(nullptr))) == sizeof(VALUE_YES);
};

} // namespace util
} // namespace rhfw

#endif /* UTILITY_H_ */
