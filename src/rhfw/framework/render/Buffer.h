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
//
//  Buffer.h
//  TestApp
//
//  Created by User on 2016. 03. 23..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef RENDER_BUFFER_H_
#define RENDER_BUFFER_H_

#include <framework/utils/utility.h>
#include <framework/utils/WriteOnlyPointer.h>
#include <framework/render/RenderObject.h>

#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {
namespace render {

class Buffer;

class BufferInitializer {
	friend class Buffer;
protected:
	unsigned int byteCount;
public:
	BufferInitializer(Buffer* buffer, unsigned int bytecount = 0)
			: byteCount { bytecount } {
	}
	unsigned int getBufferByteCount() {
		return byteCount;
	}

	virtual ~BufferInitializer() = default;

	virtual void initializeData(WriteOnlyPointer<char> data) = 0;
};

class Buffer: public RenderObject {
	friend class Renderer;
private:
	virtual bool loadImpl() = 0;
	virtual void freeImpl() = 0;
	virtual bool reloadImpl() {
		THROW()<<"Reload not implemented";
		return false;
	}
protected:
	class MapSettings {
	public:
		void* ptr = nullptr;
		unsigned int start = 0;
		unsigned int bytecount = 0;

		MapSettings()
		: ptr {nullptr}, start {0}, bytecount {0} {
		}
		MapSettings(void* ptr, unsigned int start, unsigned int bytecount)
		: ptr {ptr}, start {start}, bytecount {bytecount} {
		}
	};
	virtual bool load() override final {
		this->bufferType = BufferType::UNINITIALIZED;
		this->allocatedBytes = 0;

		return loadImpl();
	}
	virtual void free() override final {
		freeImpl();

		//Do not clear bufferType and allocatedBytes
		//those values could be used by using operator=(Buffer&&).
		//clear then in load()
	}
	virtual bool reload() override final {
		bool res = reloadImpl();
		if (res) {
			this->reinitialize();
		}
		return res;
	}
	virtual bool reloadInNewContext() {
		if(loadImpl()) {
			reinitialize();
			return true;
		}
		return false;
	}
private:
	BufferType bufferType = BufferType::UNINITIALIZED;
	unsigned int allocatedBytes = 0;
	BufferInitializer* bufferInitializer = nullptr;

	virtual MapSettings mapImpl(unsigned int start, unsigned int bytecount) {
		char* data = new char[bytecount];
		return {data, start, bytecount};
	}
	virtual void unmapImpl(MapSettings& data) {
		updateRegionImpl(data.ptr, data.start, data.bytecount);
		delete[] reinterpret_cast<char*>(data.ptr);
	}
	virtual void allocateDataImpl(const void* data, unsigned int bytecount, BufferType bufferType) = 0;
	virtual void updateRegionImpl(const void* data, unsigned int start, unsigned int bytecount) = 0;

	class Mapper {
	private:
		Buffer* buffer;
		MapSettings mapData;

	public:
		Mapper() : buffer {nullptr} {
		}
		Mapper(Buffer* buffer, unsigned int start, unsigned int bytecount)
		: buffer {buffer} {
			ASSERT(bytecount > 0) << "Creating mapper for zero bytes";
			ASSERT(buffer->bufferType == BufferType::DYNAMIC) << "Buffertype is not valid for region update: " << buffer->bufferType;
			ASSERT(start + bytecount <= buffer->allocatedBytes) << "Buffer out of bounds, allocated: " << buffer->allocatedBytes << ", start: " << start << ", bytecount: " << bytecount;

			mapData = buffer->mapImpl(start, bytecount);
		}
		Mapper(const Mapper&) = delete;
		Mapper& operator=(const Mapper&) = delete;
		Mapper(Mapper&& o)
		: buffer {o.buffer}, mapData {o.mapData} {
			o.mapData = {};
			o.buffer = nullptr;
		}
		Mapper& operator=(Mapper&& o) {
			ASSERT(this != &o);

			this->buffer = o.buffer;
			this->mapData = o.mapData;

			o.mapData = {};
			o.buffer = nullptr;
			return *this;
		}
		~Mapper() {
			if (buffer != nullptr) {
				commit();
			}
		}
		void commit() {
			ASSERT(buffer != nullptr);
			buffer->unmapImpl(mapData);
			buffer = nullptr;
		}
		unsigned int getStart() const {
			return mapData.start;
		}
		unsigned int getByteCount() const {
			return mapData.bytecount;
		}
		void* pointer() {
			return mapData.ptr;
		}
		template<typename T>
		WriteOnlyObject<T> getAtIndex(unsigned int index) {
			ASSERT(buffer != nullptr) << "Invalid creator";
			ASSERT(index * sizeof(T) <= getByteCount() - sizeof(T)) << "Map out of bounds, index: " << index << ", bytecount: " << getByteCount() << ", address: " << index * sizeof(T);
			return reinterpret_cast<T*>(pointer()) + index;
		}
	};
	class DataInitializer {
	private:
		Buffer* buffer;
		char* initdata;
	public:
		DataInitializer() : buffer {nullptr}, initdata {nullptr} {
		}
		DataInitializer(Buffer* buffer, char* initdata) : buffer {buffer}, initdata {initdata} {
		}
		DataInitializer(DataInitializer&& o) : buffer {o.buffer}, initdata {o.initdata} {
			o.buffer = nullptr;
		}
		DataInitializer& operator=(DataInitializer&& o) {
			ASSERT(this != &o);

			this->buffer = o.buffer;
			this->initdata = o.initdata;

			o.buffer = nullptr;
			return *this;
		}
		~DataInitializer() {
			if(buffer != nullptr) {
				commit();
			}
		}
		void commit() {
			ASSERT(buffer != nullptr);
			buffer->allocateDataImpl(initdata, buffer->allocatedBytes, buffer->bufferType);
			delete[] initdata;
			buffer = nullptr;
		}
		void* pointer() {
			return initdata;
		}
		unsigned int getByteCount() const {
			return buffer->allocatedBytes;
		}

		template<typename T>
		WriteOnlyObject<T> getAtIndex(unsigned int index) {
			ASSERT(buffer != nullptr) << "Invalid creator";
			ASSERT(index * sizeof(T) <= getByteCount() - sizeof(T)) << "Map out of bounds, index: " << index << ", bytecount: " << getByteCount() << ", address: "
			<< index * sizeof(T);
			return reinterpret_cast<T*>(pointer()) + index;
		}

	};
	template<typename T, typename MAPCLASS>
	class StructuredWrapper {
	private:
		MAPCLASS map;
	public:
		StructuredWrapper() = default;
		StructuredWrapper(MAPCLASS&& map) : map {util::move(map)} {
		}
		StructuredWrapper(StructuredWrapper<T, MAPCLASS>&&) = default;
		StructuredWrapper& operator=(StructuredWrapper&&) = default;

		WriteOnlyObject<T> operator[](unsigned int index) {
			return map.template getAtIndex<T>(index);
		}
		WriteOnlyObject<T> operator*() {
			return reinterpret_cast<T*>(map.pointer());
		}

		operator WriteOnlyPointer<T>() {
			return reinterpret_cast<T*>(map.pointer());
		}

		WriteOnlyPointer<T> begin() {
			return reinterpret_cast<T*>(map.pointer());
		}
		WriteOnlyPointer<T> end() {
			return reinterpret_cast<T*>(reinterpret_cast<char*>(map.pointer()) + map.getByteCount());
		}

		void commit() {
			map.commit();
		}
	};
public:
	template<typename T>
	using StructuredMapper = StructuredWrapper<T, Mapper>;
	template<typename T>
	using StructuredInitializer = StructuredWrapper<T, DataInitializer>;

public:

	Buffer() {
	}
	Buffer(Buffer&& o) = delete;
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;
	Buffer& operator=(Buffer&& o) {
		delete this->bufferInitializer;
		this->bufferInitializer = o.bufferInitializer;
		if (o.bufferType != BufferType::UNINITIALIZED) {
			if (this->bufferInitializer == nullptr) {
				this->allocate<char>(o.allocatedBytes, o.bufferType);
			} else {
				this->initialize(o.bufferType);
			}
		} else {
			this->bufferType = BufferType::UNINITIALIZED;
			this->allocatedBytes = 0;
		}

		o.bufferInitializer = nullptr;
		o.bufferType = BufferType::UNINITIALIZED;
		o.allocatedBytes = 0;

		return *this;
	}
	~Buffer() {
		delete this->bufferInitializer;
	}

	void removeBufferInitializer() {
		delete this->bufferInitializer;
		this->bufferInitializer = nullptr;
	}
	void setBufferInitializer(BufferInitializer* initializer) {
		delete this->bufferInitializer;

		this->bufferInitializer = initializer;

		this->reinitialize();
	}
	template<typename ElementT, typename FunctorT>
	void setBufferInitializer(FunctorT&& functor, unsigned int elementcount) {
		class FunctorInitializer : public BufferInitializer {
		private:
			FunctorT initer;
		public:
			FunctorInitializer(Buffer* buffer, unsigned int bytecount, FunctorT&& initer)
			: BufferInitializer {buffer, bytecount}, initer {util::forward<FunctorT>(initer)} {
			}
			virtual void initializeData(WriteOnlyPointer<char> data) override {
				initer(static_cast<ElementT*>(data.reinterpretCast<ElementT>()));
			}
		};
		auto* bufiniter = new FunctorInitializer {this, static_cast<unsigned int>(elementcount * sizeof(ElementT)), util::forward<FunctorT>(functor)};
		setBufferInitializer(bufiniter);
	}

	template<typename T>
	StructuredMapper<T> mapped(unsigned int start, unsigned int elementcount) {
		ASSERT(bufferType != BufferType::IMMUTABLE) << "Only not IMMUTABLE buffers may be mapped, type: " << bufferType;
		return { {this, static_cast<unsigned int>(start * sizeof(T)), static_cast<unsigned int>(elementcount * sizeof(T))}};
	}

	/**
	 * Updates the buffer,
	 * takes elementcount * sizeof(T) bytes from pointer data
	 * places them starting at index [start * sizeof(T)]
	 */
	template<typename T>
	void updateRegion(const T* data, unsigned int start, unsigned int elementcount) {
		ASSERT(bufferType != BufferType::IMMUTABLE) << "Only not IMMUTABLE buffers may be updated, type: " << bufferType;
		ASSERT(start * sizeof(T) + elementcount * sizeof(T) <= allocatedBytes) <<
		"Buffer out of bounds, allocated: " << allocatedBytes << ", start: " << start << ", elementcount: " << elementcount;

		updateRegionImpl(data, static_cast<unsigned int>(start * sizeof(T)), static_cast<unsigned int>(elementcount * sizeof(T)));
	}

	template<typename T>
	void allocate(unsigned int elementcount, BufferType type) {
		ASSERT(elementcount != 0) << "Elementcount is zero";
		ASSERT(type != BufferType::IMMUTABLE) << "Render buffer type cannot be immutable without initializer data";

		this->allocatedBytes = static_cast<unsigned int>(elementcount * sizeof(T));
		this->bufferType = type;

		allocateDataImpl(nullptr, this->allocatedBytes, this->bufferType);
	}
	void initialize(BufferType type) {
		ASSERT(bufferInitializer != nullptr) << "No initializer specified for buffer";

		this->allocatedBytes = bufferInitializer->getBufferByteCount();
		this->bufferType = type;

		char* initdata = new char[this->allocatedBytes];
		LOGI() << "Initialize with size: " << this->allocatedBytes;
		bufferInitializer->initializeData(initdata);
		allocateDataImpl(initdata, this->allocatedBytes, this->bufferType);
		delete[] initdata;
	}

	template<typename T>
	StructuredInitializer<T> initializer(BufferType type) {
		ASSERT(bufferInitializer != nullptr) << "No initializer specified for buffer";

		this->allocatedBytes = bufferInitializer->getBufferByteCount();
		this->bufferType = type;

		char* initdata = new char[this->allocatedBytes];
		bufferInitializer->initializeData(initdata);

		return { {this, initdata}};
	}
	template<typename T>
	StructuredInitializer<T> initializer(unsigned int elementcount, BufferType type) {
		ASSERT(bufferInitializer == nullptr) << "Buffer initializer is not nullptr. Use the function initializer without parameter elementcount.";

		this->allocatedBytes = static_cast<unsigned int>(elementcount * sizeof(T));
		this->bufferType = type;

		char* initdata = new char[this->allocatedBytes];

		return { {this, initdata}};
	}

	unsigned int getAllocatedByteCount() const {
		return allocatedBytes;
	}

	void reinitialize() {
		if(bufferType == BufferType::UNINITIALIZED) {
			return;
		}
		if(bufferInitializer == nullptr) {
			this->allocate<unsigned char>(allocatedBytes, bufferType);
			return;
		}
		this->initialize(bufferType);
	}
	bool isInitialized() const {
		return bufferType != BufferType::UNINITIALIZED;
	}
	template<typename T>
	void setBufferInitializerLength(unsigned int elementcount) {
		ASSERT(bufferInitializer != nullptr) << "Buffer initializer is nullptr";
		ASSERT(elementcount != 0) << "Cannot set buffer length to zero";

		if (bufferInitializer->byteCount == static_cast<unsigned int>(elementcount * sizeof(T))) {
			return;
		}

		bufferInitializer->byteCount = static_cast<unsigned int>(elementcount * sizeof(T));
		this->reinitialize();
	}
};

}
		// namespace render
}		// namespace rhfw

#endif /* RENDER_BUFFER_H_ */
