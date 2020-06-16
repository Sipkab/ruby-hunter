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
 * LinkedList.h
 *
 *  Created on: 2015 febr. 21
 *      Author: sipka
 */

#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

#include <framework/utils/LinkedNode.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

//TODO use policies instead of bool flag in template
template<typename T, bool OwnsItems = true>
class LinkedList {
	template<typename, bool>
	friend class LinkedList;
private:
	LinkedNode<T> node;
public:
	LinkedList()
			: node(&node, &node) {
	}
	~LinkedList() {
		while (node.next != &node) {
			ASSERT(node.next->prev == &node);
			if (OwnsItems) { //compiler will optimize this
				delete node.next;
			} else {
				node.next->removeLinkFromList();
			}
		}
		/*for (LinkedNode<T> *n = node.next; n != &node;) {
		 LinkedNode<T> *next = n->next;
		 n = next;
		 }*/
	}
	LinkedList(const LinkedList<T, OwnsItems>& o) = delete;
	LinkedList<T, OwnsItems>& operator=(const LinkedList<T, OwnsItems>& o) = delete;

	LinkedList(LinkedList<T, OwnsItems> && o) {
		if (!o.isEmpty()) {
			node.next = o.node.next;
			node.prev = o.node.prev;
			node.next->prev = &node;
			node.prev->next = &node;
			o.node.next = o.node.prev = &o.node;
		} else {
			node.prev = &node;
			node.next = &node;
		}
	}
	LinkedList<T, OwnsItems>& operator=(LinkedList<T, OwnsItems> && o) {
		clear();
		if (!o.isEmpty()) {
			node.next = o.node.next;
			node.prev = o.node.prev;
			node.next->prev = &node;
			node.prev->next = &node;
			o.node.next = o.node.prev = &o.node;
		}
		return *this;
	}

	/**
	 * Assuming the item is not currently in any list
	 */
	void addToEnd(LinkedNode<T>& item) {
		WARN(item.isInList()) << "Trying to add node to linked list, while already in list";
		if (item.isInList()) {
			item.removeLinkFromList();
		}
		//add to the end
		node.prev->next = &item;
		item.prev = node.prev;
		item.next = &node;
		node.prev = &item;
	}
	/**
	 * Assuming the item is not currently in any list
	 */
	void addToStart(LinkedNode<T>& item) {
		WARN(item.isInList()) << "Trying to add node to linked list, while already in list";
		if (item.isInList()) {
			item.removeLinkFromList();
		}
		//add to the front
		node.next->prev = &item;
		item.next = node.next;
		item.prev = &node;
		node.next = &item;
	}
	void insertBefore(LinkedNode<T>& relative, LinkedNode<T>& item) {
		item.prev = relative.prev;
		item.next = &relative;
		relative.prev->next = &item;
		relative.prev = &item;
	}
	void insertAfter(LinkedNode<T>& relative, LinkedNode<T>& item) {
		item.next = relative.next;
		item.prev = &relative;
		item.next->prev = &item;
		relative.next = &item;
	}
	LinkedNode<T>* getNodeFor(T* node) {
		for (auto* n : nodes()) {
			if (n->get() == node) {
				return n;
			}
		}
		return nullptr;
	}
	bool contains(T* node) {
		return getNodeFor(node) != nullptr;
	}
	bool hasElements() const {
		return node.next != &node;
	}
	bool isEmpty() const {
		return node.next == &node;
	}

	template<bool ParamOwns>
	void takeAllEnd(LinkedList<T, ParamOwns>& list) {
		if (list.isEmpty()) {
			return;
		}
		node.prev->next = list.node.next;
		list.node.next->prev = node.prev;
		list.node.prev->next = &node;
		node.prev = list.node.prev;

		list.node.prev = list.node.next = &list.node;
	}
	template<bool ParamOwns>
	void takeAllStart(LinkedList<T, ParamOwns>& list) {
		if (list.isEmpty()) {
			return;
		}

		node.next->prev = list.node.prev;
		list.node.prev->next = node.next;
		list.node.next->prev = &node;
		node.next = list.node.next;

		list.node.prev = list.node.next = &list.node;
	}

	LinkedNode<T>* first() {
		return node.next;
	}
	LinkedNode<T>* last() {
		return node.prev;
	}

	void clear() {
		while (node.next != &node) {
			ASSERT(node.next->prev == &node);
			if (OwnsItems) { //compiler will optimize this
				delete node.next;
			} else {
				node.next->removeLinkFromList();
			}
		}
	}
	template<typename FunctorType>
	void clear(FunctorType&& functor) {
		while (node.next != &node) {
			ASSERT(node.next->prev == &node);
			LinkedNode<T>* n = node.next;
			functor(n); //TODO when do we call this
			if (OwnsItems) { //compiler will optimize this
				delete n;
			} else {
				n->removeLinkFromList();
			}
		}
	}

	class ring_iterator {
	private:
		LinkedNode<T>* sentinel;
		LinkedNode<T>* node;
	public:
		ring_iterator(LinkedList<T, OwnsItems>& list)
				: sentinel { &list.node }, node { &list.node } {
		}
		ring_iterator(LinkedList<T, OwnsItems>& list, LinkedNode<T>* node)
				: sentinel { &list.node }, node { node } {
		}
		ring_iterator(const ring_iterator&) = default;
		ring_iterator& operator=(const ring_iterator&) = default;

		ring_iterator& operator++() {
			node = node->next;
			if (node == sentinel) {
				node = sentinel->next;
			}
			return *this;
		}
		ring_iterator& operator--() {
			node = node->prev;
			if (node == sentinel) {
				node = sentinel->prev;
			}
			return *this;
		}
		LinkedNode<T>* operator*() const {
			return node;
		}
		LinkedNode<T>* operator->() const {
			return node;
		}

		bool operator==(const ring_iterator& o) const {
			return node == o.node;
		}
		bool operator!=(const ring_iterator& o) const {
			return node != o.node;
		}

		void invalidate() {
			node = sentinel;
		}
		bool isValid() const {
			return node != sentinel;
		}
		operator bool() const {
			return isValid();
		}
	};

private:
	template<bool reverse>
	class node_iterator {
		LinkedList<T, OwnsItems>& list;
		LinkedList<T, OwnsItems>& holder;

		node_iterator(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder)
				: list(list), holder(holder) {
		}
	public:
		static node_iterator begin(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return node_iterator { list, holder };
		}
		static node_iterator end(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return node_iterator { list, holder };
		}

		node_iterator& operator++() {
			if (!holder.isEmpty()) {
				if (reverse) {
					auto last = holder.last();
					last->removeLinkFromList();
					list.addToStart(*last);
				} else {
					auto first = holder.first();
					first->removeLinkFromList();
					list.addToEnd(*first);
				}
			}
			return *this;
		}
		LinkedNode<T>* operator*() {
			ASSERT(!holder.isEmpty());
			return reverse ? holder.last() : holder.first();
		}
		LinkedNode<T>* operator->() {
			ASSERT(!holder.isEmpty());
			return reverse ? holder.last() : holder.first();
		}
		bool operator==(const node_iterator& i) const {
			return holder.isEmpty() == i.holder.isEmpty();
		}
		bool operator!=(const node_iterator& i) const {
			return holder.isEmpty() != i.holder.isEmpty();
		}
	};

	template<bool reverse>
	class pointer_iterator {
		LinkedList<T, OwnsItems>& list;
		LinkedList<T, OwnsItems>& holder;
		LinkedNode<T>* node = nullptr;
		T* obj = nullptr;

		pointer_iterator(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder)
				: list(list), holder(holder) {
			if (!holder.isEmpty()) {
				node = reverse ? holder.last() : holder.first();
				obj = node->get();
			}
		}
	public:
		static pointer_iterator begin(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return pointer_iterator { list, holder };
		}
		static pointer_iterator end(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return pointer_iterator { list, holder };
		}

		pointer_iterator& operator++() {
			if (!holder.isEmpty()) {
				if (reverse) {
					auto last = holder.last();
					if (node == last) {
						last->removeLinkFromList();
						list.addToStart(*last);
					}
				} else {
					auto first = holder.first();
					if (node == first) {
						first->removeLinkFromList();
						list.addToEnd(*first);
					}
				}
				//in case of empty holder, last obj is nullptr
				node = reverse ? holder.last() : holder.first();
				obj = node->get();
			}
			return *this;
		}
		T* operator*() {
			return obj;
		}
		T* operator->() {
			return obj;
		}
		bool operator==(const pointer_iterator& i) const {
			return holder.isEmpty() == i.holder.isEmpty();
		}
		bool operator!=(const pointer_iterator& i) const {
			return holder.isEmpty() != i.holder.isEmpty();
		}
	};

	template<bool reverse>
	class object_iterator {
		LinkedList<T, OwnsItems>& list;
		LinkedList<T, OwnsItems>& holder;
		LinkedNode<T>* node = nullptr;
		T* obj = nullptr;

		object_iterator(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder)
				: list(list), holder(holder) {
			if (!holder.isEmpty()) {
				node = reverse ? holder.last() : holder.first();
				obj = node->get();
				if (obj == nullptr) {
					goNext();
				}
			}
		}

		void goNext() {
			while (!holder.isEmpty()) {
				if (reverse) {
					auto last = holder.last();
					if (node == last) {
						last->removeLinkFromList();
						list.addToStart(*last);
					}
				} else {
					auto first = holder.first();
					if (node == first) {
						first->removeLinkFromList();
						list.addToEnd(*first);
					}
				}
				//in case of empty holder, last obj is nullptr
				node = reverse ? holder.last() : holder.first();
				obj = node->get();
				if (obj != nullptr) {
					break;
				}
			}
		}
	public:
		static object_iterator begin(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return object_iterator { list, holder };
		}
		static object_iterator end(LinkedList<T, OwnsItems>& list, LinkedList<T, OwnsItems>& holder) {
			return object_iterator { list, holder };
		}

		object_iterator& operator++() {
			goNext();
			return *this;
		}
		T& operator*() {
			ASSERT(!holder.isEmpty());
			ASSERT(obj != nullptr);
			return *obj;
		}
		T* operator->() {
			ASSERT(!holder.isEmpty());
			ASSERT(obj != nullptr);
			return obj;
		}
		bool operator==(const object_iterator& i) const {
			return holder.isEmpty() == i.holder.isEmpty();
		}
		bool operator!=(const object_iterator& i) const {
			return holder.isEmpty() != i.holder.isEmpty();
		}
	};

	template<template<bool> class IteratorType, bool Reverse>
	class iterator_creator {
	private:
		typedef IteratorType<Reverse> exact_iterator;

		LinkedNode<T> splitdummy;
		LinkedList<T, OwnsItems>& list;
		LinkedList<T, OwnsItems> target;
		LinkedList<T, OwnsItems> holder;
		LinkedList<T, OwnsItems> empty;
	public:
		iterator_creator(LinkedList<T, OwnsItems>& list)
				: list(list) {
			holder.takeAllEnd(list);
			if (!holder.isEmpty()) {
				list.addToEnd(splitdummy);
			}
		}
		iterator_creator(iterator_creator<IteratorType, Reverse> &&) = default;
		~iterator_creator() {
			if (splitdummy.isInList()) {
				if (Reverse) {
					target.takeAllStart(holder);
				} else {
					target.takeAllEnd(holder);
				}

				if (!target.isEmpty()) {
					splitdummy.prev->next = target.node.next;
					splitdummy.next->prev = target.node.prev;
					target.node.next->prev = splitdummy.prev;
					target.node.prev->next = splitdummy.next;

					splitdummy.prev = nullptr;
					target.node.next = target.node.prev = &target.node;
				}
			}

		}
		exact_iterator begin() {
			return exact_iterator::begin(target, holder);
		}
		exact_iterator end() {
			return exact_iterator::end(target, empty);
		}
		iterator_creator<IteratorType, !Reverse> reverse() {
			return {holder};
		}
	};

	class reversed_list {
	private:
		LinkedList<T, OwnsItems>& list;
	public:
		reversed_list(LinkedList<T, OwnsItems>& list)
				: list(list) {
		}
		iterator_creator<node_iterator, true> nodes() {
			return {list};
		}
		iterator_creator<pointer_iterator, true> pointers() {
			return {list};
		}
		iterator_creator<object_iterator, true> objects() {
			return {list};
		}
		LinkedList<T, OwnsItems>& reverse() {
			return list;
		}
		/*
		 * Assuming the item is not currently in any list
		 */
		void addToEnd(LinkedNode<T>* item) {
			list.addToStart(item);
		}
		/**
		 * Assuming the item is not currently in any list
		 */
		void addToStart(LinkedNode<T>* item) {
			list.addToEnd(item);
		}
		bool hasElements() const {
			return list.hasElements();
		}
		bool isEmpty() const {
			return list.isEmpty();
		}

		template<typename FunctorType>
		void clear(const FunctorType& functor) {
			for (LinkedNode<T> *n = list.node.prev; n != &list.node;) {
				functor(n); //TODO when do we call this

				LinkedNode<T>* next = n->prev;

				if (OwnsItems) { //compiler will optimize this
					delete n;
				} else {
					n->removeLinkFromList();
				}
				n = next;
			}
			list.node.next = list.node.prev = &list.node;
		}
	};
public:

	reversed_list reverse() {
		return {*this};
	}

	iterator_creator<node_iterator, false> nodes() {
		return {*this};
	}
	iterator_creator<pointer_iterator, false> pointers() {
		return {*this};
	}
	iterator_creator<object_iterator, false> objects() {
		return {*this};
	}

	ring_iterator ring() {
		return ring_iterator { *this };
	}

	typedef node_iterator<true> node_iterator_reverse;
	typedef node_iterator<false> node_iterator_forward;
	typedef pointer_iterator<true> pointer_iterator_reverse;
	typedef pointer_iterator<false> pointer_iterator_forward;
	typedef object_iterator<true> object_iterator_reverse;
	typedef object_iterator<false> object_iterator_forward;
};

} // namespace rhfw

#endif /* LINKEDLIST_H_ */
