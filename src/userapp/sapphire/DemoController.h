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
 * DemoController.h
 *
 *  Created on: 2016. dec. 20.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DEMOCONTROLLER_H_
#define TEST_SAPPHIRE_DEMOCONTROLLER_H_

#include <framework/core/Window.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/animation/Animation.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/io/touch/TouchEvent.h>

#include <appmain.h>
#include <sapphire/FrameAnimation.h>

namespace userapp {
using namespace rhfw;

class SapphireScene;
class Level;

class DemoController {
	enum class VcrAtlas {
		ACTION_NONE = 0,
		ACTION_MOVE = 1,
		ACTION_PICK = 5,
		ACTION_PUTBOMB = 9,

		CONTROL_RECORD_STOP = 13,
		CONTROL_RECORD_BACK = 14,
		CONTROL_RECORD = 15,
		CONTROL_FRAME = 16,
		CONTROL_FAST_BACK = 17,
		CONTROL_SLOW_BACK = 18,
		CONTROL_BACK = 19,
		CONTROL_PAUSE = 20,
		CONTROL_FORWARD = 21,
		CONTROL_SLOW_FORWARD = 22,
		CONTROL_FAST_FORWARD = 23,
		CONTROL_RECORD_PAUSE = 24,
		CONTROL_RECORD_FORWARD = 25,
		CONTROL_RECORD_SLOW_FORWARD = 26,
		CONTROL_RECORD_FAST_FORWARD = 27,
	};
	enum class ControlIndex {
		FAST_BACK = 0,
		BACK,
		SLOW_BACK,
		PAUSE,
		SLOW_PLAY,
		PLAY,
		FAST_PLAY,
	};
	static const unsigned int CONTROLS_COUNT = 7;

	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	Rectangle paddings;
	Matrix2D mvp;
	Rectangle controlAreaRectangle;
	Rectangle controlAreaBorderRectangle;

	FixedString demoSteps;

	long long turnTime = SAPPHIRE_TURN_MILLIS;
	long long prevTurnTime = SAPPHIRE_TURN_MILLIS;

	VcrAtlas playerAtlasIcon = VcrAtlas::CONTROL_FORWARD;
	ControlIndex playerControlIndex = ControlIndex::PLAY;
	AutoResource<FrameAnimation> vcratlas = getAnimation(ResIds::gameres::game_sapphire::vcr_panel);
	AutoResource<FrameAnimation> controlIcons[CONTROLS_COUNT];
	VcrAtlas vcrIndexAtlas[CONTROLS_COUNT] { VcrAtlas::CONTROL_FAST_BACK, VcrAtlas::CONTROL_BACK, VcrAtlas::CONTROL_SLOW_BACK,
			VcrAtlas::CONTROL_PAUSE, VcrAtlas::CONTROL_SLOW_FORWARD, VcrAtlas::CONTROL_FORWARD, VcrAtlas::CONTROL_FAST_FORWARD };

	unsigned int controlsItemsPerRow = 1;
	unsigned int controlsRowCount = CONTROLS_COUNT;
	TouchPointer* controlsTouch[CONTROLS_COUNT] { };
	Rectangle controlsRects[CONTROLS_COUNT];
	const FrameAnimation::Element* controlElems[CONTROLS_COUNT];

	Rectangle controlButtonRect;
	TouchPointer* controlButtonTouch = nullptr;

	float displayAlpha = 0.0f;
	LifeCycleChain<Animation> controlsAnimation;

	Level* level;

	float hudHeight = 0.0f;

	bool useRecordingIcons = false;

	void setTurnTime(long long ms);

	void setTurnSpeed(ControlIndex index);

	ControlIndex getSelectedTurnSpeed() const {
		for (unsigned int i = 0; i < CONTROLS_COUNT; ++i) {
			if (vcrIndexAtlas[i] == playerAtlasIcon) {
				return (ControlIndex) i;
			}
		}
		THROW()<< "No control index found";
		return ControlIndex::PLAY;
	}

	void drawSpeedIndicator(const Matrix2D& mvp, float alpha, const Rectangle& rect, ControlIndex index, const Color& color, const Color& selcolor);

public:
	DemoController(Level* level);
	~DemoController();

	void onSizeChanged(const core::WindowSize& size);

	void showControls();
	void hideControls();

	void draw(float turnpercent, float displaypercent);

	void setScene(SapphireScene* scene);

	void setDemoSteps(const char* steps, unsigned int length) {
		this->demoSteps = FixedString { steps, length };
	}
	void setDemoSteps(FixedString steps) {
		this->demoSteps = util::move(steps);
	}

	long long getTurnTime() const {
		return turnTime;
	}
	long long getPreviousTurnTime() const {
		return prevTurnTime;
	}
	void setPreviousTurnTime(long long prevturntime) {
		this->prevTurnTime = prevturntime;
	}

	const Rectangle& getPaddings() const {
		return paddings;
	}

	bool onKeyEvent();
	bool onTouchEvent();

	void clearInput();
	void cancelTouch();

	unsigned int getDemoStepsLength() const {
		return demoSteps.length();
	}
	const char* getDemoSteps() const {
		return demoSteps;
	}

	bool isControlsVisible() {
		return displayAlpha > 0.0f || controlsAnimation.get() != nullptr;
	}

	void setUseRecordingIcons();
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_DEMOCONTROLLER_H_ */
