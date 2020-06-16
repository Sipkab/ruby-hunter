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
 * MemberLinkedNode.h
 *
 *  Created on: 2014.08.22.
 *      Author: sipka
 */

#ifndef MEMBERLINKEDNODE_H_
#define MEMBERLINKEDNODE_H_
#include <framework/utils/LinkedNode.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>

namespace rhfw {

template<typename T>
class MemberLinkedNode final: public LinkedNode<T> {
	T* parent;
public:

	MemberLinkedNode(T* parent)
			: LinkedNode<T>(), parent(parent) {
	}
	MemberLinkedNode(LinkedNode<T>* prev, LinkedNode<T>* next, T* parent)
			: LinkedNode<T>(prev, next), parent(parent) {
	}
	MemberLinkedNode(MemberLinkedNode<T> && n) = delete;
	MemberLinkedNode(MemberLinkedNode<T> && n, T* parent)
			: LinkedNode<T>(util::move(n)), parent { parent } {
	}
	MemberLinkedNode<T>& operator=(MemberLinkedNode<T> && n) = delete;
	MemberLinkedNode(const MemberLinkedNode<T>&) = delete;
	MemberLinkedNode<T>& operator=(const MemberLinkedNode<T>&) = delete;

	MemberLinkedNode<T>& moveFrom(MemberLinkedNode<T> && n, T* parent) {
		LinkedNode<T>::operator =(util::move(n));
		this->parent = parent;
		return *this;
	}

	virtual ~MemberLinkedNode() {
	}

	virtual T* get() override {
		return parent;
	}
};
}

#endif /* MEMBERLINKEDNODE_H_ */
