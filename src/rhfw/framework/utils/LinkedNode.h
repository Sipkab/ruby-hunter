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
 * LinkedNode.h
 *
 *  Created on: 2014.08.22.
 *      Author: sipka
 */

#ifndef LINKEDNODE_H_
#define LINKEDNODE_H_

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {
//not declaring them here, might result in friend compilation errors
template<typename, bool>
class LinkedList;
template<typename, bool, typename >
class LifeCycleChain;

template<typename T>
class LinkedNode {
private:
	template<typename, bool>
	friend class LinkedList;

	template<typename, bool, typename >
	friend class LifeCycleChain;

	LinkedNode<T>* prev;
	LinkedNode<T>* next;
public:
	typedef T NODE_GET_TYPE;

	LinkedNode()
			: prev { nullptr }, next { nullptr } {
	}
	LinkedNode(LinkedNode<T>* prev, LinkedNode<T>* next)
			: prev { prev }, next { next } {
	}
	LinkedNode(LinkedNode<T> && n)
			: prev { n.prev }, next { n.next } {
		if (this->prev != nullptr) {
			this->prev->next = this;
			this->next->prev = this;
			n.prev = nullptr;
		}
	}
	LinkedNode<T>& operator=(LinkedNode<T> && n) {
		ASSERT(&n != this) << "Self move assignment";

		if (this->prev != nullptr) {
			this->prev->next = this->next;
			this->next->prev = this->prev;
		}

		this->prev = n.prev;
		this->next = n.next;
		if (this->prev != nullptr) {
			this->prev->next = this;
			this->next->prev = this;
			n.prev = nullptr;
		}
		return *this;
	}
	LinkedNode(const LinkedNode<T>&) = delete;
	LinkedNode<T>& operator=(const LinkedNode<T>&) = delete;

	bool isInList() const {
		return this->prev != nullptr;
	}
	virtual ~LinkedNode() {
		if (this->prev != nullptr) {
			this->prev->next = this->next;
			this->next->prev = this->prev;
		}
	}

	void removeLinkFromList() {
		if (this->prev != nullptr) {
			this->prev->next = this->next;
			this->next->prev = this->prev;
			this->prev = nullptr;
		}
	}
	virtual T* get() {
		return nullptr;
	}
};
} // namespace rhfw

#endif /* LINKEDNODE_H_ */
