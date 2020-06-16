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
 * InputSource.h
 *
 *  Created on: 2016. marc. 8.
 *      Author: sipka
 */

#ifndef INPUTSOURCE_H_
#define INPUTSOURCE_H_

#include <gen/log.h>
#include <gen/fwd/types.h>

namespace rhfw {

class InputSource {
public:
	class Data {
	private:
		InputSource* input;

		const void* ptr;
		unsigned int length;
	public:
		Data()
				: input { nullptr }, ptr { nullptr }, length { 0 } {
		}
		Data(InputSource* input, const void* ptr, unsigned int length)
				: input { input }, ptr { ptr }, length { length } {
		}
		Data(Data&&) = default;
		Data& operator=(Data&&) = default;
		Data(const Data&) = delete;
		Data& operator=(const Data&) = delete;

		~Data() {
			if (isSuccess()) {
				input->closeData(*this);
			}
		}

		bool isSuccess() const {
			return input != nullptr;
		}

		unsigned int getLength() const {
			return length;
		}
		template<typename T>
		operator const T*() const {
			return static_cast<const T*>(ptr);
		}

		bool operator==(NULLPTR_TYPE)const {
			return ptr == nullptr;
		}
	};
	InputSource() = default;
	InputSource(const InputSource&) = default;
	InputSource(InputSource&&) = default;
	InputSource& operator=(const InputSource&) = default;
	InputSource& operator=(InputSource&&) = default;
	virtual ~InputSource() = default;

	virtual InputSource::Data getData() = 0;

private:
	virtual void closeData(InputSource::Data& data) = 0;
public:
};

}  // namespace rhfw

#endif /* INPUTSOURCE_H_ */
