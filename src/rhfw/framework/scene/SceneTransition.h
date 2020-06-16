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
 * SceneTransition.h
 *
 *  Created on: 2015 m�rc. 22
 *      Author: sipka
 */

#ifndef SCENETRANSITION_H_
#define SCENETRANSITION_H_

#include <framework/core/Window.h>
#include <framework/utils/MemberLinkedNode.h>

#include <gen/types.h>
#include <gen/configuration.h>
#include <gen/resources.h>
namespace rhfw {

class Scene;
class ScreenSize;
class SceneManager;

class SceneTransition: public core::DrawListener, public core::WindowSizeListener {
protected:
	static void handleSceneChanges(SceneTransition* transition);
	static void handleSceneChanges(SceneTransition* transition, Scene* newscene);

	SceneManager& manager;

	SceneTransitionOptions options;

	Scene* oldScene;
	Scene* newScene;

	const ResId oldSceneId;
	const ResId newSceneId;

	void* data;

public:
	SceneTransition(SceneManager& manager, ResId oldSceneId, ResId newSceneId);
	SceneTransition(SceneManager& manager, ResId newSceneId);
	virtual ~SceneTransition();

	virtual void onDraw() override {
	}

	virtual void startSceneTransition();
	virtual void sizeChanged(const core::WindowSize& size) {
	}
	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override {
		sizeChanged(size);
	}

	void putData(void* data) {
		this->data = data;
	}
	void* takeData() {
		void* result = this->data;
		this->data = nullptr;
		return result;
	}

	Scene* getOldScene() const {
		return oldScene;
	}
	Scene* getNewScene() const {
		return newScene;
	}
	SceneTransitionOptions getOptions() const {
		return options;
	}
	ResId getNewSceneId() const {
		return newSceneId;
	}
	ResId getOldSceneId() const {
		return oldSceneId;
	}

	void setOldScene(Scene* scene) {
		oldScene = scene;
	}
	void setNewScene(Scene* scene) {
		newScene = scene;
	}
	void setOptions(SceneTransitionOptions options) {
		this->options = options;
	}

};
}

#endif /* SCENETRANSITION_H_ */
