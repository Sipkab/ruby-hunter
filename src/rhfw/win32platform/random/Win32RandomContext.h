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
 * Win32RandomContext.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef WIN32PLATFORM_RANDOM_WIN32RANDOMCONTEXT_H_
#define WIN32PLATFORM_RANDOM_WIN32RANDOMCONTEXT_H_

#include <framework/random/RandomContext.h>
#include <framework/random/Randomer.h>
#include <win32platform/LibraryHandle.h>
#include <Wincrypt.h>

namespace rhfw {

class Win32RandomContext final: public RandomContextBase {
protected:
	LibraryHandle lib;

	HCRYPTPROV cryptoProvider = NULL;

	virtual bool load() override;
	virtual void free() override;
public:
	BOOL (WINAPI *CryptAcquireContextProto)(
			_Out_ HCRYPTPROV *phProv,
			_In_ LPCTSTR pszContainer,
			_In_ LPCTSTR pszProvider,
			_In_ DWORD dwProvType,
			_In_ DWORD dwFlags
	) = nullptr;

	BOOL (WINAPI *CryptReleaseContextProto)(
			_In_ HCRYPTPROV hProv,
			_In_ DWORD dwFlags
	) = nullptr;

	BOOL (WINAPI *CryptGenRandomProto)(
			_In_ HCRYPTPROV hProv,
			_In_ DWORD dwLen,
			_Inout_ BYTE *pbBuffer
	) = nullptr;

	Win32RandomContext();
	~Win32RandomContext();

	virtual Randomer* createRandomer() override;

	HCRYPTPROV getCryptoProvider() const {
		return cryptoProvider;
	}
};

} // namespace rhfw

#endif /* WIN32PLATFORM_RANDOM_WIN32RANDOMCONTEXT_H_ */
