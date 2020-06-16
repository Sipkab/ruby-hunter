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
 * AppleAssetDescriptor.h
 *
 *  Created on: 2015 m�rc. 25
 *      Author: Bence
 */

#ifndef APPLEASSETDESCRIPTOR_H_
#define APPLEASSETDESCRIPTOR_H_

#include <framework/io/files/AssetFileDescriptor.h>
#include <appleplatform/asset/AppleAssetFileInput.h>

#include <gen/configuration.h>
namespace rhfw {

class AppleAssetDescriptor: public AssetFileDescriptorBase {
private:
public:
	AppleAssetDescriptor();
	AppleAssetDescriptor(RAssetFile assetId);
	AppleAssetDescriptor(AppleAssetDescriptor&& o) = default;
	AppleAssetDescriptor& operator=(AppleAssetDescriptor&& o) = default;
	~AppleAssetDescriptor();

	AppleAssetFileInput* createInput() override {
		return new AppleAssetFileInput { assetFileId };
	}
	auto openInputStream() -> decltype(openAsStream(AppleAssetFileInput {assetFileId})) {
		return openAsStream(AppleAssetFileInput { assetFileId });
	}
};

} // namespace rhfw

#endif /* APPLEASSETDESCRIPTOR_H_ */
