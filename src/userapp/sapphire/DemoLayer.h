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
 * DemoLayer.h
 *
 *  Created on: 2016. apr. 28.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DEMOLAYER_H_
#define TEST_SAPPHIRE_DEMOLAYER_H_

#include <framework/core/timing.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/animation/PropertyAnimator.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/levelrender/HudDrawer.h>
#include <sapphire/levelrender/LevelSounder.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/DemoController.h>
#include <sapphire/AsynchronTask.h>
#include <sapphire/steam_opt.h>

namespace userapp {

using namespace rhfw;
class LevelSelectorLayer;

class DemoLayer: public SapphireUILayer, public SapphireScene::SettinsChangedListener, private core::TimeListener {
private:
	AutoResource<render::Texture> menuTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_menu_white);
	Rectangle menuRect;
	TouchPointer* menuTouch = nullptr;

	static void playLevelTurns(const char* steps, unsigned int stepslength, unsigned int turncount, Level& level);

	STEAM_CALLBACK_MANUAL(DemoLayer, OnGameOverlayActivated, GameOverlayActivated_t, steamOverlayCallback);
protected:
	const SapphireLevelDescriptor* descriptor;
	Level levelPrototype;
	Level level;
	LevelDrawer drawer {&level};
	HudDrawer hudDrawer;
	LevelSounder sounder {&level};

	float turnPercent = 0.0f;

	Level* backwardCacheProto = nullptr;
	Level* backwardNextCache = nullptr;
	unsigned int backwardLoaderCacheTargetTurn = 0;
	AsynchronTask* levelBackwardLoaderTask = nullptr;

	LevelSelectorLayer* selectorLayer = nullptr;

	DemoController demoController {&level};

	unsigned int pausedTurns = 0;

	void advanceMilliseconds(long long ms);
	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	unsigned int advanceTurns(long long ms, long long turnms, unsigned int maxturns = 0xFFFFFFFF);

	void setPaddings(const Rectangle& paddings);

	virtual void drawHud(float displaypercent);

	void advanceUpdateBackwardCache();
protected:
	virtual void onLosingInput() override;
	virtual void onGainingInput() override;
	void advanceNextTurnPaused();

	virtual bool onBackRequested() override;

	virtual void onPausedIdle(long long ms) {
	}

	void rollbackToTurn(unsigned int turn);

	void goToTurn(unsigned int turn);

	void startCacheLoaderTask(unsigned int cacheturn);

	virtual void onLevelEnded() = 0;
	virtual void showPausedDialog() = 0;

	virtual void goToNextTurn() = 0;
	virtual void goToPreviousTurn(unsigned int target);

	virtual bool isOver() = 0;
	virtual int getOverTurns() = 0;

	void setRandomSeed(unsigned int seed);

	virtual void drawDemoRelated(float displaypercent) {
	}
public:
	DemoLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level&& level);
	~DemoLayer();

	virtual bool touchImpl() override;
	virtual bool onKeyEventImpl() override;

	virtual void drawImpl(float displaypercent) override;

	virtual void displayKeyboardSelection() override {
	}
	virtual void hideKeyboardSelection() override {
	}

	virtual void sizeChanged(const core::WindowSize& size) override;

	const SapphireLevelDescriptor* getDescriptor() const {
		return descriptor;
	}

	virtual void setScene(Scene* scene) override;

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	virtual void onVisibilityToUserChanged(core::Window& window, bool visible) override;
	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) override;

	const Level& getLevel() const {
		return level;
	}
	LevelDrawer& getDrawer() {
		return drawer;
	}

	void setSelectorLayer(LevelSelectorLayer* selectorlayer) {
		this->selectorLayer = selectorlayer;
	}
	LevelSelectorLayer* getSelectorLayer() {
		return selectorLayer;
	}

};
}
  // namespace userapp

#endif /* TEST_SAPPHIRE_DEMOLAYER_H_ */
