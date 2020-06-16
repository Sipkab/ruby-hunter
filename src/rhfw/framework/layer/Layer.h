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
 * Layer.h
 *
 *  Created on: 2014.06.08.
 *      Author: sipka
 */

#ifndef VIEW_H_
#define VIEW_H_

#include <framework/core/Window.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/MemberLinkedNode.h>

#include <gen/types.h>
#include <gen/configuration.h>
namespace rhfw {

class TouchEvent;
class Layer;
class ResourceLoader;
class ScreenSize;
class Scene;

class Layer: public LinkedNode<Layer> {
protected:
	LayerOptions flags;

	Scene* scene = nullptr;

public:
	Layer();

	virtual ~Layer() {
	}

	virtual void setScene(Scene* scene) {
		this->scene = scene;
	}
	Scene* getScene() {
		return scene;
	}

	virtual void draw() {
	}

	virtual Layer* touch() {
		return nullptr;
	}

	virtual void sizeChanged(const core::WindowSize& size) {
	}

	Layer* get() override {
		return this;
	}

	LayerOptions getFlags() const {
		return flags;
	}

	virtual void loadResources(ResourceLoader& loader) {
	}

	virtual void onResourcesLoaded() {
	}

	void removeFromParent() {
		this->removeLinkFromList();
	}
};
}

#endif /* VIEW_H_ */
