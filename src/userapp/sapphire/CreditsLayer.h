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
 * CreditsLayer.h
 *
 *  Created on: 2016. maj. 20.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_CREDITSLAYER_H_
#define TEST_SAPPHIRE_CREDITSLAYER_H_

#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/SapphireUILayer.h>
#include <sapphire/FastFontDrawer.h>

namespace userapp {

class CreditsLayer: public SapphireUILayer {
private:
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);
	float leading = 0.0f;
	float textSize = 0.0f;
	float scrolledPos = 0.0f;
	float stepBySec = 0.0f;

	bool scrolling = false;
	core::time_millis lastScroll;

	unsigned int creditsLength = 0;
	char* creditsBuffer = nullptr;
	const char* creditsPointer = nullptr;
	float linesScrolledOver = 0.0f;

	FastFontDrawerPool fontDrawerPool;
	FastFontDrawer fontDrawer { fontDrawerPool };

	float longestLineWidth();
protected:
	virtual void onLosingInput() override;
	virtual void onGainingInput() override;
public:
	CreditsLayer(SapphireUILayer* parent);
	~CreditsLayer();

	virtual void sizeChanged(const core::WindowSize& size) override;

	virtual void drawImpl(float displaypercent) override;
	virtual bool touchImpl() override;

	virtual void displayKeyboardSelection() override {
		//ignore
	}
	virtual void hideKeyboardSelection() override {
		//ignore
	}

	virtual void setScene(Scene* scene) override;
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_CREDITSLAYER_H_ */
