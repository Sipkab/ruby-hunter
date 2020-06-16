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
 * StreamData.h
 *
 *  Created on: 2016. apr. 10.
 *      Author: sipka
 */

#ifndef FRAMEWORK_AUDIO_STREAMDATA_H_
#define FRAMEWORK_AUDIO_STREAMDATA_H_

#include <framework/core/timing.h>
#include <framework/utils/LinkedNode.h>
#include <gen/fwd/types.h>

namespace rhfw {
namespace audio {

class StreamData {
private:
	typedef void (*StreamDataReleaser)(StreamData& data, void* param);
	StreamDataReleaser releaser;
	void* releaserParam;

	const char* data;
	unsigned int dataLength;

public:
	StreamData()
			: releaser { nullptr }, releaserParam { nullptr }, data { nullptr }, dataLength { 0 } {
	}
	StreamData(NULLPTR_TYPE)
		: releaser {nullptr}, releaserParam {nullptr}, data {nullptr}, dataLength {0} {
	}
	StreamData(StreamDataReleaser releaser, const char* data, unsigned int datalength, void* releaserparam =
			nullptr)
		: releaser {releaser}, releaserParam {releaserparam}, data {data}, dataLength {datalength} {
	}
	StreamData(const StreamData& o) = delete;
	StreamData& operator=(const StreamData& o) = delete;
	StreamData(StreamData&& o)
		: releaser {o.releaser}, releaserParam {o.releaserParam}, data {o.data}, dataLength {o.dataLength} {
		o.data = nullptr;
	}
	StreamData& operator=(NULLPTR_TYPE) {
		if (releaser != nullptr) {
			releaser(*this, releaserParam);
		}

		this->releaser = nullptr;
		this->releaserParam = nullptr;
		this->data = nullptr;
		this->dataLength = 0;

		return *this;
	}
	StreamData& operator=(StreamData&& o) {
		ASSERT(&o != this) << "Self move assignment";
		if (releaser != nullptr) {
			releaser(*this, releaserParam);
		}

		this->releaser = o.releaser;
		this->releaserParam = o.releaserParam;
		this->data = o.data;
		this->dataLength = o.dataLength;

		o.data = nullptr;
		o.releaser = nullptr;
		o.dataLength = 0;
		return *this;
	}
	~StreamData() {
		if (releaser != nullptr) {
			releaser(*this, releaserParam);
		}
	}

	bool operator==(NULLPTR_TYPE) {
		return data == nullptr;
	}
	bool operator!=(NULLPTR_TYPE) {
		return data != nullptr;
	}

	const char* getData() const {
		return data;
	}
	unsigned int getDataLength() const {
		return dataLength;
	}
};

} // namespace audio
} // namespace rhfw

#endif /* FRAMEWORK_AUDIO_STREAMDATA_H_ */
