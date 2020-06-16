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
 * SapphireMenu.h
 *
 *  Created on: 2016. apr. 14.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIREMENU_H_
#define TEST_SAPPHIRE_SAPPHIREMENU_H_

#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/xml/XmlAttributes.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/SapphireUILayer.h>

namespace userapp {
using namespace rhfw;

class SapphireMenuElement;
class LevelDrawer3D;

class SapphireMenu: public SapphireUILayer, private SapphireScene::SettinsChangedListener, private core::TimeListener {

	AutoResource<render::Texture> titleTexture = getTexture(ResIds::gameres::game_sapphire::art::title_sapp);
	int elementCount = 0;
	SapphireMenuElement* elements;
	AutoResource<Font> font;
	int selectedIndex = -1;

	Matrix2D mvp;
	Rectangle titleRect;
	Rectangle* elementRects;
	LifeCycleChain<Animation>* iconAnimations;
	float* iconAnimationValues;
	float elementTextSize = 0.0f;
	float iconTextPadding = 0.0f;

	LevelDrawer3D* drawer3D = nullptr;

	void startIconAnimation(int index, float target);
	void setSelectedItem(int index);

	void showLoadingDialog(unsigned int index);

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;
public:
	SapphireMenu(const xml::XmlNode& node, const xml::XmlAttributes& attributes);
	~SapphireMenu();

	virtual void drawImpl(float displaypercent) override;

	virtual void sizeChanged(const core::WindowSize& size) override;

	void selectMenu(unsigned int index);

	virtual bool touchImpl() override;

	virtual bool onKeyEventImpl() override;

	virtual void displayKeyboardSelection() override {
		if (selectedIndex == -1) {
			selectedIndex = 0;
		}
	}
	virtual void hideKeyboardSelection() override {
		selectedIndex = -1;
	}

	bool addXmlChild(const xml::XmlNode& child);
	void* getXmlChild(const xml::XmlNode& child, const xml::XmlAttributes& attributes);

	virtual void setScene(Scene* scene) override;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIREMENU_H_ */
