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
 * Resource.h
 *
 *  Created on: 2014.06.08.
 *      Author: sipka
 */

#ifndef RESOURCE_H_
#define RESOURCE_H_

#include <framework/resource/ShareableResource.h>
#include <framework/utils/utility.h>

#include <gen/log.h>
#include <gen/configuration.h>
#include <gen/platform.h>
#include <gen/fwd/types.h>

namespace rhfw {

class ResourceBlock {
private:
	template<typename, typename >
	friend class ResourceBase;
protected:
	ShareableResource* resource;
	int reference = 0;
	int handleCount = 0;
public:

	ResourceBlock(ShareableResource* resource, int referencecount = 0)
			: resource { resource }, reference(referencecount) {
	}
	ResourceBlock(ResourceBlock&& o)
			: resource { o.resource }, reference { o.reference }, handleCount { o.handleCount } {
		o.resource = nullptr;
	}
	ResourceBlock& operator=(ResourceBlock&& o) {
		ASSERT(this != &o) << "Self move assignment";
		this->resource = o.resource;
		this->reference = o.reference;
		this->handleCount = o.handleCount;

		o.resource = nullptr;
		return *this;
	}
	ResourceBlock(const ResourceBlock&) = delete;
	ResourceBlock& operator=(const ResourceBlock&) = delete;

	ShareableResource* replace(ShareableResource* res) {
		ShareableResource* old = this->resource;
		this->resource = res;
		return old;
	}

	ShareableResource* get() {
		return resource;
	}

	bool isLoaded() const {
		return reference > 0;
	}

	void addReference(int count) {
		reference += count;
	}
	int getReferenceCount() const {
		return reference;
	}
	void addHandles(int count) {
		handleCount += count;
	}
	int getHandleCount() const {
		return handleCount;
	}

	virtual ~ResourceBlock() {
		delete resource;
	}
};

template<typename T, typename ActionHandler>
class ResourceBase: private ActionHandler {
	//inherit to get empty base optimization
private:
	template<typename From, typename To>
	struct is_castable_to {
		typedef long long VALUE_YES;
		typedef char VALUE_NO;

		template<typename U>
		static VALUE_YES test(U*);
		template<typename U>
		static VALUE_NO test(...);

		static const bool value = sizeof(test<To>(static_cast<From*>(nullptr))) == sizeof(VALUE_YES);
	};

	template<typename, typename >
	friend class ResourceBase;
	typedef ResourceBase<T, ActionHandler> self_type;
	ResourceBlock* block;

	void invalidateBlock() {
		if (block != nullptr) {
			--block->handleCount;
			ActionHandler::_action_lose(*this);
			if (block->handleCount == 0) {
				delete block;
			}
		}
	}
protected:
public:
	//automatic conversions enabled if conversion type (CT) can be automatically casted to target type (T)
	//explicit required if CT cannot be automatically casted to T, and T can be static_cast-ed to CT

	ResourceBase()
			: block { nullptr } {
	}
	ResourceBase(NULLPTR_TYPE)
	: block {nullptr} {
	}
	explicit ResourceBase(ResourceBlock* block)
	: block {block} {
		if (block != nullptr) {
			++block->handleCount;
			ActionHandler::_action_get(*this);
		}
	}
	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	ResourceBase(const ResourceBase<CT, A>& o)
	: ResourceBase {o.block} {
	}
	template<typename CT, typename A, typename util::enable_if<!is_castable_to<CT, T>::value && is_castable_to<T, CT>::value>::type* =
	nullptr>
	explicit ResourceBase(const ResourceBase<CT, A>& o)
	: ResourceBase {o.block} {
	}
	ResourceBase(const self_type& o)
	: ResourceBase {o.block} {
	}
	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	ResourceBase(ResourceBase<CT, A> && o)
	: block {o.block} {
		if (block != nullptr) {
			ActionHandler::_action_get(*this);
			o._action_lose(o);
			o.block = nullptr;
		}
	}
	template<typename CT, typename A, typename util::enable_if<!is_castable_to<CT, T>::value && is_castable_to<T, CT>::value>::type* =
	nullptr>
	explicit ResourceBase(ResourceBase<CT, A> && o)
	: block {o.block} {
		if (block != nullptr) {
			ActionHandler::_action_get(*this);
			o._action_lose(o);
			o.block = nullptr;
		}
	}

	template<typename CT, typename A, typename Initer, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	ResourceBase(const rhfw::ResourceBase<CT, A>& o, Initer&& initer)
	: block {o.block} {
		if (block != nullptr) {
			++block->handleCount;
			initer(static_cast<T*>(block->resource));
			ActionHandler::_action_get(*this);
		}
	}
	template<typename CT, typename A, typename Initer, typename util::enable_if<
	!is_castable_to<CT, T>::value && is_castable_to<T, CT>::value>::type* = nullptr>
	explicit ResourceBase(const rhfw::ResourceBase<CT, A>& o, Initer&& initer)
	: block {o.block} {
		(void) static_cast<CT*>((T*) nullptr);
		if (block != nullptr) {
			++block->handleCount;
			initer(static_cast<T*>(block->resource));
			ActionHandler::_action_get(*this);
		}
	}
	template<typename CT, typename A, typename Initer, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	ResourceBase(rhfw::ResourceBase<CT, A> && o, Initer&& initer)
	: block {o.block} {
		if (block != nullptr) {
			initer(static_cast<T*>(block->resource));
			ActionHandler::_action_get(*this);
			o._action_lose(o);
			o.block = nullptr;
		}
	}
	template<typename CT, typename A, typename Initer, typename util::enable_if<
	!is_castable_to<CT, T>::value && is_castable_to<T, CT>::value>::type* = nullptr>
	explicit ResourceBase(rhfw::ResourceBase<CT, A> && o, Initer&& initer)
	: block {o.block} {
		if (block != nullptr) {
			initer(static_cast<T*>(block->resource));
			ActionHandler::_action_get(*this);
			o._action_lose(o);
			o.block = nullptr;
		}
	}

	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	self_type& operator=(const ResourceBase<CT, A>& o) {
		invalidateBlock();

		this->block = o.block;
		if (this->block != nullptr) {
			++this->block->handleCount;
			ActionHandler::_action_get(*this);
		}
		return *this;
	}
	self_type& operator=(const self_type& o) {
		invalidateBlock();

		this->block = o.block;
		if (this->block != nullptr) {
			++this->block->handleCount;
			ActionHandler::_action_get(*this);
		}
		return *this;
	}
	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value>::type* = nullptr>
	self_type& operator=(ResourceBase<CT, A> && o) {
		invalidateBlock();

		this->block = o.block;
		if (o.block != nullptr) {
			ActionHandler::_action_get(*this);
			o._action_lose(o);
			o.block = nullptr;
		}
		return *this;
	}
	self_type& operator=(NULLPTR_TYPE) {
		if (block != nullptr) {
			--block->handleCount;
			ActionHandler::_action_lose(*this);
			if (block->handleCount == 0) {
				delete block;
			}
			this->block = nullptr;
		}
		return *this;
	}
	~ResourceBase() {
		invalidateBlock();
	}
	operator T*() {
		ASSERT(block != nullptr) << "Resource points to nullptr";
		return static_cast<T*>(block->resource);
	}
	template<typename CastT, typename util::enable_if<!util::is_same<CastT, T>::value>::type* = nullptr>
	explicit operator CastT*() {
		return static_cast<CastT*>(static_cast<T*>(*this));
	}
	T* operator->() {
		ASSERT(block != nullptr) << "Resource points to nullptr";
		return static_cast<T*>(block->resource);
	}
	T& operator*() {
		ASSERT(block != nullptr) << "Resource points to nullptr";
		ASSERT(block->resource != nullptr) << "Resource block points to nullptr";
		return *static_cast<T*>(block->resource);
	}
	operator T&() {
		return **this;
	}

	T* replace(T* nres) {
		ASSERT(block != nullptr) << "Resource block is nullptr, can't replace";
		T* oldres = static_cast<T*>(block->resource);
		block->resource = nres;
		return oldres;
	}

	bool operator==(NULLPTR_TYPE) const {
		return block == nullptr;
	}
	bool operator!=(NULLPTR_TYPE) const {
		return block != nullptr;
	}

	bool operator==(T* ptr) const {
		return block != nullptr && block->resource == ptr;
	}
	bool operator!=(T* ptr) const {
		return block == nullptr || block->resource != ptr;
	}

	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value || is_castable_to<T, CT>::value>::type* =
	nullptr>
	bool operator==(const ResourceBase<CT, A>& o) const {
		return block == o.block;
	}
	template<typename CT, typename A, typename util::enable_if<is_castable_to<CT, T>::value || is_castable_to<T, CT>::value>::type* =
	nullptr>
	bool operator!=(const ResourceBase<CT, A>& o) const {
		return block != o.block;
	}

	ShareableResource*& getPointerReference() {
		ASSERT(block != nullptr) << "Resource points to nullptr";
		return block->resource;
	}
	bool load() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		++block->reference;
		if (block->reference == 1) {
			if (block->resource->load()) {
				return true;
			} else {
				LOGW()<< "Failed to load resource";
				block->reference = 0;
				return false;
			}
		}
		return true;
	}
	void free() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		ASSERT(block->reference > 0) << "Freeing resource while reference (" << block->reference << ") <= 0";
		if (block->reference > 0) {
			--block->reference;
			if (block->reference == 0) {
				block->resource->free();
			}
		}
	}
	void freeIfLoaded() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		if (block->reference > 0) {
			free();
		}
	}
	bool reload() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		return block->resource->reload();
	}
	bool reloadIfLoaded() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		if (block->reference > 0) {
			return block->resource->reload();
		}
		return true;
	}
	bool loadOrReload() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		if (block->reference == 0) {
			return load();
		}
		return block->resource->reload();
	}

	bool isLoaded() const {
		ASSERT(block != nullptr) << "Resource points to nullptr";

		return block->reference > 0;
	}
};
namespace resourcedetail {

class VoidAction {
public:
	template<typename Param>
	void _action_get(Param& p) {
	}
	template<typename Param>
	void _action_lose(Param& p) {
	}
};
class AutoLoaderAction {
private:
	bool loadSuccess = false;
public:
	template<typename Param>
	void _action_get(Param& p) {
		loadSuccess = p.load();
	}
	template<typename Param>
	void _action_lose(Param& p) {
		if (loadSuccess) {
			p.free();
			//dont need to set loadSucces to false, since _action_lose will not be called before _action_get
		}
	}
};

}  // namespace resourcedetail
template<typename T>
using Resource = ResourceBase<T, resourcedetail::VoidAction>;
template<typename T>
using AutoResource = ResourceBase<T, resourcedetail::AutoLoaderAction>;

}  // namespace rhfw

#endif /* RESOURCE_H_ */
