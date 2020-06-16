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
 * WinstoreAssetDescriptor.h
 *
 *  Created on: 2015 febr. 28
 *      Author: Bence
 */

#ifndef WINSTOREASSETDESCRIPTOR_H_
#define WINSTOREASSETDESCRIPTOR_H_

#include <framework/io/files/AssetFileDescriptor.h>

#include <winstoreplatform/asset/WinstoreAssetFileInput.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class WinstoreAssetDescriptor final : public AssetFileDescriptorBase {
private:
public:
	WinstoreAssetDescriptor() {
	}
	WinstoreAssetDescriptor(RAssetFile assetFileId)
			: AssetFileDescriptorBase { assetFileId } {
	}
	WinstoreAssetDescriptor(WinstoreAssetDescriptor&& o) = default;
	WinstoreAssetDescriptor& operator=(WinstoreAssetDescriptor&& o) = default;
	~WinstoreAssetDescriptor() {
	}

	WinstoreAssetFileInput* createInput() override {
		return new WinstoreAssetFileInput { assetFileId };
	}

	auto openInputStream() -> decltype(openAsStream(WinstoreAssetFileInput {assetFileId})) {
		return openAsStream(WinstoreAssetFileInput { assetFileId });
	}
};

}

#endif /* WINSTOREASSETDESCRIPTOR_H_ */
