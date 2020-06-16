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
 * LifeCycleChain.h
 *
 *  Created on: 2015 febr. 25
 *      Author: sipka
 */

#ifndef LIFECYCLECHAIN_H_
#define LIFECYCLECHAIN_H_

#include <framework/utils/utility.h>
#include <framework/utils/LinkedNode.h>

#include <gen/log.h>
#include <gen/configuration.h>

namespace rhfw {

template<typename NodeType, bool DeleteOnDestruction = true, typename GetType = NodeType>
class LifeCycleChain: public LinkedNode<NodeType> {
private:
	typedef LifeCycleChain<NodeType, DeleteOnDestruction, GetType> self_type;
	LinkedNode<NodeType> innernode;
public:
	LifeCycleChain()
			: innernode(&innernode, &innernode) {
	}
	LifeCycleChain(self_type&& o) = default;
	self_type& operator=(self_type&& o) = default;
	LifeCycleChain(const self_type&) = delete;
	self_type& operator=(const self_type& o) = delete;
	~LifeCycleChain() {
		if (innernode.next != &innernode) {
			if (DeleteOnDestruction) {
				NodeType* obj = innernode.next->get();
				delete obj;
			} else {
				//dont delete, just remove myself from chain
				if (this->prev != nullptr) {
					this->prev->next = innernode.next;
					this->next->prev = innernode.prev;
					innernode.next->prev = this->prev;
					innernode.prev->next = this->next;
					//so LinkedNode parent doesnt remove itself again
					this->prev = nullptr;
				} else {
					innernode.next->prev = nullptr;
				}
			}
		}
	}
	/**
	 * Links the LifeCycleChain around the obj param.
	 * If the obj param was in a list before, the LifeCycleChain swaps its place and act as a reference to the obj param
	 */
	LinkedNode<NodeType>& link(LinkedNode<NodeType>* obj) {
		ASSERT(obj != nullptr) << "obj to link is null";
		ASSERT(innernode.next == &innernode) << "already linked";
		if (obj->prev != nullptr) {
			//obj is in a list, swap with me
			obj->prev->next = this;
			obj->next->prev = this;
			this->prev = obj->prev;
			this->next = obj->next;
		}
		obj->prev = &innernode;
		obj->next = &innernode;
		innernode.next = obj;
		innernode.prev = obj;

		return *this;
	}
	LinkedNode<NodeType>& link(LinkedNode<NodeType>& obj) {
		return this->link(&obj);
	}
	/**
	 * Calls delete on the held object, removes the LifeCycleChain from the enclosing list
	 */
	void kill() {
		if (innernode.next != &innernode) {
			GetType* obj = get();
			if (obj != nullptr) {
				delete obj;
				innernode.next = &innernode;
				innernode.prev = &innernode;
			} //else the get() removes us from the list, and sets innernode pointers
		}
		this->removeLinkFromList();
	}
	/**
	 * Removes this LifeCycleChain from the chain, leaving the held object in place
	 */
	void unlink() {
		if (innernode.next != &innernode) {
			if (this->prev != nullptr) {
				this->prev->next = innernode.next;
				this->next->prev = innernode.prev;
				innernode.next->prev = this->prev;
				innernode.prev->next = this->next;
				this->prev = nullptr;
			} else {
				innernode.next->prev = nullptr;
			}

			innernode.next = &innernode;
			innernode.prev = &innernode;
		}
		this->removeLinkFromList();
	}
	/**
	 * Gets the enclosed object pointer.
	 * nullptr, if the object was removed, destructed, or doesnt contains one.
	 */
	GetType* get() override {
		GetType* obj = nullptr;
		if (innernode.next != &innernode) {
			ASSERT(innernode.next != nullptr) << "innernode next is nullptr";
			ASSERT(innernode.prev != nullptr) << "innernode prev is nullptr";

			obj = static_cast<GetType*>(innernode.next->get());
		}
		if (obj == nullptr) {
			innernode.next = &innernode;
			innernode.prev = &innernode;

			this->removeLinkFromList();
		}
		return obj;
	}
};
}

#endif /* LIFECYCLECHAIN_H_ */
