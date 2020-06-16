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
 * SceneManager.cpp
 *
 *  Created on: 2016. febr. 16.
 *      Author: sipka
 */

#include <framework/scene/SceneManager.h>

#include <framework/resource/ResourceManager.h>
#include <framework/scene/Scene.h>
#include <framework/scene/SceneTransition.h>
#include <framework/xml/XmlParser.h>

namespace rhfw {

SceneManager::SceneManager() {
}
SceneManager::~SceneManager() {
	destroy();
}

SceneManager::SceneStackEntry::SceneStackEntry(SceneTransition& transition)
		: flags(transition.getOptions()), oldScene(transition.getOldScene()), newScene(transition.getNewScene()), oldSceneId(
				transition.getOldSceneId()), newSceneId(transition.getNewSceneId()) {
}

Scene* SceneManager::getScene(ResId identifier) {
	//ASSERT(scene != nullptr, "No scene is set");
	//TODO implement
	return nullptr;
}

Scene* SceneManager::getTopScene() {
	return topscene;
}

void SceneManager::saveSceneState(Scene* scene) {
	//TODO save based on id
	//const ResId sceneid = scene->getIdentifier();
}

void SceneManager::startSceneTransition(SceneTransition* transition) {
	ASSERT(transition != nullptr) << "transition to start is nullptr";
	ASSERT(this->transition == nullptr) << "A transition is already going on";
	ASSERT(
			(topscene == nullptr && transition->getOldSceneId() == ResId::RES_INVALID)
			|| (topscene != nullptr && transition->getOldSceneId() == topscene->getIdentifier())) <<
	"Top scene(" << topscene->getIdentifier() << ") is not the starting scene(" << transition->getOldSceneId() << ") of the transition";

	this->transition = transition;
	transition->setOldScene(topscene);
	if (this->topscene != nullptr) {
		this->topscene->navigatingTo(transition);
	}
	this->topscene = nullptr;

	LOGI()<< "Start scene stransition from " << transition->getOldSceneId() << " -> " << transition->getNewSceneId();

	transition->startSceneTransition();
}

void SceneManager::finishSceneTransition(SceneTransition* transition) {
	ASSERT(transition == this->transition) << "Tried to finish transition not bound to current context";
	ASSERT(transition->getNewScene() != nullptr) << "Transition's new scene is nullptr";

	this->transition = nullptr;
	if (HAS_FLAG(transition->getOptions(), SceneTransitionOptions::CLEAR_HISTORY_STACK)) {
		sceneStack.clear();
	} else {
		sceneStack.addToEnd(*new SceneStackEntry(*transition));
	}

	this->topscene = transition->getNewScene();
	this->topscene->navigatingFrom(transition);
	this->topscene->show();

	transition->setOldScene(nullptr);
	transition->setNewScene(nullptr);

	LOGI()<< "End scene stransition from " << transition->getOldSceneId() << " -> " << transition->getNewSceneId();

	delete transition;
}
void SceneManager::create(core::Window* window, Scene* startscene) {
	this->window = window;
	topscene = startscene;
	topscene->setSceneManager(this);
	topscene->setScene(topscene);
	topscene->show();
}
Scene* SceneManager::create(core::Window* window, ResId startscene) {
	Scene* scene = XmlParser::parseXmlAsset(ResourceManager::idToFile(startscene));
	scene->setIdentifier(startscene);
	create(window, scene);
	return scene;
}

void SceneManager::destroy() {
	sceneStack.clear();

	if (topscene != nullptr) {
		delete topscene;
		topscene = nullptr;
	}
	if (transition != nullptr) {
		delete transition;
		transition = nullptr;
	}
}

}
