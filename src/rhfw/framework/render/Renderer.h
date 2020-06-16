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
 * Renderer.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RENDER_RENDERER_H_
#define RENDER_RENDERER_H_

#include <framework/resource/Resource.h>
#include <framework/resource/TrackingResourceBlock.h>
#include <framework/geometry/Vector.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/ConditionalArray.h>

#include <framework/render/RenderingContext.h>
#include <framework/render/IndexBuffer.h>
#include <framework/render/ShaderProgram.h>
#include <framework/render/VertexBuffer.h>
#include <framework/render/Texture.h>
#include <framework/render/RenderTarget.h>
#include <framework/render/ViewPort.h>
#include <framework/core/Window.h>
#include <framework/resource/ShareableResource.h>
#include <framework/utils/utility.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/log.h>

namespace rhfw {
namespace render {

#define RENDERER_CHECK_IS_DRAW_INITED() ASSERT(isDrawInited()) <<  "Call initDraw() before calling any render functions."

class Texture;
class ShaderProgram;
class ShaderPipelineStage;
class VertexBuffer;
class IndexBuffer;
class Renderer;
class RenderingContext;
class RenderTarget;
class ViewPort;

class Renderer: public ShareableResource {
	//for attachWindow and detachWindow
	friend core::Window;
protected:
	static const int RENDER_TARGET_TYPE_WINDOW = 0;
	static const int RENDER_TARGET_TYPE_GENERIC = 1;

	class RenderTargetStackEntry {
	public:
		Resource<RenderTarget> target;
		int type;
	};
private:

	LinkedList<core::Window, false> attachedWindows;

	LinkedList<Texture, false> textureObjects;
	LinkedList<VertexBuffer, false> vertexBufferObjects;
	LinkedList<IndexBuffer, false> indexBufferObjects;
	LinkedList<RenderTarget, false> renderTargetObjects;
	LinkedList<RenderBuffer, false> renderBufferObjects;

	template<typename T>
	class ArrayTracker;

	ConditionalArray<ArrayTracker<ShaderProgram>*, (unsigned int) UnifiedShaders::_count_of_entries> shaderPrograms;
	ConditionalArray<ArrayTracker<ShaderPipelineStage>*, (unsigned int) UnifiedShaderPipelineStage::_count_of_entries> shaderPipelineStages;

	RenderingContext* renderContext = nullptr;

	Topology topology = Topology::TRIANGLES;

	static const int MAX_RENDER_TARGETS = 32;

	RenderTargetStackEntry renderTargetStack[MAX_RENDER_TARGETS];
	int renderTargetStackCount = 0;

	const RenderConfig shaderRendererType;
	const TextureLoadOrder textureLoadOrder;
	const TextureUvOrientation textureUvOrientation;
	const DepthRange depthRange;

	RenderingContextOptions renderingContextOptions;

	class DrawStateCommand: public LinkedNode<DrawStateCommand> {
	private:
		void (*func)(Renderer*, void*, void*);
		Renderer* renderer;
		void* current;
		void* target;
	public:
		DrawStateCommand(void (*func)(Renderer*, void*, void*), Renderer* renderer, void* current, void* target)
				: func { func }, renderer { renderer }, current { current }, target { target } {
		}
		DrawStateCommand* get() override {
			return this;
		}

		void operator()() const {
			func(renderer, current, target);
		}
	};
	template<typename ValueType, void (Renderer::*RemoteCommand)(ValueType)>
	class DrawState {
	private:
		typedef typename util::remove_const<typename util::remove_reference<ValueType>::type>::type exact_type;
		//MSVC randomly throws INTERNAL COMPILER ERROR if we introduce an interface for draw state container with virtual method
		//instead we use a static callback function, with 1 Renderer parameter, and a current and target value pointers

		Renderer* renderer;

		exact_type currentValue;
		exact_type targetValue;

		static void executestate(Renderer* renderer, void* current, void* target) {
			exact_type* c = reinterpret_cast<exact_type*>(current);
			exact_type* t = reinterpret_cast<exact_type*>(target);

			*c = *t;
			(renderer->*RemoteCommand)(*t);
		}

		DrawStateCommand command;

	public:
		DrawState(Renderer* renderer, const ValueType& currentValue, const ValueType& targetValue)
				: renderer { renderer }, currentValue { currentValue }, targetValue { targetValue }, command { executestate, renderer,
						&this->currentValue, &this->targetValue } {
		}

		void setValues(const ValueType& value) {
			currentValue = value;
			targetValue = value;
			command.removeLinkFromList();
		}

		void setCurrentValue(const ValueType& value) {
			if (currentValue != value) {
				currentValue = value;
				if (targetValue == value) {
					command.removeLinkFromList();
				} else {
					postCommand();
				}
			}
		}
		void setTargetValue(const ValueType& value) {
			if (targetValue != value) {
				targetValue = value;
				if (currentValue == value) {
					command.removeLinkFromList();
				} else {
					postCommand();
				}
			}
		}

		const ValueType& getTargetValue() const {
			return targetValue;
		}
		const ValueType& getCurrentValue() const {
			return currentValue;
		}

		void postCommand() {
			renderer->addStateChangeCommand(command);
		}
	};

	LinkedList<DrawStateCommand, false> stateChangeCommands;

	virtual Texture* createTextureImpl() = 0;
	virtual VertexBuffer* createVertexBufferImpl() = 0;
	virtual IndexBuffer* createIndexBufferImpl() = 0;
	virtual RenderTarget* createRenderTargetImpl() = 0;
	virtual RenderBuffer* createRenderBufferImpl() = 0;

	virtual void setDepthTestImpl(bool enabled) = 0;
	virtual void setFaceCullingImpl(bool enabled) = 0;
	virtual void setCullToFrontFaceImpl(bool isFront) = 0;
	virtual void setViewPortImpl(const ViewPort& vp) = 0;

	void addStateChangeCommand(DrawStateCommand& command) {
		if (!command.isInList()) {
			stateChangeCommands.addToEnd(command);
		}
	}

	void attachWindow(core::Window* window);
	void detachWindow(core::Window* window);

	void pushRenderTarget(const Resource<RenderTarget>& target, int type);

protected:
	virtual void activateRenderTarget(RenderTargetStackEntry* target) {
		target->target->bindForDrawing();
	}

protected:

	DrawState<bool, &Renderer::setDepthTestImpl> depthTestEnabledState { this, false, false };
	DrawState<bool, &Renderer::setFaceCullingImpl> faceCullingEnabledState { this, false, false };
	DrawState<bool, &Renderer::setCullToFrontFaceImpl> cullFrontFaceState { this, false, false };

	DrawState<RenderTargetStackEntry*, &Renderer::activateRenderTarget> renderTargetState { this, nullptr, nullptr };

	DrawState<const ViewPort&, &Renderer::setViewPortImpl> viewPortState { this, ViewPort { 0, 0, 0, 0 }, ViewPort { 0, 0, 0, 0 } };
protected:

	const RenderTargetStackEntry* getRenderTarget() const {
		return renderTargetState.getTargetValue();
	}

	virtual bool load() override;
	virtual void free() override;
	virtual bool reload() override;
public:
	Renderer(RenderConfig shaderrenderertype, TextureLoadOrder textureLoadOrder, TextureUvOrientation textureUvOrientation,
			DepthRange depthRange);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&& o);
	~Renderer();

	Resource<Texture> createTexture() {
		return Resource<Texture> { new TrackingResourceBlock<Texture> { createTextureImpl(), textureObjects } };
	}
	Resource<VertexBuffer> createVertexBuffer() {
		return Resource<VertexBuffer> { new TrackingResourceBlock<VertexBuffer> { createVertexBufferImpl(), vertexBufferObjects } };
	}
	Resource<IndexBuffer> createIndexBuffer() {
		return Resource<IndexBuffer> { new TrackingResourceBlock<IndexBuffer> { createIndexBufferImpl(), indexBufferObjects } };
	}
	Resource<RenderTarget> createRenderTarget() {
		return Resource<RenderTarget> { new TrackingResourceBlock<RenderTarget> { createRenderTargetImpl(), renderTargetObjects } };
	}
	Resource<RenderBuffer> createRenderBuffer() {
		return Resource<RenderBuffer> { new TrackingResourceBlock<RenderBuffer> { createRenderBufferImpl(), renderBufferObjects } };
	}
	Resource<ShaderProgram> getShaderProgram(UnifiedShaders shader);
	Resource<ShaderPipelineStage> getShaderPipelineStage(UnifiedShaderPipelineStage pipeline);

	virtual void clearColor(const Color& color) = 0;
	virtual void clearDepthBuffer() = 0;
	virtual void clearColorDepthBuffer(const Color& color) = 0;

	void setDepthTest(bool enabled) {
		depthTestEnabledState.setTargetValue(enabled);
	}
	bool isDepthTest() const {
		return depthTestEnabledState.getTargetValue();
	}

	void setFaceCulling(bool enabled) {
		faceCullingEnabledState.setTargetValue(enabled);
	}
	bool isFaceCulling() const {
		return faceCullingEnabledState.getTargetValue();
	}

	void setCullFrontFace(bool cullFrontFace) {
		cullFrontFaceState.setTargetValue(cullFrontFace);
	}
	bool isCullFrontFace() const {
		return cullFrontFaceState.getTargetValue();
	}

	virtual void setTopology(Topology topology) {
		this->topology = topology;
	}
	Topology getTopology() {
		return topology;
	}

	void setViewPort(const ViewPort& vp) {
		viewPortState.setTargetValue(vp);
	}
	void setViewPort(RenderTarget* target) {
		viewPortState.setTargetValue(target->getDefaultViewPort());
	}
	void setViewPort(core::Window* window) {
		setViewPort(window->getRenderSurface().getRenderTarget());
	}
	render::ViewPort getViewPort() const {
		return viewPortState.getTargetValue();
	}
	void resetViewPort() {
		setViewPort(renderTargetState.getTargetValue()->target);
	}

	void initDraw() {
		stateChangeCommands.clear([](LinkedNode<DrawStateCommand>* node) {
			static_cast<DrawStateCommand&>(*node)();
		});
	}
	bool isDrawInited() const {
		return stateChangeCommands.isEmpty();
	}

	void pushRenderTarget(Resource<RenderTarget> target);
	void pushRenderTarget(core::Window* window);
	void popRenderTarget();

	RenderConfig getRendererType() const {
		return shaderRendererType;
	}
	TextureLoadOrder getTextureLoadOrder() const {
		return textureLoadOrder;
	}
	TextureUvOrientation getTextureUvOrientation() const {
		return textureUvOrientation;
	}
	DepthRange getDepthRange() const {
		return depthRange;
	}

	RenderingContext* getRenderingContext() {
		return renderContext;
	}

	const RenderingContextOptions& getRenderingContextOptions() const {
		return renderingContextOptions;
	}
	RenderingContextOptionsResult setRenderingContextOptions(const RenderingContextOptions& options) {
		this->renderingContextOptions = options;
		if (renderContext != nullptr) {
			return renderContext->setRenderingContextOptions(options);
		}
		return RenderingContextOptionsResult::NO_FLAG;
	}

	LinkedList<core::Window, false>& getAttachedWindows() {
		//TODO should move this to renderingcontext, and this should not be public API?
		return attachedWindows;
	}
};

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_RENDERER_H_ */
