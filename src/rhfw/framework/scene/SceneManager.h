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
 * SceneManager.h
 *
 *  Created on: 2016. febr. 16.
 *      Author: sipka
 */

#ifndef SCENEMANAGER_H_
#define SCENEMANAGER_H_

#include <framework/core/Window.h>
#include <framework/utils/LinkedList.h>

#include <gen/configuration.h>
#include <gen/types.h>
#include <gen/resources.h>
namespace rhfw {

class Scene;
class SceneTransition;

class SceneManager {
private:
	class SceneStackEntry: public LinkedNode<SceneStackEntry> {
	public:
		SceneTransitionOptions flags;

		Scene* oldScene;
		Scene* newScene;

		ResId oldSceneId;
		ResId newSceneId;

		SceneStackEntry(SceneTransition& transition);

		SceneStackEntry* get() {
			return this;
		}
	};

	LinkedList<SceneStackEntry> sceneStack;
	Scene* topscene = nullptr;
	SceneTransition* transition = nullptr;

	core::Window* window = nullptr;
public:
	SceneManager();
	~SceneManager();

	Scene* getScene(ResId identifier);
	Scene* getTopScene();

	Scene* create(core::Window* window, ResId startscene);
	void create(core::Window* window, Scene* startscene);
	void destroy();

	void saveSceneState(Scene* scene);
	void startSceneTransition(SceneTransition* transition);
	void finishSceneTransition(SceneTransition* transition);

	core::Window* getWindow() {
		return window;
	}
};

}
#endif /* SCENEMANAGER_H_ */
