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
 * RenderTarget.h
 *
 *  Created on: 2016. marc. 9.
 *      Author: sipka
 */

#ifndef RENDER_RENDERTARGET_H_
#define RENDER_RENDERTARGET_H_

#include <framework/render/RenderObject.h>
#include <framework/resource/Resource.h>
#include <framework/render/ViewPort.h>
#include <framework/render/Texture.h>
#include <framework/render/RenderBuffer.h>

#include <gen/types.h>

namespace rhfw {
namespace render {

class Texture;
class RenderBuffer;

class RenderTargetDescriptor {
private:
	RenderTargetType colorType = RenderTargetType::UNUSED;
	RenderTargetType depthStencilType = RenderTargetType::UNUSED;

	Resource<RenderObject> colorTarget { };
	Resource<RenderObject> depthStencilTarget { };
public:
	void setColorTarget(RenderTargetType type) {
		this->colorType = type;
		this->colorTarget = Resource<RenderObject> { };
	}
	template<typename A>
	void setColorTarget(const ResourceBase<Texture, A>& target) {
		this->colorType = RenderTargetType::TEXTURE;
		this->colorTarget = target;
	}
	template<typename A>
	void setColorTarget(const ResourceBase<RenderBuffer, A>& target) {
		this->colorType = RenderTargetType::RENDERBUFFER;
		this->colorTarget = target;
	}

	void setDepthStencilTarget(RenderTargetType type) {
		this->depthStencilType = type;
		this->depthStencilTarget = Resource<RenderObject> { };
	}
	template<typename A>
	void setDepthStencilTarget(const ResourceBase<Texture, A>& target) {
		this->depthStencilType = RenderTargetType::TEXTURE;
		this->depthStencilTarget = target;
	}
	template<typename A>
	void setDepthStencilTarget(const ResourceBase<RenderBuffer, A>& target) {
		this->depthStencilType = RenderTargetType::RENDERBUFFER;
		this->depthStencilTarget = target;
	}

	RenderTargetType getColorType() const {
		return colorType;
	}
	Resource<RenderObject> getColorTarget() const {
		return colorTarget;
	}
	RenderTargetType getDepthStencilType() const {
		return depthStencilType;
	}
	Resource<RenderObject> getDepthStencilTarget() const {
		return depthStencilTarget;
	}
};

class RenderTarget: public RenderObject {
private:
	RenderTargetDescriptor descriptor;
protected:
public:
	void setDescriptor(const RenderTargetDescriptor& desc) {
		this->descriptor = desc;
	}
	const RenderTargetDescriptor& getDescriptor() const {
		return descriptor;
	}
	RenderTarget& operator=(RenderTarget&& o) {
		this->descriptor = o.descriptor;
		return *this;
	}

	virtual void bindForDrawing() = 0;
	virtual void finishDrawing() = 0;
	virtual ViewPort getDefaultViewPort() = 0;
};

} // namespace render
} // namespace rhfw

#endif /* RENDER_RENDERTARGET_H_ */
