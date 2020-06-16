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
 * ArrayList.h
 *
 *  Created on: 2014.06.08.
 *      Author: sipka
 */

#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

//owns the items
template<typename T>
class ArrayList {
private:
	T** data;
	int capacity;
	int mSize;

	void grow(int target) {
		if (target <= this->capacity) {
			return;
		}
		int start = this->capacity;
		int newcap = 1;
		while (target != 0) {
			target >>= 1;
			newcap <<= 1;
		}

		if (start < newcap) {
			T** old = data;

			capacity = newcap;
			data = new T*[capacity];

			for (int i = 0; i < mSize; i++) {
				data[i] = old[i];
			}
			delete[] old;
		}
	}
public:
	ArrayList()
			: capacity(32), mSize(0) {
		data = new T*[capacity];
	}
	ArrayList(ArrayList<T> && o)
			: data { o.data }, capacity { o.capacity }, mSize { o.mSize } {
		o.data = nullptr;
		o.mSize = 0;
		o.capacity = 0;
	}
	ArrayList(const ArrayList<T>& o)
			: capacity(o.capacity), mSize(o.mSize) {
		data = new T*[capacity];
		for (int i = 0; i < mSize; ++i) {
			this->data[i] = new T(*o.data[i]);
		}
	}

	ArrayList<T>& operator=(ArrayList<T> && o) {
		clear();
		delete[] data;

		this->data = o.data;
		this->mSize = o.mSize;
		this->capacity = o.capacity;

		o.data = nullptr;
		o.capacity = 0;
		o.mSize = 0;
		return *this;
	}
	ArrayList<T>& operator=(const ArrayList<T>& o) {
		clear();
		grow(o.mSize);
		this->mSize = o.mSize;
		for (int i = 0; i < mSize; ++i) {
			this->data[i] = new T(*o.data[i]);
		}
		return *this;
	}

	~ArrayList() {
		clear();
		delete[] data;
	}

	int indexOf(T* data) const {
		for (int i = 0; i < mSize; ++i) {
			if (this->data[i] == data) {
				return i;
			}
		}
		return -1;
	}
	int indexOfValue(T* data) const {
		for (int i = 0; i < mSize; ++i) {
			if (*(this->data[i]) == *data) {
				return i;
			}
		}
		return -1;
	}

	void reserveInFront(unsigned int count) {
		grow(mSize + count);
		for (int i = 1; i <= mSize; ++i) {
			data[mSize - i + count] = data[mSize - i];
		}
		for (int i = 0; i < count; ++i) {
			data[i] = nullptr;
		}
		mSize += count;
	}
	void reserveInEnd(unsigned int count) {
		grow(mSize + count);
		for (int i = 0; i < count; ++i) {
			data[mSize + i] = nullptr;
		}
		mSize += count;
	}

	void add(T* object) {
		grow(mSize + 1);

		data[mSize++] = object;
	}

	void add(int index, T* object) {
		ASSERT(index <= mSize);
		grow(mSize + 1);

		for (int i = mSize - 1; i >= index; i--) {
			data[i + 1] = data[i];
		}
		data[index] = object;
		mSize++;
	}

	void addAll(int index, const ArrayList<T>& l) {
		grow(mSize + l.mSize);

		for (int i = mSize - 1; i >= index; i--) {
			data[i + l.mSize] = data[i];
		}

		for (int i = 0; i < l.mSize; i++) {
			data[index + i] = new T(*l.data[i]);
		}
		mSize += l.mSize;
	}

	void addAll(const ArrayList<T>& l) {
		grow(mSize + l.mSize);

		for (int i = 0; i < l.mSize; i++) {
			data[mSize + i] = new T(*l.data[i]);
		}
		mSize += l.mSize;
	}

	void addRange(const ArrayList<T>& l, int start, int count) {
		grow(mSize + count);

		for (int i = 0; i < count; i++) {
			data[mSize + i] = new T(*l.data[i]);
		}
		mSize += count;
	}

	void addRange(int index, const ArrayList<T>& l, int start, int count) {
		grow(mSize + count);

		for (int i = mSize - 1; i >= index; i--) {
			data[i + count] = data[i];
		}

		for (int i = 0; i < count; i++) {
			data[index + i] = new T(*l.data[i]);
		}
		mSize += count;
	}

	T* remove(int index) {
		ASSERT(index < mSize);
		T* result = data[index];
		for (int i = index; i + 1 < mSize; i++) {
			data[i] = data[i + 1];
		}
		mSize--;
		return result;
	}

	T* removeOne(const T* data) {
		for (int i = 0; i < mSize; i++) {
			if (this->data[i] == data) {
				return remove(i);
			}
		}
		return nullptr;
	}

	void clear() {
		while (mSize > 0) {
			delete data[--mSize];
		}
	}
	void clearWithoutDelete() {
		mSize = 0;
	}

	template<typename Handler>
	void clearHandle(Handler&& h) {
		while (mSize > 0) {
			--mSize;
			h(data[mSize]);
			delete data[mSize];
		}
	}
	template<typename Handler>
	void clearHandleWithoutDelete(Handler&& h) {
		while (mSize > 0) {
			--mSize;
			h(data[mSize]);
		}
	}

	T* get(int index) {
		ASSERT(index < mSize);
		return data[index];
	}
	const T* get(int index) const {
		ASSERT(index < mSize);
		return data[index];
	}
	void set(int index, T* data) {
		ASSERT(index < mSize);
		delete this->data[index];
		this->data[index] = data;
	}

	T& operator[](int index) {
		ASSERT(index < mSize);
		return *data[index];
	}

	const T& operator[](int index) const {
		ASSERT(index < mSize);
		ASSERT(data[index] != nullptr);
		return *data[index];
	}

	int size() const {
		return mSize;
	}
	bool isEmpty() const {
		return mSize == 0;
	}

	T& last() {
		return *data[mSize - 1];
	}

	const T& last() const {
		return *data[mSize - 1];
	}

	void sort() {
		for (int i = 0; i < mSize - 1; ++i) {
			for (int j = i + 1; j < mSize; ++j) {
				if (*data[j] < *data[i]) {
					auto* tmp = data[i];
					data[i] = data[j];
					data[j] = tmp;
				}
			}
		}
	}
	template<typename Comparator>
	void sort(Comparator&& comp) {
		for (int i = 0; i < mSize - 1; ++i) {
			for (int j = i + 1; j < mSize; ++j) {
				if (comp(*data[j], *data[i])) {
					auto* tmp = data[i];
					data[i] = data[j];
					data[j] = tmp;
				}
			}
		}
	}

	T** begin() {
		return data;
	}
	T** end() {
		return data + mSize;
	}

	template<typename Comparator>
	int setSorted(T* value, Comparator&& comp) {
		int index = getIndexForSorted(value, util::forward<Comparator>(comp));
		if (index >= 0) {
			set(index, value);
		} else {
			index = -(index + 1);
			add(index, value);
		}
		return index;
	}

	template<typename ValueType, typename Comparator>
	int getIndexForSorted(ValueType&& value, Comparator&& comp) const {
		int low = 0;
		int mid = mSize;
		int high = mid - 1;
		int cmpresult = 1;
		while (low <= high) {
			mid = low + (high - low) / 2;
			cmpresult = comp(data[mid], value);
			if (cmpresult == 0) {
				return mid;
			} else if (cmpresult < 0) {
				low = mid + 1;
			} else {
				high = mid - 1;
			}
		}
		return -mid - (cmpresult > 0 ? 1 : 2);
	}

};

} // namespace rhfw

#endif /* ARRAYLIST_H_ */
