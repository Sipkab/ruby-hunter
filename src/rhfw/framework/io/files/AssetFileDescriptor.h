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
 * AssetFileDescriptor.h
 *
 *  Created on: 2015 okt. 25
 *      Author: sipka
 */

#ifndef ASSETFILEDESCRIPTOR_H_
#define ASSETFILEDESCRIPTOR_H_

#include <framework/io/files/FileDescriptor.h>
#include <framework/utils/InputSource.h>
#include <framework/utils/utility.h>
#include <framework/io/stream/BufferedInputStream.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OwnerStream.h>
#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/assets.h>
#include <gen/log.h>
#include <gen/platform.h>

namespace rhfw {

class AssetFileDescriptorBase: public FileDescriptor {
protected:
	RAssetFile assetFileId;

	AssetFileDescriptorBase(RAssetFile assetFileId = RAssetFile::INVALID_ASSET_IDENTIFIER)
			: assetFileId { assetFileId } {
	}
	AssetFileDescriptorBase(AssetFileDescriptorBase&&) = default;
	AssetFileDescriptorBase(const AssetFileDescriptorBase&) = default;
	AssetFileDescriptorBase& operator=(AssetFileDescriptorBase&&) = default;
	AssetFileDescriptorBase& operator=(const AssetFileDescriptorBase&) = default;

	template<typename FileInputType>
	auto openAsStream(
			FileInputType* input) -> decltype(EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(OwnerInputStream::wrap(input)))) {
		input->open();
		return EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(OwnerInputStream::wrap(input)));
	}
	template<typename FileInputType>
	auto openAsStream(FileInputType&& input) //
			-> decltype(EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(util::forward<FileInputType>(input)))) {
		input.open();
		return EndianInputStream<Endianness::Big>::wrap(BufferedInputStream::wrap(util::forward<FileInputType>(input)));
	}
public:
	FileOutput* createOutput() override final {
		THROW()<<"Invalid call, Asset files do not have output available";
		return nullptr;
	}

	virtual bool exists() override final {
		return true;
	}
	virtual bool isDirectory() override final {
		return false;
	}
	virtual bool isFile() override final {
		return true;
	}

	virtual bool create() override final {
		return true;
	}
	virtual bool remove() override final {
		return false;
	}
};

}
 // namespace rhfw

#include ASSETFILEDESCRIPTOR_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class ASSETFILEDESCRIPTOR_EXACT_CLASS_TYPE AssetFileDescriptor;
} // namespace rhfw

namespace rhfw {

class AssetInputSource: public InputSource {
private:
AssetFileDescriptor assetfd;

virtual void closeData(InputSource::Data& data) override {
	delete[] static_cast<const char*>(data);
}
public:
AssetInputSource(AssetFileDescriptor afd)
		: assetfd { util::move(afd) } {
}
AssetInputSource(RAssetFile asset)
		: assetfd { asset } {
}

virtual InputSource::Data getData() override {
	unsigned int len;
	const char* data = assetfd.readFully(&len);
	return {this, data, len};
}
};

}  // namespace rhfw

#endif /* ASSETFILEDESCRIPTOR_H_ */
