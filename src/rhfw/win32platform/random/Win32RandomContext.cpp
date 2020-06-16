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
 * Win32RandomContext.cpp
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#include <win32platform/random/Win32RandomContext.h>
#include <framework/random/PseudoRandomer.h>
#include <Wincrypt.h>

namespace rhfw {

class Win32Randomer final: public Randomer {
	Win32RandomContext* context;
public:
	Win32Randomer(Win32RandomContext* context)
			: context(context) {
	}

	virtual int read(void* buffer, unsigned int count) override {
		if (context->CryptGenRandomProto(context->getCryptoProvider(), count, (BYTE*) buffer)) {
			return count;
		}
		return -1;
	}
};

Win32RandomContext::Win32RandomContext() {
}
Win32RandomContext::~Win32RandomContext() {
}

bool Win32RandomContext::load() {
	lib = LibraryHandle { "Advapi32.dll" };
	WARN(!lib) << "Failed to load library";
	if (lib) {
		CryptAcquireContextProto = (decltype(CryptAcquireContextProto)) GetProcAddress((HMODULE) lib, "CryptAcquireContextA");
		if (CryptAcquireContextProto == nullptr) {
			lib.free();
			LOGTRACE()<< "Failed to lookup function: " << GetLastError();
			return false;
		}
		CryptReleaseContextProto = (decltype(CryptReleaseContextProto)) GetProcAddress((HMODULE) lib, "CryptReleaseContext");
		if (CryptReleaseContextProto == nullptr) {
			lib.free();
			LOGTRACE()<< "Failed to lookup function: " << GetLastError();
			return false;
		}
		CryptGenRandomProto = (decltype(CryptGenRandomProto)) GetProcAddress((HMODULE) lib, "CryptGenRandom");
		if (CryptGenRandomProto == nullptr) {
			lib.free();
			LOGTRACE()<< "Failed to lookup function: " << GetLastError();
			return false;
		}

		if (!CryptAcquireContextProto(&cryptoProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
			lib.free();
			LOGTRACE()<< "Failed to acquire context: " << GetLastError();
			return false;
		}
	}
	return true;
}

void Win32RandomContext::free() {
	if (cryptoProvider != NULL) {
		CryptReleaseContextProto(cryptoProvider, 0);
	}
	lib.free();
}

Randomer* Win32RandomContext::createRandomer() {
	if (lib) {
		return new Win32Randomer(this);
	}
	return new PseudoRandomer(GetTickCount());
}

} // namespace rhfw

