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
 * DirectX11RenderingContext.cpp
 *
 *  Created on: 2016. szept. 16.
 *      Author: sipka
 */

#include <directx11/DirectX11RenderingContext.h>
#include <gen/platform.h>
#include WINDOWS_EXECPTION_HELPER_INCLUDE

namespace rhfw {
namespace render {

DirectX11RenderingContext::DirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev, ID3D11DeviceContext1* devcon)
		: DirectX11Functions(functions), dev(dev), devcon(devcon) {
	LOGTRACE()<<"Created DirectX11 rendering context";
}

DirectX11RenderingContext::~DirectX11RenderingContext() {
	ASSERT(dev == nullptr && devcon == nullptr) << "Context wasn't destroyed";
}

void DirectX11RenderingContext::destroyContext() {
#if RHFW_DEBUG
	if (USE_DX11_DEBUG_CREATION_FLAG()) {
		ID3D11Debug *debugDev;
		ThrowIfFailed(dev->QueryInterface(IID_PPV_ARGS(&debugDev)));
		debugDev->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		debugDev->Release();
	}
#endif /* RHFW_DEBUG */
	ULONG res;
	res = devcon->Release();
	ASSERT(res == 0);
	res = dev->Release();
	ASSERT(res == 0);

	LOGTRACE()<<"Destroyed DirectX11 rendering context";

	devcon = nullptr;
	dev = nullptr;
}

} // namespace render
} // namespace rhfw

