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
 *  Created on: 2016. febr. 26.
 *      Author: sipka
 */

#ifndef BASICLISTENER_H_
#define BASICLISTENER_H_

#include <framework/utils/LinkedList.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/log.h>
#include <gen/fwd/types.h>

namespace rhfw {

template<typename T>
class BasicListener: public LinkedNode<T> {
protected:
	template<typename Functor>
	class BasicLambda: public T {
	protected:
		Functor functor;
	public:
		BasicLambda(Functor&& f)
				: functor(util::forward<Functor>(f)) {
		}
	};
public:
	//TODO make thread-safe Events?
	class Listener {
	private:
		LinkedNode<T>* link;
	public:
		Listener()
				: link(nullptr) {
		}
		Listener(LinkedNode<T>* link)
				: link(link) {
		}
		Listener(Listener&& o)
				: link(o.link) {
			o.link = nullptr;
		}
		Listener(const Listener&) = delete;
		Listener& operator=(Listener&& o) {
			ASSERT(this != &o);
			delete this->link;

			this->link = o.link;

			o.link = nullptr;
			return *this;
		}
		Listener& operator=(const Listener&) = delete;
		Listener& operator=(NULLPTR_TYPE) {
			delete link;
			link = nullptr;
			return *this;
		}

		bool operator==(NULLPTR_TYPE) const {
			return link == nullptr;
		}
		bool operator!=(NULLPTR_TYPE) const {
			return link != nullptr;
		}
		~Listener() {
			delete link;
		}

		operator LinkedNode<T>&() {
			ASSERT(link != nullptr);
			return *link;
		}
		operator T&() {
			ASSERT(link != nullptr);
			return *link->get();
		}

		bool isAttached() const {
			ASSERT(link != nullptr);
			return link->isInList();
		}
		void unsubscribe() {
			ASSERT(link != nullptr);
			link->removeLinkFromList();
		}
	};

	class Events {
	private:
		LinkedList<T, false> listeners;
	public:
		bool isEmpty() const {
			return listeners.isEmpty();
		}

		void operator+=(LinkedNode<T>& listener) {
			listeners.addToEnd(listener);
		}
		void operator-=(LinkedNode<T>& listener) {
			//ASSERT(listeners.contains(listener.get())) << "Eventlistener collection does not contain passed listener";
			listener.removeLinkFromList();
		}

		decltype(listeners.objects()) foreach() {
			return listeners.objects();
		}
		decltype(listeners.pointers()) pointers() {
			return listeners.pointers();
		}
		decltype(listeners.nodes()) nodes() {
			return listeners.nodes();
		}
	};
	template<typename U = T, typename Functor>
	static Listener make_listener(Functor&& f) {
		auto* res = new typename U::template Lambda<Functor>(util::forward<Functor>(f));
		return Listener(res);
	}
	template<typename U = T, typename Functor>
	static U* new_listener(Functor&& f) {
		return new typename U::template Lambda<Functor>(util::forward<Functor>(f));
	}
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
		constexpr static U* castaway(BasicListener<U>* ptr) {
			return nullptr;
		}
	};
	template<typename U>
	struct CastAway<U, true> {
		constexpr static U* castaway(BasicListener<U>* ptr) {
			return static_cast<U*>(ptr);
		}
	};

protected:
	BasicListener() = default;
public:
	virtual ~BasicListener() = default;

	T* get() override {
		return CastAway<T, IsDerived<BasicListener<T>, T>::value>::castaway(this);
	}

	void unsubscribe() {
		this->removeLinkFromList();
	}
};

template<typename ... Args>
class SimpleListener;

template<typename Ret, typename ... Args>
class SimpleListener<Ret(Args...)> : public BasicListener<SimpleListener<Ret(Args...)>> {
public:
	template<typename Functor>
	class Lambda: public SimpleListener<Ret(Args...)> {
		Functor functor;
	public:
		Lambda(Functor&& f)
				: functor(util::forward<Functor>(f)) {
		}
		virtual Ret operator()(Args ... args) override {
			return functor(util::forward<Args>(args)...);
		}
	};
	virtual Ret operator()(Args ... args) = 0;
};

} // namespace rhfw
#endif /* BASICLISTENER_H_ */
