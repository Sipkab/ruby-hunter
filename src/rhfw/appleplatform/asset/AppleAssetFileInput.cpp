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
//  AppleAssetFileInput.cpp
//  TestApp
//
//  Created by User on 2016. 03. 01..
//  Copyright © 2016. User. All rights reserved.
//

#include <appleplatform/asset/AppleAssetFileInput.h>

#include <CoreFoundation/CFURL.h>
#include <CoreFoundation/CFBundle.h>

namespace rhfw {

bool AppleAssetFileInput::getAssetFilePath(char * result, RAssetFile assetid, unsigned int maxLength) {
	CFStringRef fname = CFStringCreateWithFormat(0, 0, CFSTR("%x"), (unsigned int) assetid);
	if (fname == 0) {
		LOGWTF()<< "Failed to create string for asset path: " << assetid;
		return false;
	}

	CFURLRef fileref = CFBundleCopyResourceURL(CFBundleGetMainBundle(), fname, nullptr, CFSTR("assets/res"));
	if (fileref == 0) {
		LOGWTF()<< "Failed to get asset reference for: " << assetid;
		CFRelease(fname);
		return false;
	}

	Boolean bresult = CFURLGetFileSystemRepresentation(fileref, true, (UInt8*) result, maxLength);

	CFRelease(fileref);
	CFRelease(fname);

	if (!bresult) {
		LOGWTF()<< "Failed to get asset system representation for: " << assetid;
		return false;
	}
	return true;
}

} // namespace rhfw

