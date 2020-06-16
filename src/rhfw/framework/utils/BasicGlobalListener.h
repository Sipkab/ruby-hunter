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
 * BasicListener.h
 *
 *  Created on: 2016. febr. 24.
 *      Author: sipka
 */

#ifndef BASICGLOBALLISTENER_H_
#define BASICGLOBALLISTENER_H_

#include <framework/utils/LinkedList.h>
#include <framework/utils/BasicListener.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

//TODO make thread safe version
template<typename T>
class BasicGlobalListener: public LinkedNode<T> {
private:
	template<typename BASE, typename DERIVED>
	struct IsDerived {
		//MSVC throws INTERNAL COMPILER ERROR if constexpr test is used
		typedef long long yes;
		typedef char no;

		static yes test(BASE*);
		static no test(...);

		static DERIVED* get(void);

		static const bool value = sizeof(test(get())) == sizeof(yes);
	};

	template<typename U, bool isderived>
	struct CastAway;
	template<typename U>
	struct CastAway<U, false> {
		constexpr static U* castaway(BasicGlobalListener<U>* ptr) {
			return nullptr;
		}
	};
	template<typename U>
	struct CastAway<U, true> {
		constexpr static U* castaway(BasicGlobalListener<U>* ptr) {
			return static_cast<U*>(ptr);
		}
	};

	static typename BasicListener<T>::Events listeners;

protected:
	BasicGlobalListener() = default;
public:
	static bool hasListeners() {
		return !listeners.isEmpty();
	}
	virtual ~BasicGlobalListener() = default;

	static void addListenerToEnd(LinkedNode<T>& listener) {
		listeners += listener;
	}

	static auto foreach() -> decltype(listeners.foreach()) {
		return listeners.foreach();
	}
	static auto pointers() -> decltype(listeners.pointers()) {
		return listeners.pointers();
	}

	static void subscribeListener(LinkedNode<T>& listener) {
		addListenerToEnd(listener);
	}
	static void unsubscribeListener(LinkedNode<T>& listener) {
		listeners -= listener;
	}
	void subscribe() {
		subscribeListener(*this);
	}
	void unsubscribe() {
		unsubscribeListener(*this);
	}

	T* get() override {
		return CastAway<T, IsDerived<BasicGlobalListener<T>, T>::value>::castaway(this);
	}
};

template<typename T>
typename BasicListener<T>::Events BasicGlobalListener<T>::listeners;

}

#endif /* BASICGLOBALLISTENER_H_ */
