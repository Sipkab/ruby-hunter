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
 * Win32AssetDescriptor.h
 *
 *  Created on: 2015 febr. 28
 *      Author: Bence
 */

#ifndef WIN32ASSETDESCRIPTOR_H_
#define WIN32ASSETDESCRIPTOR_H_

#include <framework/io/files/AssetFileDescriptor.h>

#include <win32platform/asset/Win32AssetFileInput.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

class Win32AssetDescriptor final : public AssetFileDescriptorBase {
private:
public:
	Win32AssetDescriptor() {
	}
	Win32AssetDescriptor(RAssetFile assetFileId)
			: AssetFileDescriptorBase { assetFileId } {
	}
	Win32AssetDescriptor(Win32AssetDescriptor&& o) = default;
	Win32AssetDescriptor& operator=(Win32AssetDescriptor&& o) = default;
	~Win32AssetDescriptor() {
	}

	Win32AssetFileInput* createInput() override {
		return new Win32AssetFileInput { assetFileId };
	}

	auto openInputStream() -> decltype(openAsStream(Win32AssetFileInput {assetFileId})) {
		return openAsStream(Win32AssetFileInput { assetFileId });
	}

};

}

#endif /* WIN32ASSETDESCRIPTOR_H_ */
