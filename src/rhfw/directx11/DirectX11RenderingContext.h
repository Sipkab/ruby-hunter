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
 * DirectX11RenderingContext.h
 *
 *  Created on: 2016. szept. 10.
 *      Author: sipka
 */

#ifndef DIRECTX11_DIRECTX11RENDERINGCONTEXT_H_
#define DIRECTX11_DIRECTX11RENDERINGCONTEXT_H_

#include <framework/render/RenderingContext.h>
#include <d3d11_2.h>
#include <gen/configuration.h>

#if RHFW_DEBUG
#if _WIN32_WINNT >= 0x0602
//Do not query IsWindows8OrGreater, when building for that
#define USE_DX11_DEBUG_CREATION_FLAG() true
#else
#include <VersionHelpers.h>
#define USE_DX11_DEBUG_CREATION_FLAG() IsWindows8OrGreater()
#endif /* _WIN32_WINNT >= 0x0602 */
#endif /* RHFW_DEBUG */

namespace rhfw {
class DirectX11Renderer;
namespace render {

class DirectX11Functions {
public:
	typedef HRESULT (WINAPI *PROTO_D3D11CreateDevice)(
			IDXGIAdapter *pAdapter,
			D3D_DRIVER_TYPE DriverType,
			HMODULE Software,
			UINT Flags,
			const D3D_FEATURE_LEVEL *pFeatureLevels,
			UINT FeatureLevels,
			UINT SDKVersion,
			ID3D11Device **ppDevice,
			D3D_FEATURE_LEVEL *pFeatureLevel,
			ID3D11DeviceContext **ppImmediateContext
	);

	PROTO_D3D11CreateDevice dxfunc_D3D11CreateDevice = nullptr;
};

class DirectX11RenderingContext: public render::RenderingContext, public DirectX11Functions {
protected:
	ID3D11Device1* dev;
	ID3D11DeviceContext1* devcon;
public:
	DirectX11RenderingContext(const DirectX11Functions& functions, ID3D11Device1* dev, ID3D11DeviceContext1* devcon);
	~DirectX11RenderingContext();

	void destroyContext();

	ID3D11Device1* getDevice() const {
		return dev;
	}

	ID3D11DeviceContext1* getDeviceContext() const {
		return devcon;
	}
};
} // namespace render
} // namespace rhfw

#endif /* DIRECTX11_DIRECTX11RENDERINGCONTEXT_H_ */
