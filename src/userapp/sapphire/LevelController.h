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
 * LevelController.h
 *
 *  Created on: 2016. dec. 17.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVELCONTROLLER_H_
#define TEST_SAPPHIRE_LEVELCONTROLLER_H_

#include <framework/core/Window.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/animation/Animation.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/io/touch/TouchEvent.h>
#include <appmain.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireScene.h>

#include <gen/resources.h>
#include <gen/types.h>

namespace userapp {
using namespace rhfw;

class Level;
class SapphireScene;

class LevelController: private SapphireScene::KeyMapChangedListener, private SapphireScene::SettinsChangedListener {
	struct ControlCommand: public LinkedNode<ControlCommand> {
		SapphireDirection dir;
		SapphireControl control;

		ControlCommand(SapphireDirection dir, SapphireControl control)
				: dir(dir), control(control) {
		}

		ControlCommand* get() override {
			return this;
		}
	};
	class PlayerControlTracker {
	public:
		bool bombKeyDown = false;
		bool pickKeyDown = false;
		bool hadBomb = false;

		bool touchDirectionsDown[4] { false, false, false, false };
		bool keysDirectionsDown[4] { false, false, false, false };

		TouchPointer* pickTouch = nullptr;
		TouchPointer* placeBombTouch = nullptr;

		LinkedList<ControlCommand> commands;

		SapphireDirection getDirection() const {
			if (isDirDown(SapphireDirection::Left) == isDirDown(SapphireDirection::Right)) {
				if (isDirDown(SapphireDirection::Up) == isDirDown(SapphireDirection::Down)) {
					return SapphireDirection::Undefined;
				}
				return isDirDown(SapphireDirection::Up) ? SapphireDirection::Up :
						isDirDown(SapphireDirection::Down) ? SapphireDirection::Down : SapphireDirection::Undefined;
			}
			return isDirDown(SapphireDirection::Left) ? SapphireDirection::Left :
					isDirDown(SapphireDirection::Right) ? SapphireDirection::Right : SapphireDirection::Undefined;
		}
		SapphireControl getControl() {
			if (pickKeyDown || pickTouch != nullptr) {
				return SapphireControl::Take;
			} else if (bombKeyDown || placeBombTouch != nullptr || hadBomb) {
				hadBomb = false;
				return SapphireControl::PutBomb;
			} else {
				return SapphireControl::Move;
			}
		}

		void dirDown(SapphireDirection dir) {
			ASSERT((unsigned int ) dir < 4);
			touchDirectionsDown[(unsigned int) dir] = true;
			commands.addToEnd(*new ControlCommand { dir, getControl() });
		}
		void dirUp(SapphireDirection dir) {
			ASSERT((unsigned int ) dir < 4);
			touchDirectionsDown[(unsigned int) dir] = false;
		}
		void keyDown(SapphireDirection dir) {
			ASSERT((unsigned int ) dir < 4);
			keysDirectionsDown[(unsigned int) dir] = true;
			commands.addToEnd(*new ControlCommand { dir, getControl() });
		}
		void keyUp(SapphireDirection dir) {
			ASSERT((unsigned int ) dir < 4);
			keysDirectionsDown[(unsigned int) dir] = false;
		}
		bool isDirDown(SapphireDirection dir) const {
			ASSERT((unsigned int ) dir < 4);
			return touchDirectionsDown[(unsigned int) dir] || keysDirectionsDown[(unsigned int) dir];
		}

		template<SapphireControl Control>
		void controlDown();
		template<SapphireControl Control>
		void controlUp();

		void keyDirectionAction(SapphireDirection dir, KeyAction action);
		void keyDirectionAction(SapphireDirection dir, bool down);

		void bombKeyEvent(bool down);
		void pickKeyEvent(bool down);

		bool isAnyTouchDown() const {
			return pickTouch != nullptr || placeBombTouch != nullptr || touchDirectionsDown[0] || touchDirectionsDown[1]
					|| touchDirectionsDown[2] || touchDirectionsDown[3];
		}
	};
	class ControlsAnimator: public Animation {
	protected:
		virtual void onProgress(const core::time_millis& progress) override {
			const float percent = (float) ((progress - starttime) / duration);
			prop.left = prop.right = pstart + pdiff * (percent * percent);
			alpha = 1.0f - percent;
		}
		virtual void onFinish() override {
			Animation::onFinish();
			prop.left = prop.right = ptarget;
			alpha = 0.0f;
		}

		Rectangle& prop;
		float& alpha;
		float ptarget;
		float pstart = 0.0f;
		float pdiff = 0.0f;

		void onStart() override {
			pstart = prop.left;
			pdiff = ptarget - pstart;
		}
	public:
		ControlsAnimator(Rectangle& prop, float& alpha, float target, core::time_millis start, core::time_millis duration)
				: Animation(start, duration), prop(prop), alpha(alpha), ptarget(target) {
		}
	};

	class ControllerKeyActions {
	public:
		static const unsigned int KEY_COUNT = (unsigned int) KeyCode::_count_of_entries;
		typedef bool (*KeyActionFunction)(LevelController* lc, bool down, int playerid);
		KeyActionFunction keyActions[KEY_COUNT] { nullptr };
		void setKeyAction(KeyCode kc, KeyActionFunction func);

		KeyActionFunction operator[](KeyCode kc) const {
			unsigned int idx = (unsigned int) kc;
			if (idx >= KEY_COUNT) {
				return nullptr;
			}
			return keyActions[idx];
		}
		void clear() {
			for (int i = 0; i < (unsigned int) KeyCode::_count_of_entries; ++i) {
				keyActions[i] = nullptr;
			}
		}
	};
	ControllerKeyActions keyboardActions;
	ControllerKeyActions gamepadActions[SAPPHIRE_MAX_PLAYER_COUNT];

	AutoResource<render::Texture> menuTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_menu_white);
	AutoResource<render::Texture> arrowTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_keyboard_arrow_left);
	AutoResource<render::Texture> circleTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_circle);
	AutoResource<render::Texture> pickaxeTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_pickaxe);
	AutoResource<FrameAnimation> tickbombAnimation = getAnimation(ResIds::build::sipka_rh_texture_convert::statusicons_anim);

	PlayerControlTracker controlTrackers[2];

	bool hadKeyboard = false;
	TouchPointer* directionTouch = nullptr;
	SapphireDirection touchDirectionDown = SapphireDirection::Undefined;

	Vector2F directionMiddle { 0, 0 };
	float directionRadius = 0.0f;
	Size2F directionsSeparatorOffset { 0, 0 };
	Rectangle pickAxeDrawRect;
	Rectangle placeBombDrawRect;
	Rectangle pickAxeTouchRect;
	Rectangle placeBombTouchRect;

	float controlsVisibility = 1.0f;
	LifeCycleChain<Animation> controlAnimator;
	Level* level;

	Matrix2D mvp;

	Rectangle paddings;

	SapphireScene* scene = nullptr;

	void animateHadKeyboard();
	void findNewDirectionTouchPointer();
	SapphireDirection getTouchedDirection(Vector2F pos);

	virtual void onKeyMapChanged(const SapphireKeyMap& keymap, const SapphireKeyMap& gamepadkeymap) override;

	void applyKeyMap(const SapphireKeyMap& map);
	void applyGamePadKeyMap(const SapphireKeyMap& map);

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	void updateControlPositions(const core::WindowSize& size, const SapphireSettings& settings);

	friend class LevelControllerExecutor;
public:
	LevelController(Level* level);
	~LevelController();

	bool onKeyEvent();
	bool onTouchEvent();
	void cancelTouch();

	void clearInput();

	void setScene(SapphireScene* scene);

	void onSizeChanged(const core::WindowSize& size);

	void draw(float displaypercent);

	void applyControls();

	unsigned int getActedPlayerCount() const;
	bool hasActions();
	bool hasAllActions();

	const Rectangle& getPaddings() const {
		return paddings;
	}

	void hideControls();

	bool hasBombDown(unsigned int plrindex) {
		return controlTrackers[plrindex].hadBomb;
	}
	bool isBombDown(unsigned int plrindex) {
		return controlTrackers[plrindex].bombKeyDown;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_LEVELCONTROLLER_H_ */
