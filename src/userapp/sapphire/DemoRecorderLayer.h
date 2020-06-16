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
 * DemoRecorderLayer.h
 *
 *  Created on: 2016. dec. 17.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DEMORECORDERLAYER_H_
#define TEST_SAPPHIRE_DEMORECORDERLAYER_H_

#include <sapphire/SapphireUILayer.h>
#include <sapphire/level/Level.h>
#include <sapphire/DemoLayer.h>
#include <sapphire/LevelController.h>

namespace userapp {
class LevelEditorLayer;

class DemoRecorderLayer final: public DemoLayer, private GamePadStateListener {
private:
	LevelController levelController;
	char* demoSteps = new char[512];
	unsigned int capacity = 512;

	long long actStart = -1;

	bool endResumed = false;

	LevelEditorLayer* editorLayer = nullptr;

	bool spaceDown = false;
	TouchPointer* spaceTouch = nullptr;
	Rectangle spaceRect;

	unsigned int overTurn = 0;

	virtual bool isOver() override;
	virtual int getOverTurns() override;

	void promptSaveDemoAndExit(SapphireUILayer* dialog);

	virtual void onGamePadAttached(GamePad* gamepad) override;
	virtual void onGamePadDetached(GamePad* gamepad) override;
protected:
	virtual void onLevelEnded() override;
	virtual void showPausedDialog() override;

	virtual void displayKeyboardSelection() override {
		DemoLayer::displayKeyboardSelection();
		levelController.hideControls();
	}

	virtual void goToNextTurn() override;
	virtual void goToPreviousTurn(unsigned int target) override;

	virtual void onPausedIdle(long long ms) override;

	virtual bool touchImpl() override;
	virtual bool onKeyEventImpl() override;

	virtual void onLosingInput() override;

	virtual void drawDemoRelated(float displaypercent) override;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	virtual void drawHud(float displaypercent) override;
public:
	DemoRecorderLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* descriptor, Level level);
	~DemoRecorderLayer();

	virtual void drawImpl(float displaypercent) override;

	virtual void sizeChanged(const core::WindowSize& size) override;

	void setLevelEditorLayer(LevelEditorLayer* layer) {
		this->editorLayer = layer;
	}

	virtual void setScene(Scene* scene) override;

	void applyDemoSteps(const char* steps, unsigned int stepslength);
	void setRandomSeed(unsigned int seed);
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_DEMORECORDERLAYER_H_ */
