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
 * SapphireUILayer.h
 *
 *  Created on: 2016. apr. 23.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIREUILAYER_H_
#define TEST_SAPPHIREUILAYER_H_

#include <framework/animation/Animation.h>
#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <framework/layer/Layer.h>
#include <framework/utils/LifeCycleChain.h>
#include <gen/types.h>
#include <sapphire/sapphireconstants.h>

namespace userapp {

using namespace rhfw;

class SapphireUILayer: public rhfw::Layer, private rhfw::core::KeyEventListener, private core::WindowAccesStateListener {
private:
	class Hider;
	class Dismisser;
	class Shower;
	class Displayer;
	friend class Hider;

	bool dialog = false;
	bool needBackground = true;
	bool shouldHaveInput = false;
	float displayPercent = 0.0f;
	/**
	 * 1.0f means fully dimmed
	 */
	float dimPercent = 0.0f;

	bool visible = true;

	Color defaultColor = SAPPHIRE_COLOR;
	Color selectedColor = Color { 1, 1, 1, 1 };
protected:
	bool gainedInput = false;
	SapphireUILayer* parent = nullptr;
	const char* returnText = nullptr;

	void setDisplayPercent(float percent) {
		this->displayPercent = percent;
	}

	template<typename ShowerAnimator>
	void showAnimate();
	template<typename HiderAnimator>
	void hideAnimate();

	bool showing = false;

	rhfw::LifeCycleChain<rhfw::Animation> displayAnim;

	virtual void onLosingInput() {
	}
	virtual void onGainingInput() {
	}

	virtual void drawImpl(float displaypercent) = 0;

	virtual bool onKeyEventImpl() {
		return false;
	}
	virtual bool touchImpl() {
		return false;
	}

	virtual bool onBackRequested();

	/**
	 * Animates to be visible, and gets input
	 */
	void showLayer();

	virtual void onSceneSizeInitialized() {
	}

	virtual void onHidingLayer() {
	}
	virtual void onShowingLayer() {
	}

public:
	SapphireUILayer() {
	}
	SapphireUILayer(SapphireUILayer* parent)
			: parent { parent } {
		setColors(parent->getUiColor(), parent->getUiSelectedColor());
	}
	~SapphireUILayer() {
	}

	void setColors(const rhfw::Color& defaultcolor, const rhfw::Color& selectedcolor) {
		this->defaultColor = defaultcolor;
		this->selectedColor = selectedcolor;
	}
	void setColors(SapphireDifficulty diff);

	const rhfw::Color& getUiColor() const {
		return defaultColor;
	}

	const rhfw::Color& getUiSelectedColor() const {
		return selectedColor;
	}

	virtual void draw() override final {
		if (visible) {
			drawImpl(displayPercent);
		}
	}

	virtual void show(Scene* scene, bool dialog = false);
	void showDialog(Scene* scene) {
		show(scene, true);
	}
	virtual void dismiss();
	virtual bool onKeyEvent() override final;
	virtual Layer* touch() override final;
	virtual void onKeyInputChanged(bool hasInput) override {
	}
	virtual bool onShouldTakeKeyInput() override {
		return visible && (parent == nullptr || !parent->visible);
	}

	virtual void displayKeyboardSelection() = 0;
	virtual void hideKeyboardSelection() = 0;

	virtual void setScene(Scene* scene) override;

	bool isNeedBackground() const {
		return needBackground && (parent == nullptr || parent->isNeedBackground());
	}

	void setNeedBackground(bool needBackground) {
		this->needBackground = needBackground;
	}

	SapphireUILayer* getParent() {
		return parent;
	}

	virtual void onVisibilityToUserChanged(core::Window& window, bool visible) override {
	}
	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) override {
	}

	bool hasInputFocus() const {
		return gainedInput;
	}

	float getDisplayPercent() const {
		return displayPercent;
	}

	float getDimPercent() const {
		return dimPercent;
	}

	const char* getNextReturnText() const;
};

}
// namespace userapp

#endif /* TEST_SAPPHIREUILAYER_H_ */
