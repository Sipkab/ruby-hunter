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
 * PlayerLayer.h
 *
 *  Created on: 2016. apr. 26.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_PLAYERLAYER_H_
#define TEST_SAPPHIRE_PLAYERLAYER_H_

#include <framework/core/timing.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LinkedNode.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/geometry/Rectangle.h>
#include <gen/types.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelSounder.h>
#include <sapphire/SapphireUILayer.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/animation/Animation.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/levelrender/HudDrawer.h>
#include <sapphire/LevelController.h>
#include <sapphire/level/LevelStatistics.h>

#include <sapphire/steam_opt.h>

namespace userapp {
using namespace rhfw;

class LevelSelectorLayer;
class LevelEditorLayer;

class PlayerLayer: public SapphireUILayer,
		public SapphireScene::SettinsChangedListener,
		private core::TimeListener,
		private GamePadStateListener {
private:
	AutoResource<render::Texture> menuTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_menu_white);
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	TouchPointer* menuTouch = nullptr;
	Rectangle menuRect;

	Level level;
	LevelStatistics finishStatistics;
	LevelDrawer drawer { &level };
	HudDrawer hudDrawer;
	LevelSounder sounder { &level };

	const SapphireLevelDescriptor* descriptor = nullptr;

	float turnPercent = 0.0f;

	unsigned int overTurn = 0;

	bool hadKeyboard = false;

	LevelEditorLayer* editorLayer = nullptr;
	LevelSelectorLayer* selectorLayer = nullptr;

	bool suspendedLevel = false;

	LevelController controller { &level };

	unsigned int playedTurns = 0;

	LevelStatistics statisticsBase;
	LevelStatistics uncommittedStats;

	unsigned int speedIndex = 0;

	void showPausedDialogChoose();
	void showPausedDialog();

	void showEndingDialog();

	void advanceMilliseconds(long long ms);
	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	void commitPlayedTurns();

	virtual void onGamePadAttached(GamePad* gamepad) override;
	virtual void onGamePadDetached(GamePad* gamepad) override;

	STEAM_CALLBACK_MANUAL(PlayerLayer, OnGameOverlayActivated, GameOverlayActivated_t, steamOverlayCallback);
protected:
	virtual void onLosingInput() override;
	virtual void onGainingInput() override;

	virtual bool onBackRequested() override;

	virtual void onHidingLayer() override;
	virtual void onShowingLayer() override;
public:
	PlayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc);
	PlayerLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc, Level level);
	PlayerLayer(SapphireUILayer* parent, LevelEditorLayer* editorlayer);
	~PlayerLayer();

	virtual bool touchImpl() override;
	virtual bool onKeyEventImpl() override;

	void setEditorLayer(LevelEditorLayer* editor) {
		this->editorLayer = editor;
	}
	LevelEditorLayer* getEditorLayer() const {
		return editorLayer;
	}

	virtual void drawImpl(float displaypercent) override;

	virtual void displayKeyboardSelection() override {
		if (!hadKeyboard) {
			hadKeyboard = true;
			controller.hideControls();
		}
	}
	virtual void hideKeyboardSelection() override {
		//dont show the controls back
	}

	void setSuspendedLevel() {
		statisticsBase = level.getStatistics();
		ASSERT(descriptor != nullptr);
		this->suspendedLevel = true;
		this->turnPercent = 0.0f;
	}
	bool isSuspendedLevel() const {
		return suspendedLevel;
	}

	virtual void sizeChanged(const core::WindowSize& size) override;

	void restartLevel();
	void restartSuspendedLevel();
	void quickSuspend();

	const SapphireLevelDescriptor* getDescriptor() const {
		return descriptor;
	}

	virtual void setScene(Scene* scene) override;

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	Level& getLevel() {
		return level;
	}

	void setLevel(const SapphireLevelDescriptor* desc);

	virtual void onVisibilityToUserChanged(core::Window& window, bool visible) override;
	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) override;

	void setSelectorLayer(LevelSelectorLayer* selectorlayer) {
		this->selectorLayer = selectorlayer;
	}
	LevelSelectorLayer* getSelectorLayer() {
		return selectorLayer;
	}

	const FixedString& getLevelTitle() const {
		return level.getInfo().title;
	}

	bool isTestingLevel() const {
		return editorLayer != nullptr;
	}

	LevelDrawer& getDrawer() {
		return drawer;
	}

	const LevelStatistics& getLevelStatistics() const {
		return finishStatistics;
	}

	virtual void dismiss() override;

	unsigned int getUncommittedPlayedTurns() const {
		return playedTurns;
	}

};

}
 // namespace userapp

#endif /* TEST_SAPPHIRE_PLAYERLAYER_H_ */
