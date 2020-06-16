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
 * Texture.cpp
 *
 *  Created on: 2016. dec. 2.
 *      Author: sipka
 */

#include <framework/render/Texture.h>

#if RENDERAPI_directx11_AVAILABLE
#include <directx11/texture/DirectX11Texture.h>
void rhfw::render::TextureInputSource::apply(rhfw::DirectX11Texture* texture) {
	apply(static_cast<rhfw::render::Texture*>(texture));
}
#endif /* RENDERAPI_opengl30_AVAILABLE */

#if RENDERAPI_opengles20_AVAILABLE
#include <opengles20/texture/OpenGlEs20Texture.h>
void rhfw::render::TextureInputSource::apply(rhfw::OpenGlEs20Texture* texture) {
	apply(static_cast<rhfw::render::Texture*>(texture));
}
#endif /* RENDERAPI_opengles20_AVAILABLE */

#if RENDERAPI_opengl30_AVAILABLE
#include <opengl30/texture/OpenGl30Texture.h>
void rhfw::render::TextureInputSource::apply(rhfw::OpenGl30Texture* texture) {
	apply(static_cast<rhfw::render::Texture*>(texture));
}
#endif /* RENDERAPI_directx11_AVAILABLE */

