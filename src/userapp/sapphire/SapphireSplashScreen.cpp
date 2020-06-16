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
 * SapphireSplashScreen.cpp
 *
 *  Created on: 2017. febr. 16.
 *      Author: sipka
 */

#include <sapphire/sapphireconstants.h>
#include <sapphire/SapphireScene.h>
#include <framework/scene/Scene.h>

#include <gen/types.h>
#include <gen/xmldecl.h>

#include <sapphire/SapphireSplashScreen.h>
#include <sapphire/dialogs/LoadingDialog.h>

#include <cstring>

using namespace rhfw;
using namespace userapp;

LINK_XML_SIMPLE(SapphireSplashScreen)

namespace userapp {
using namespace rhfw;

SapphireSplashScreen::SapphireSplashScreen(SapphireUILayer* parent)
		: SapphireUILayer(parent), background(true, (unsigned int) (core::time_millis) core::MonotonicTime::getCurrent()) {
	setNeedBackground(false);
}
SapphireSplashScreen::SapphireSplashScreen()
		: background(true, (unsigned int) (core::time_millis) core::MonotonicTime::getCurrent()) {
	setNeedBackground(false);
}
SapphireSplashScreen::~SapphireSplashScreen() {
}

#define SPLASHSCREEN_TIME_MS 2000

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
class ArgumentPlayingLevel: public LoadingDialog {
protected:
	virtual void handleLoadingComplete() override {
		auto* ss = static_cast<SapphireScene*>(getScene());
		ss->startPlayingLevelRequested(ss->getProgramArguments()[2]);
	}
public:
	ArgumentPlayingLevel(SapphireUILayer* parent)
			: LoadingDialog(parent) {
	}
};
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */

void SapphireSplashScreen::drawImpl(float displaypercent) {
	core::time_millis currenttime { core::MonotonicTime::getCurrent() };
	if (state == STATE_NODRAW) {
		state = STATE_DURING;
		startTime = currenttime;
	}
	unsigned int currentmillis = (unsigned int) (currenttime - startTime);
	if (showing && state == STATE_DURING && currentmillis >= SPLASHSCREEN_TIME_MS) {
		state = STATE_FADING;
		dismiss();
	}
	auto&& size = getScene()->getWindow()->getWindowSize().pixelSize;
	background.drawSimpleBackground(displaypercent, (float) size.width() / size.height());

	renderer->setDepthTest(false);
	renderer->initDraw();

	Rectangle texrect { 0, 0, (float) splashTexture->getWidth(), (float) splashTexture->getHeight() };
	Rectangle screct { 0, 0, (float) size.width(), (float) size.height() };

	drawSapphireTexture(Matrix2D { }.setScreenDimension(size), splashTexture, displaypercent, screct.fitInto(texrect).scaleAtMiddle(0.55f),
			Rectangle { 0, 0, 1, 1 });
}

void SapphireSplashScreen::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	static_cast<SapphireScene*>(scene)->setTopSapphireLayer(this);
}

void SapphireSplashScreen::dismiss() {
	SapphireUILayer::dismiss();
	state = STATE_FADING;

#if defined(SAPPHIRE_STEAM_API_AVAILABLE)
	auto* ss = static_cast<SapphireScene*>(getScene());
	int argc = ss->getProgramArgumentCount();
	if (argc >= 2) {
		auto argv = ss->getProgramArguments();
//		if (strcmp("+connect", argv[1]) == 0) {
		if (ss->isLevelsLoaded()) {
			ss->startPlayingLevelRequested(argv[1]);
		} else {
			auto* dialog = new ArgumentPlayingLevel(getParent());
			dialog->showDialog(ss);
		}
//		}
	}
#endif /* defined(SAPPHIRE_STEAM_API_AVAILABLE) */
}

} // namespace userapp
