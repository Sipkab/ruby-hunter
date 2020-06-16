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
 * DifficultySelectorLayer.h
 *
 *  Created on: 2016. apr. 21.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIFFICULTYSELECTORLAYER_H_
#define TEST_SAPPHIRE_DIFFICULTYSELECTORLAYER_H_

#include <framework/animation/Animation.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/LifeCycleChain.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/SapphireScene.h>

namespace userapp {
using namespace rhfw;
class LevelDrawer3D;

class DifficultySelectorLayer: public SapphireUILayer, private SapphireScene::SettinsChangedListener, private core::TimeListener {
private:
	rhfw::AutoResource<rhfw::render::Texture> backIconWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_arrow_back_white);
	rhfw::AutoResource<rhfw::render::Texture> statsIconWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_chart_bar);
	AutoResource<rhfw::render::Texture> leftArrowWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_chevron_left_white);
	AutoResource<rhfw::render::Texture> rightArrowWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_chevron_right_white);

	static const unsigned int DIFFICULTY_VIEW_COUNT = SAPPHIRE_DIFFICULTY_COUNT + 1;

	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);
	AutoResource<FrameAnimation> icons[DIFFICULTY_VIEW_COUNT];
	AutoResource<FrameAnimation> dualTutorialIcon;
	float iconAnimationValues[DIFFICULTY_VIEW_COUNT] { };
	LifeCycleChain<Animation> iconAnimations[DIFFICULTY_VIEW_COUNT];
	FixedString labels[DIFFICULTY_VIEW_COUNT];
	Rectangle rects[DIFFICULTY_VIEW_COUNT];
	Vector2UI positions[DIFFICULTY_VIEW_COUNT];
	Color colors[DIFFICULTY_VIEW_COUNT];
	Color selectedColors[DIFFICULTY_VIEW_COUNT];
	float scrollOffset = 0.0f;

	LifeCycleChain<Animation> pageAnimator;

	Size2UI pageSize;
	Size2F elementPixelSize;
	unsigned int pageCapacity = 0;

	float titleTextSize = 0.0f;

	float pageOffset = 0.0f;

	static const int SELECTED_PLAYERCOUNT = -2;
	static const int SELECTED_STATS = -3;
	static const unsigned int MAX_PLAYER_COUNT = 2;

	LevelDrawer3D* drawer3D = nullptr;
	SapphireDirection oldYamYamDir = SapphireDirection::Undefined;
	SapphireDirection yamYamDir = SapphireDirection::Undefined;
	SapphireRandom randomer;

	int lastSelected = 0;
	int selectedDifficulty = -1;

	enum class Arrow {
		LEFT,
		RIGHT,
		NONE
	};

	Rectangle arrowLeftIconRect;
	Rectangle arrowRightIconRect;

	Rectangle arrowLeft;
	Rectangle arrowRight;
	Arrow arrowTouch = Arrow::NONE;

	Rectangle statsRect;
	TouchPointer* statsTouch = nullptr;

	Rectangle backButtonPos;
	bool backPressed = false;

#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
	unsigned int playerCount = 0;
	float playerModeTextSize = 0.0f;
	Rectangle playerModeRect;
	TouchPointer* playerModeTouch = nullptr;

	void addToPlayerCount(int count);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

	void onSelectDifficulty(int id);

	void clearSelected() {
		if (selectedDifficulty != -1) {
			//lastSelected = selectedDifficulty;
			setSelectedDifficulty(-1);
		}
	}

	unsigned int getPlayerCount() const {
#ifdef SAPPHIRE_DUAL_PLAYER_AVAILABLE
		return playerCount + 1;
#else
		return 1;
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */
	}

	void startPageAnimator();

	template<Arrow arrow> bool isArrowVisible();

	void startIconAnimation(int index, float target);

	void setSelectedDifficulty(int index);

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	void showLifetimeStatDialog();
public:
	static bool showLockedLevelDialog(SapphireUILayer* parent, SapphireDifficulty difficulty);

	DifficultySelectorLayer(SapphireUILayer* parent);
	~DifficultySelectorLayer();

	virtual void displayKeyboardSelection() override {
		if (selectedDifficulty == -1) {
			setSelectedDifficulty(lastSelected);
		}
	}
	virtual void hideKeyboardSelection() override {
		selectedDifficulty = -1;
	}

	virtual void drawImpl(float displaypercent) override;

	virtual bool onKeyEventImpl() override;

	virtual bool touchImpl() override;

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override;

	virtual void setScene(Scene* scene) override;
};

}
// namespace userapp

#endif /* TEST_SAPPHIRE_DIFFICULTYSELECTORLAYER_H_ */
