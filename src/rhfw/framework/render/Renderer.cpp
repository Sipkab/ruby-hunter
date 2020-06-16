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
 * Renderer.cpp
 *
 *  Created on: 2016. marc. 8.
 *      Author: sipka
 */

#include <framework/render/Renderer.h>
#include <framework/render/RenderingContext.h>
#include <framework/render/RenderTarget.h>
#include <framework/render/ShaderPipelineStage.h>

#include <gen/shader/shaders.h>
#include <gen/renderers.h>

namespace rhfw {
namespace render {

template<typename T>
class Renderer::ArrayTracker: public ResourceBlock {
private:
public:
	ArrayTracker<T>** selfInCache;

	ArrayTracker(T* resource, ArrayTracker<T>** selfInCache)
			: ResourceBlock { resource }, selfInCache(selfInCache) {
		*selfInCache = this;
	}
	~ArrayTracker() {
		*selfInCache = nullptr;
	}
};

Renderer::Renderer(RenderConfig shaderrenderertype, TextureLoadOrder textureloadorder, TextureUvOrientation textureuvorientation,
		DepthRange depthrange)
		: shaderRendererType { shaderrenderertype }, textureLoadOrder { textureloadorder }, textureUvOrientation { textureuvorientation }, depthRange {
				depthrange } {
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; ++i) {
		shaderPrograms[i] = nullptr;
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		shaderPipelineStages[i] = nullptr;
	}
}
Renderer::~Renderer() {
	ASSERT(renderContext == nullptr) << "Renderer was not unloaded";
}
Renderer& Renderer::operator=(Renderer&& o) {
	ASSERT(this->renderContext == nullptr) << "Renderer is already loaded";
	ASSERT(o.renderContext != nullptr) << "Other renderer is not loaded";

	depthTestEnabledState.setTargetValue(o.depthTestEnabledState.getTargetValue());
	faceCullingEnabledState.setTargetValue(o.faceCullingEnabledState.getTargetValue());
	cullFrontFaceState.setTargetValue(o.cullFrontFaceState.getTargetValue());

	viewPortState.setTargetValue(o.viewPortState.getTargetValue());

	renderTargetState.setValues(nullptr);
	if (o.renderTargetStackCount > 0) {
		renderTargetStackCount = o.renderTargetStackCount - 1;
		for (int i = 0; i < renderTargetStackCount; i++) {
			renderTargetStack[i] = o.renderTargetStack[i];
		}
		pushRenderTarget(o.renderTargetStack[renderTargetStackCount].target, o.renderTargetStack[renderTargetStackCount].type);
	} else {
		renderTargetStackCount = 0;
	}

	attachedWindows = util::move(o.attachedWindows);

	textureObjects = util::move(o.textureObjects);
	vertexBufferObjects = util::move(o.vertexBufferObjects);
	indexBufferObjects = util::move(o.indexBufferObjects);
	renderTargetObjects = util::move(o.renderTargetObjects);
	renderBufferObjects = util::move(o.renderBufferObjects);

	for (auto&& w : attachedWindows.objects()) {
		o.renderContext->detachWindow(&w);
	}
	for (auto* o : renderTargetObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderTarget>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : textureObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<Texture>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : renderBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : vertexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<VertexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : indexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<IndexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	//before programs, because freeing programs will free these
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = o.shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			resblock->addHandles(1);
			resblock->addReference(1);
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; i++) {
		auto& resblock = o.shaderPrograms[i];
		if (resblock != nullptr) {
			auto* nres = instantiateShader((UnifiedShaders) i, this);
			auto* oldres = static_cast<ShaderProgram*>(resblock->replace(nres));
			if (resblock->isLoaded()) {
				nres->moveResourcesFrom(util::move(*oldres));
				oldres->free();
			}
			this->shaderPrograms[i] = resblock;
			resblock = nullptr;
			delete oldres;
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = o.shaderPipelineStages[i];
		if (resblock != nullptr) {
			auto* nres = instantiatePipeline((UnifiedShaderPipelineStage) i, this);
			auto* oldres = static_cast<ShaderPipelineStage*>(resblock->replace(nres));
			if (resblock->isLoaded()) {
				nres->moveResourcesFrom(util::move(*oldres));
				oldres->free();
			}
			this->shaderPipelineStages[i] = resblock;
			resblock = nullptr;
			delete oldres;
		}
	}
	o.free();
	this->renderingContextOptions = util::move(o.renderingContextOptions);
	this->load();

	for (auto&& w : attachedWindows.objects()) {
		renderContext->attachWindow(&w);
	}
	for (auto&& o : vertexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<VertexBuffer>*>(o);
		auto* nres = createVertexBufferImpl();
		auto* oldres = static_cast<VertexBuffer*>(res->replace(nres));
		if (res->isLoaded()) {
			nres->load();
		}
		*nres = util::move(*oldres);
		delete oldres;
	}
	for (auto&& o : indexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<IndexBuffer>*>(o);
		auto* nres = createIndexBufferImpl();
		auto* oldres = static_cast<IndexBuffer*>(res->replace(nres));
		if (res->isLoaded()) {
			nres->load();
		}
		*nres = util::move(*oldres);
		delete oldres;
	}
	for (auto* o : textureObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<Texture>*>(o);
		auto* nres = createTextureImpl();
		auto* oldres = static_cast<Texture*>(res->replace(nres));
		*nres = util::move(*oldres);
		if (res->isLoaded()) {
			nres->load();
		}
		delete oldres;
	}
	for (auto&& o : renderBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderBuffer>*>(o);
		auto* nres = createRenderBufferImpl();
		auto* oldres = static_cast<RenderBuffer*>(res->replace(nres));
		*nres = util::move(*oldres);
		if (res->isLoaded()) {
			nres->load();
		}
		delete oldres;
	}
	//render target must be after renderbuffer and texture, because of the dependencies
	for (auto&& o : renderTargetObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderTarget>*>(o);
		auto* nres = createRenderTargetImpl();
		auto* oldres = static_cast<RenderTarget*>(res->replace(nres));
		*nres = util::move(*oldres);
		if (res->isLoaded()) {
			nres->load();
		}
		delete oldres;
	}

	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = this->shaderPipelineStages[i];
		if (resblock != nullptr) {
			auto* res = static_cast<ShaderPipelineStage*>(resblock->get());
			if (resblock->isLoaded()) {
				res->load();
				res->updateMovedResources();
			}
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; i++) {
		auto& resblock = this->shaderPrograms[i];
		if (resblock != nullptr) {
			resblock->selfInCache = this->shaderPrograms + i;
			auto* res = static_cast<ShaderProgram*>(resblock->get());
			if (resblock->isLoaded()) {
				res->load();
				res->updateMovedResources();
			}
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = this->shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			resblock->selfInCache = this->shaderPipelineStages + i;
			resblock->addHandles(-1);
			resblock->addReference(-1);
		}
	}

	setTopology(o.topology);

	return *this;
}

bool Renderer::load() {
	renderContext = instantiateRenderingContext(this, &renderingContextOptions);
	if (renderContext == nullptr) {
		return false;
	}

	depthTestEnabledState.postCommand();
	faceCullingEnabledState.postCommand();
	cullFrontFaceState.postCommand();

	return true;
}
void Renderer::free() {
	ASSERT(attachedWindows.isEmpty()) << "Some windows are not detached from renderer";
	ASSERT(textureObjects.isEmpty()) << "Objects are still alive";
	ASSERT(vertexBufferObjects.isEmpty()) << "Objects are still alive";
	ASSERT(indexBufferObjects.isEmpty()) << "Objects are still alive";
	ASSERT(renderTargetObjects.isEmpty()) << "Objects are still alive";
	ASSERT(renderBufferObjects.isEmpty()) << "Objects are still alive";

#if LOGGING_ENABLED
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; i++) {
		auto& resblock = this->shaderPrograms[i];
		ASSERT(resblock == nullptr) << "Shaderprogram: " << (UnifiedShaders) i << " not destroyed handles: " << resblock->getHandleCount()
				<< " loadcount: " << resblock->getReferenceCount();
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = this->shaderPipelineStages[i];
		ASSERT(resblock == nullptr) << "Shader stage: " << (UnifiedShaderPipelineStage) i << " not destroyed handles: "
				<< resblock->getHandleCount() << " loadcount: " << resblock->getReferenceCount();
	}
#endif

	delete renderContext;
	renderContext = nullptr;
}
bool Renderer::reload() {
	depthTestEnabledState.postCommand();
	faceCullingEnabledState.postCommand();
	cullFrontFaceState.postCommand();

	viewPortState.postCommand();

	for (auto&& w : attachedWindows.objects()) {
		renderContext->detachWindow(&w);
	}
	for (auto* o : renderTargetObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderTarget>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : textureObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<Texture>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : renderBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : vertexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<VertexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (auto* o : indexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<IndexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->free();
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			resblock->addHandles(1);
			resblock->addReference(1);
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			static_cast<ShaderPipelineStage*>(resblock->get())->free();
			static_cast<ShaderPipelineStage*>(resblock->get())->freeInContext();
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; i++) {
		auto& resblock = shaderPrograms[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			static_cast<ShaderProgram*>(resblock->get())->free();
			static_cast<ShaderProgram*>(resblock->get())->freeInContext();
		}
	}

	renderContext->reload();

	for (auto* o : indexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<IndexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->reloadInNewContext();
		}
	}
	for (auto* o : vertexBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<VertexBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->reloadInNewContext();
		}
	}
	for (auto* o : renderBufferObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderBuffer>*>(o);
		if (res->isLoaded()) {
			res->get()->load();
		}
	}
	for (auto* o : textureObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<Texture>*>(o);
		if (res->isLoaded()) {
			res->get()->load();
		}
	}
	for (auto* o : renderTargetObjects.nodes()) {
		auto* res = static_cast<TrackingResourceBlock<RenderTarget>*>(o);
		if (res->isLoaded()) {
			res->get()->load();
		}
	}

	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			static_cast<ShaderPipelineStage*>(resblock->get())->load();
			static_cast<ShaderPipelineStage*>(resblock->get())->updateMovedResources();
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaders::_count_of_entries; i++) {
		auto& resblock = shaderPrograms[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			static_cast<ShaderProgram*>(resblock->get())->load();
			//TODO create new function instead of updateMovedResources, since it allocates new instances
			static_cast<ShaderProgram*>(resblock->get())->updateMovedResources();
		}
	}
	for (unsigned int i = 0; i < (unsigned int) UnifiedShaderPipelineStage::_count_of_entries; i++) {
		auto& resblock = this->shaderPipelineStages[i];
		if (resblock != nullptr && resblock->isLoaded()) {
			resblock->addHandles(-1);
			resblock->addReference(-1);
		}
	}

	for (auto&& w : attachedWindows.objects()) {
		renderContext->attachWindow(&w);
	}

	setTopology(topology);

	return true;
}

void Renderer::attachWindow(core::Window* window) {
	ASSERT(window != nullptr) << "Window is nullptr";
	ASSERT(renderContext != nullptr) << "Renderer was not loaded";

	renderContext->attachWindow(window);
	attachedWindows.addToEnd(window->getRenderSurface());
}

void Renderer::detachWindow(core::Window* window) {
	ASSERT(window != nullptr) << "Window is nullptr";
	ASSERT(renderContext != nullptr) << "Renderer was not loaded";

	renderContext->detachWindow(window);
	window->getRenderSurface().removeLinkFromList();
}

void Renderer::pushRenderTarget(const Resource<RenderTarget>& target, int type) {
	ASSERT(target != nullptr) << "target is nullptr";
	ASSERT(renderTargetStackCount < MAX_RENDER_TARGETS) << "render target count reached";

	renderTargetStack[renderTargetStackCount].target = target;
	renderTargetStack[renderTargetStackCount].type = type;
	renderTargetState.setTargetValue(renderTargetStack + renderTargetStackCount);

	++renderTargetStackCount;
}

void Renderer::pushRenderTarget(Resource<RenderTarget> target) {
	pushRenderTarget(target, RENDER_TARGET_TYPE_GENERIC);
}
void Renderer::pushRenderTarget(core::Window* window) {
	pushRenderTarget(window->getRenderSurface().getRenderTarget(), RENDER_TARGET_TYPE_WINDOW);
}

void Renderer::popRenderTarget() {
	ASSERT(renderTargetStackCount > 0) << "No rendertargets to pop";
	--renderTargetStackCount;
	Resource<RenderTarget>& top = renderTargetStack[renderTargetStackCount].target;
	top->finishDrawing();
	top = nullptr;
	renderTargetState.setValues(nullptr);
	if (renderTargetStackCount != 0) {
		renderTargetState.setTargetValue(renderTargetStack + renderTargetStackCount - 1);
	}
}

Resource<ShaderProgram> Renderer::getShaderProgram(UnifiedShaders shader) {
	auto* tracker = shaderPrograms + (unsigned int) shader;
	if (*tracker == nullptr) {
		*tracker = new ArrayTracker<ShaderProgram> { instantiateShader(shader, this), tracker };
	}
	return Resource<ShaderProgram> { *tracker };
}
Resource<ShaderPipelineStage> Renderer::getShaderPipelineStage(UnifiedShaderPipelineStage pipeline) {
	auto* tracker = shaderPipelineStages + (unsigned int) pipeline;
	if (*tracker == nullptr) {
		*tracker = new ArrayTracker<ShaderPipelineStage> { instantiatePipeline(pipeline, this), tracker };
	}
	return Resource<ShaderPipelineStage> { *tracker };
}

}  // namespace render
}  // namespace rhfw

