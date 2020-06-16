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
 * Scene.h
 *
 *  Created on: 2014.06.21.
 *      Author: sipka
 */

#ifndef UI_H_
#define UI_H_

#include <framework/utils/LifeCycleChain.h>
#include <framework/layer/LayerGroup.h>
#include <framework/core/Window.h>

#include <gen/resources.h>
#include <gen/configuration.h>
namespace rhfw {

class Context;
class Layer;
class TouchEvent;
class XmlWriter;
class SceneTransition;
class SceneManager;

class Scene: public LayerGroup, public core::DrawListener, public core::TouchEventListener, public core::WindowSizeListener {
private:
	LifeCycleChain<Layer, false> touchTarget;

	ResId identifier;

	SceneManager* manager = nullptr;

public:

	Scene();
	virtual ~Scene();

	Layer* touch() override;

	core::Window* getWindow();
	virtual void setSceneManager(SceneManager* manager) {
		this->manager = manager;
	}
	SceneManager& getSceneManager() {
		ASSERT(manager != nullptr) << "SceneManager is nullptr";
		return *manager;
	}

	virtual bool onTouchEvent() override {
		touch();
		return true;
	}

	virtual void onDraw() override {
		LayerGroup::draw();
	}

	virtual void onSizeChanged(core::Window& window, const core::WindowSize& size) override {
		LayerGroup::sizeChanged(size);
	}

	//away from this scene
	virtual void navigatingTo(SceneTransition* transition) {
	}
	//to this scene
	virtual void navigatingFrom(SceneTransition* transition) {
	}

	ResId getIdentifier() const {
		return identifier;
	}
	void setIdentifier(ResId identifier) {
		this->identifier = identifier;
	}

	void show();
	void hide();

	void clearChildren() {
		touchTarget.unlink();
		LayerGroup::clearChildren();
	}

};
}

#endif /* UI_H_ */
