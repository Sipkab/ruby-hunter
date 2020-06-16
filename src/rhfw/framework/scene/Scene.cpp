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
 * Scene.cpp
 *
 *  Created on: 2014.09.15.
 *      Author: sipka
 */

#include <framework/scene/Scene.h>
#include <framework/scene/SceneManager.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/xml/XmlNode.h>

#include <gen/log.h>
#include <gen/xmldecl.h>
#include <gen/types.h>
#include <gen/resources.h>

using namespace rhfw;

LINK_XML_SIMPLE(Scene)

namespace rhfw {

Scene::Scene()
		: identifier(ResId::RES_INVALID) {
}
Scene::~Scene() {
	touchTarget.unlink();
}

Layer* Scene::touch() {
	TouchEvent& event = TouchEvent::instance;
	const TouchAction action = event.getAction();
	const unsigned int pointerCount = event.getPointerCount();

	Layer* target = touchTarget.get();

	if (target != nullptr) {
		Layer* ntarget = target->touch();
		if (ntarget != target) {
			target = ntarget;
			touchTarget.unlink();
			if (ntarget != nullptr) {
				touchTarget.link(ntarget);
			}
		}
	} else {
		//touch target == 0, and its first down now
		Layer* ntarget = LayerGroup::touch();
		if (ntarget != nullptr) {
			touchTarget.link(ntarget);
		}
	}
	if ((action == TouchAction::UP || action == TouchAction::CANCEL) && pointerCount == 0) {
		touchTarget.unlink();
	}
	return target;
}

void Scene::show() {
	getWindow()->drawListeners += *this;
	getWindow()->sizeListeners += *this;
	getWindow()->touchListeners += *this;
}

void Scene::hide() {
	DrawListener::unsubscribe();
	WindowSizeListener::unsubscribe();
	TouchEventListener::unsubscribe();
}
core::Window* rhfw::Scene::getWindow() {
	return manager->getWindow();
}

}

