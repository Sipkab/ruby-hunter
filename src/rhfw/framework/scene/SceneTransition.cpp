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
 * SceneTransition.cpp
 *
 *  Created on: 2015 m·rc. 22
 *      Author: sipka
 */

#include <framework/scene/SceneTransition.h>
#include <framework/scene/Scene.h>

#include <framework/resource/ResourceManager.h>
#include <framework/xml/XmlParser.h>
#include <framework/io/files/FileDescriptor.h>
#include <framework/io/files/FileInput.h>
#include <framework/resource/ResourceLoader.h>
#include <framework/scene/SceneManager.h>

#include <gen/log.h>
#include <gen/types.h>
#include <gen/resources.h>
namespace rhfw {

void SceneTransition::handleSceneChanges(SceneTransition* transition, Scene* newScene) {
	newScene->setSceneManager(&transition->manager);
	newScene->setScene(newScene);

	ResourceLoader resLoader;
	newScene->loadResources(resLoader);

	if (transition->oldScene != nullptr) {
		//handle old changes
		switch (transition->options & SceneTransitionOptions::MASK_OLD) {
			case SceneTransitionOptions::KEEP_OLD: {
				//do nothing
				THROW()<< "not supported yet";
				break;
			}
			case SceneTransitionOptions::SAVE_OLD: {
				//TODO save
				THROW() << "not supported yet";
				break;
			}
			case SceneTransitionOptions::DELETE_OLD: {
				//TODO simply delete
				delete transition->oldScene;
				transition->oldScene = nullptr;
				break;
			}
			default: {
				THROW() << "Specify at least one old flag";
				break;
			}
		}
	}

	core::WindowSize size = transition->manager.getWindow()->getWindowSize();

	newScene->sizeChanged(size);
}

void SceneTransition::handleSceneChanges(SceneTransition* transition) {
	LOGI()<< "Handling scene changes with flags: " << transition->options;

	if (transition->newScene == nullptr) {
		//create new
		switch (transition->options & SceneTransitionOptions::MASK_NEW) {
			case SceneTransitionOptions::LOAD_NEW: {
				//TODO load saved from file
				THROW() <<"not supported yet";
				break;
			}
			case SceneTransitionOptions::TRYLOAD_EXCCREATE_NEW: {
				//TODO load from file if exist else create
				THROW() << "not supported yet";
				break;
			}
			case SceneTransitionOptions::ALLOCATE_NEW: {
				//TODO create new
				RAssetFile fileid = ResourceManager::idToFile(transition->newSceneId);
				transition->newScene = static_cast<Scene*>(XmlParser::parseXmlAsset(fileid));
				transition->newScene->setIdentifier(transition->newSceneId);
				break;
			}
			default: {
				THROW() << "Specify at least one new flag";
				break;
			}
		}
		ASSERT(transition->newScene != nullptr) << "Did not set new scene";
	}
	handleSceneChanges(transition, transition->newScene);

	LOGI() << "Handled scene changes";
}

SceneTransition::SceneTransition(SceneManager& manager, ResId oldSceneId, ResId newSceneId)
		: manager(manager), options(SceneTransitionOptions::NO_FLAG), oldScene(nullptr), newScene(nullptr), oldSceneId(oldSceneId), newSceneId(
				newSceneId), data(nullptr) {
}
SceneTransition::SceneTransition(SceneManager& manager, ResId newSceneId)
		: manager(manager), options(SceneTransitionOptions::NO_FLAG), oldScene(nullptr), newScene(nullptr), oldSceneId(
				manager.getTopScene()->getIdentifier()), newSceneId(newSceneId), data(nullptr) {
}
SceneTransition::~SceneTransition() {
	ASSERT(data == nullptr) << "Data wasn't taken from scenetransaction";
	delete oldScene;
	delete newScene;
}

void SceneTransition::startSceneTransition() {
	manager.getWindow()->drawListeners += *this;
	manager.getWindow()->sizeListeners += *this;
	if (oldScene != nullptr) {
		oldScene->hide();
	}
}
}
