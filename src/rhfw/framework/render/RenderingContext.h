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
 * RenderingContext.h
 *
 *  Created on: 2016. marc. 9.
 *      Author: sipka
 */

#ifndef RENDER_RENDERINGCONTEXT_H_
#define RENDER_RENDERINGCONTEXT_H_

#include <framework/core/Window.h>
#include <gen/fwd/types.h>
#include <gen/types.h>

namespace rhfw {
namespace render {

class RenderingContextOptions {
public:
	VSyncOptions vsyncOptions = VSyncOptions::NO_FLAG;
	unsigned int multiSamplingFactor = 0;

	void reduceMultiSamplingFactor() {
		if (multiSamplingFactor > 0) {
			multiSamplingFactor /= 2;
			if (multiSamplingFactor == 1) {
				multiSamplingFactor = 0;
			}
		}
	}
};

class RenderingContext {
private:
	friend class Renderer;
protected:
	RenderingContextOptions contextOptions;

	virtual bool reload() {
		return true;
	}
public:
	virtual void attachWindow(core::Window* window) = 0;
	virtual void detachWindow(core::Window* window) = 0;

	virtual bool supportsExclusiveFullScreen() {
		return false;
	}
	virtual bool setExclusiveFullScreen(core::Window* window, bool fullscreen) {
		return false;
	}
	virtual bool isExclusiveFullScreen(core::Window* window) {
		return false;
	}

	virtual VSyncOptions getSupportedVSyncOptions() {
		return VSyncOptions::NO_FLAG;
	}
	VSyncOptions getCurrentVSyncOptions() {
		return contextOptions.vsyncOptions;
	}
	virtual RenderingContextOptionsResult setVSyncOptions(VSyncOptions options) {
		return RenderingContextOptionsResult::FAILED;
	}

	virtual unsigned int getMaximumMultiSampleFactor() {
		return 0;
	}

	const RenderingContextOptions& getRenderingContextOptions() const {
		return contextOptions;
	}
	virtual RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) {
		return RenderingContextOptionsResult::FAILED;
	}

	virtual ~RenderingContext() = default;
};

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_RENDERINGCONTEXT_H_ */
