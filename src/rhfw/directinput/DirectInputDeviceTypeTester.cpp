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
 * DirectInputDeviceTypeTester.cpp
 *
 *  Created on: 2017. aug. 24.
 *      Author: sipka
 */

#include <directinput/DirectInputDeviceTypeTester.h>

#include <wbemidl.h>
#include <oleauto.h>

#include <cwchar>

#include <gen/log.h>

namespace rhfw {

//SysAllocString
//SysFreeString
//TODO link oleaut32 dynamically?
//#pragma comment(lib, "oleaut32.lib")

//-----------------------------------------------------------------------------
// Enum each PNP device using WMI and check each device ID to see if it contains
// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
// Unfortunately this information can not be found by just using DirectInput
//-----------------------------------------------------------------------------
DirectInputDeviceTypeTester::DirectInputDeviceTypeTester() {
//	bstrDeviceID = SysAllocString(L"DeviceID");
//	if (bstrDeviceID == NULL) {
//		goto LCleanup;
//	}
//
//	HRESULT hr;
//
//	// Create WMI
//	hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*) &pIWbemLocator);
//	if (FAILED(hr) || pIWbemLocator == NULL) {
//		goto LCleanup;
//	}
//
//	// Connect to WMI
//	BSTR bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
//	if (bstrNamespace == NULL) {
//		goto LCleanup;
//	}
//	hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
//	SysFreeString(bstrNamespace);
//	if (FAILED(hr) || pIWbemServices == NULL) {
//		goto LCleanup;
//	}
//
//	// Switch security level to IMPERSONATE.
//	CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
//			EOAC_NONE);
//
//	BSTR bstrClassName = SysAllocString(L"Win32_PNPEntity");
//	if (bstrClassName == NULL) {
//		goto LCleanup;
//	}
//	hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
//	SysFreeString(bstrClassName);
//	if (FAILED(hr) || pEnumDevices == NULL) {
//		goto LCleanup;
//	}
//
//	//dont cleanup
//	return;
//
//	LCleanup:
//
//	if (pEnumDevices != nullptr) {
//		pEnumDevices->Release();
//		pEnumDevices = nullptr;
//	}
//	if (pIWbemLocator != nullptr) {
//		pIWbemLocator->Release();
//		pIWbemLocator = nullptr;
//	}
//	if (pIWbemServices != nullptr) {
//		pIWbemServices->Release();
//		pIWbemServices = nullptr;
//	}
}
DirectInputDeviceTypeTester::~DirectInputDeviceTypeTester() {
//	if (bstrDeviceID != NULL) {
//		SysFreeString(bstrDeviceID);
//	}
//	if (pEnumDevices != nullptr) {
//		pEnumDevices->Release();
//	}
//	if (pIWbemLocator != nullptr) {
//		pIWbemLocator->Release();
//	}
//	if (pIWbemServices != nullptr) {
//		pIWbemServices->Release();
//	}
}


//BOOL DirectInputDeviceTypeTester::IsXInputDevice(const GUID* pGuidProductFromDirectInput) {
//	if (pEnumDevices == nullptr) {
//		//treat all devices as xinput devices
//		LOGI()<< "All devices are treated as XInput devices";
//		return true;
//	}
//	IWbemClassObject* pDevices[20] = { 0 };
//	bool bIsXinputDevice = false;
//	HRESULT hr;
//
//	// Loop over all devices
//	for (;;) {
//		// Get 20 at a time
//		DWORD uReturned = 0;
//		hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
//		if (FAILED(hr)) {
//			THROW()<< hr;
//			goto LCleanup;
//		}
//		if (uReturned == 0) {
//			break;
//		}
//
//		for (UINT iDevice = 0; iDevice < uReturned; iDevice++) {
//			// For each device, get its device ID
//			VARIANT var;
//			hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
//			if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
//				// Check if the device ID contains "IG_".  If it does, then it's an XInput device
//				// This information can not be found from DirectInput
//				if (wcsstr(var.bstrVal, L"IG_")) {
//					// If it does, then get the VID/PID from var.bstrVal
//					DWORD dwPid = 0, dwVid = 0;
//					WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
//					if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
//						dwVid = 0;
//					WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
//					if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
//						dwPid = 0;
//
//					// Compare the VID/PID to the DInput device
//					DWORD dwVidPid = MAKELONG(dwVid, dwPid);
//					if (dwVidPid == pGuidProductFromDirectInput->Data1) {
//						bIsXinputDevice = true;
//						goto LCleanup;
//					}
//				}
//			}
//			if (pDevices[iDevice] != nullptr) {
//				pDevices[iDevice]->Release();
//				pDevices[iDevice] = nullptr;
//			}
//		}
//	}
//
//	LCleanup:
//
//	for (UINT iDevice = 0; iDevice < 20; iDevice++) {
//		if (pDevices[iDevice] != nullptr) {
//			pDevices[iDevice]->Release();
//		}
//	}
//
//	return bIsXinputDevice;
//}

} // namespace rhfw
