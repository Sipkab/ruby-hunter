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
 * SapphireSplashScreen.h
 *
 *  Created on: 2017. febr. 16.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIRESPLASHSCREEN_H_
#define TEST_SAPPHIRE_SAPPHIRESPLASHSCREEN_H_

#include <framework/xml/XmlAttributes.h>
#include <framework/xml/XmlNode.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>

#include <appmain.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/levelrender/Level3DBackground.h>

#include <gen/resources.h>

namespace userapp {
using namespace rhfw;

class SapphireSplashScreen: public SapphireUILayer {
	StandaloneLevel3DBackground background;
	AutoResource<render::Texture> splashTexture = getTexture(ResIds::gameres::game_sapphire::art::icon_splash);

	static const int STATE_NODRAW = 0;
	static const int STATE_DURING = 1;
	static const int STATE_FADING = 2;
	int state = STATE_NODRAW;

	core::time_millis startTime { 0 };
public:
	SapphireSplashScreen(SapphireUILayer* parent);
	SapphireSplashScreen();
	~SapphireSplashScreen();

	virtual void drawImpl(float displaypercent) override;

	virtual void setScene(Scene* scene) override;

	bool addXmlChild(const xml::XmlNode& child) {
		return false;
	}
	void* getXmlChild(const xml::XmlNode& child, const xml::XmlAttributes& attributes) {
		return nullptr;
	}

	virtual void displayKeyboardSelection() override {
	}
	virtual void hideKeyboardSelection() override {
	}
	virtual void dismiss() override;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIRESPLASHSCREEN_H_ */
