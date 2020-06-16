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
 * ContainerLinkedNode.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef CONTAINERLINKEDNODE_H_
#define CONTAINERLINKEDNODE_H_

#include <framework/utils/LinkedNode.h>
#include <framework/utils/utility.h>

namespace rhfw {

template<typename T>
class ContainerLinkedNode final: public LinkedNode<T> {
	T data;
public:

	template<typename ... Args>
	ContainerLinkedNode(Args ... args)
			: LinkedNode<T>(), data { util::forward<Args>(args)... } {
	}
	template<typename ... Args>
	ContainerLinkedNode(LinkedNode<T>* prev, LinkedNode<T>* next, Args ... args)
			: LinkedNode<T>(prev, next), data { util::forward<Args>(args)... } {
	}
	ContainerLinkedNode(ContainerLinkedNode<T> && n)
			: LinkedNode<T>(util::move(n)), data { util::move(n.data) } {
	}
	ContainerLinkedNode<T>& operator=(ContainerLinkedNode<T> && n) {
		LinkedNode<T>::operator =(util::move(n));
		data = util::move(n.data);
		return *this;
	}
	ContainerLinkedNode(const ContainerLinkedNode<T>&) = delete;
	ContainerLinkedNode<T>& operator=(const ContainerLinkedNode<T>&) = delete;

	virtual ~ContainerLinkedNode() {
	}

	virtual T* get() override final {
		return &data;
	}

	operator T&() {
		return data;
	}
	operator const T&() const {
		return data;
	}

	T* operator->() {
		return &data;
	}
	const T* operator->() const {
		return &data;
	}
};

}  // namespace rhfw

#endif /* CONTAINERLINKEDNODE_H_ */
