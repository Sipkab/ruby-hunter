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
 * SapphireBackgroundLayer.cpp
 *
 *  Created on: 2016. apr. 21.
 *      Author: sipka
 */

#include <framework/geometry/Vector.h>
#include <framework/render/Renderer.h>
#include <framework/xml/XmlAttributes.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <gen/xmldecl.h>
#include <appmain.h>
#include <sapphire/SapphireBackgroundLayer.h>
#include <sapphire/SapphireScene.h>

using namespace rhfw;
using namespace userapp;

LINK_XML_SIMPLE(SapphireBackgroundLayer)

namespace userapp {

SapphireBackgroundLayer::SapphireBackgroundLayer() {
	random.setSeed((unsigned int) core::MonotonicTime::getCurrent());

}
SapphireBackgroundLayer::~SapphireBackgroundLayer() {
}
void SapphireBackgroundLayer::drawLoadingIndicator(const core::WindowSize& windowsize, const Matrix2D& mvp, const FixedString& str) {
	Size2F size { textSize, textSize };
	Size2F padding { windowsize.toPixelsX(0.25f), textSize };
//	auto& animelem = loadingSapphireAnimation->getAtPercent(
//			((long long) core::time_millis { core::MonotonicTime::getCurrent() } % (SAPPHIRE_TURN_MILLIS * 2))
//					/ (SAPPHIRE_TURN_MILLIS * 2.0f));
	Rectangle texpos { padding.width(), windowsize.pixelSize.height() - padding.height() - size.height(), padding.width() + size.width(),
			windowsize.pixelSize.height() - padding.height() };
//	drawSapphireTexture(mvp, animelem, 1.0f, texpos, Rectangle { animelem.getPosition() });
	drawString(mvp, str.begin(), str.end(), font, SAPPHIRE_COLOR,
			Vector2F { texpos.left+ windowsize.toPixelsX(0.1f), texpos.middle().y() }, windowsize.toPixelsY(0.75f),
			Gravity::LEFT | Gravity::CENTER_VERTICAL);
}
void SapphireBackgroundLayer::draw() {
	SapphireScene* ss = static_cast<SapphireScene*>(getScene());

	renderer->setDepthTest(false);
	renderer->initDraw();
	renderer->clearColor(Color { 0, 0, 0, 0 });

	auto windowsize = ss->getUiSize();
	//draw level
	Matrix2D mvp;
	mvp.setScreenDimension(windowsize.pixelSize);

	bool drawlevel = true;
	if (ss->isShowBackgroundLoading()) {
		drawLoadingIndicator(windowsize, mvp, *ss->getBackgroundLoadingTexts().objects().begin());
	}
	if (!ss->isNeedBackground()) {
		return;
	}
	if (descriptor != nullptr) {
		drawer.draw(turnPercent, 1.0f);
		hudDrawer.draw(level, 1.0f);

		renderer->setDepthTest(false);
		renderer->initDraw();

		drawString(mvp, displayText, font, difficultyToColor(descriptor->difficulty),
				Vector2F { 0.0f, drawer.getSize().pixelSize.height() }, textSize, Gravity::LEFT | Gravity::BOTTOM);
		//dim the background demo
		drawRectangleColor(Matrix2D { }.setIdentity(), Color { 0, 0, 0, 0.65 }, Rectangle { -1, 1, 1, -1 });
	}
}

void SapphireBackgroundLayer::loadNextLevelAndDemo() {
	SapphireScene* scene = static_cast<SapphireScene*>(getScene());

	auto begin = scene->getDifficultyBegin(1, SapphireDifficulty::Tutorial);
	auto end = scene->getDifficultyEnd(1, SapphireDifficulty::Tutorial);
	ASSERT(begin != end) << "No background levels available";
	if (begin == end || (*begin)->communityLevel) {
		return;
	}
	//select some level
	auto* prev = descriptor;
	do {
		descriptor = begin[random.next(end - begin)];
	} while (descriptor == prev || descriptor->communityLevel || descriptor->demoCount == 0);
	applyDisplayLevel(descriptor);
}

void SapphireBackgroundLayer::sizeChanged(const core::WindowSize& size) {
	Layer::sizeChanged(size);
	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
	hudDrawer.setSize(size, ss->getUiUserScale());

	float menuicondimcm = min(min(size.getPhysicalWidth() / 10.0f, size.getPhysicalHeight() / 8.0f), 1.5f);
	Size2F menuicondim = size.toPixels(Size2F { menuicondimcm, menuicondimcm });
	drawer.setPaddings(Rectangle { menuicondim, menuicondim });

	textSize = size.toPixelsY(min(size.getPhysicalSize().height() / 18, 1.0f));
}

void SapphireBackgroundLayer::setScene(Scene* scene) {
	Layer::setScene(scene);
	//scene->getWindow()->foregroundTimeListeners += *this;
	SapphireScene* ss = static_cast<SapphireScene*>(scene);
	ss->setBackgroundLayer(this);

	hudDrawer.setScene(ss);

	ss->settingsChangedListeners += *this;
	ss->getWindow()->foregroundTimeListeners += *this;
}

void SapphireBackgroundLayer::onTimeChanged(const core::time_millis& time, const core::time_millis& previous) {
	SapphireScene* ss = static_cast<SapphireScene*>(getScene());
	if (!ss->isNeedBackground()) {
		return;
	}

	if (descriptor == nullptr) {
		if (ss->isLevelsLoaded()) {
			loadNextLevelAndDemo();
		} else {
			return;
		}
	}

	if (!previewMode) {
		long long ms = (long long) (time - previous);

		turnPercent += (float) ms / SAPPHIRE_TURN_MILLIS;
		while (turnPercent >= 1.0f) {
			turnPercent -= 1.0f;
			bool over = level.isOver();

			if (!demo.isOver() && !over) {
				demo.next(level);
			} else if (overTurn > 0 && level.getTurn() - overTurn >= 10) {
				loadNextLevelAndDemo();
			} else {
				level.applyTurn();
			}
			if (!over && level.isOver()) {
				overTurn = level.getTurn();
			}
		}
	}
}

void SapphireBackgroundLayer::onSettingsChanged(const SapphireSettings& settings) {
	if (drawer.hasDrawer()) {
		//if we hasnt loaded the levels yet, we do not have a drawer
		drawer.setDrawer(settings.getArtStyleForLevel(level.getInfo().difficulty));
	}
	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
}

void SapphireBackgroundLayer::applyDisplayLevel(const SapphireLevelDescriptor* descriptor) {
	this->level.loadLevel(descriptor->getFileDescriptor());
	if (!previewMode) {
		demo.play(level.getDemo(random.next(descriptor->demoCount)), level);
		demo.next(level);
	}

	drawer.setDrawer(static_cast<SapphireScene*>(getScene())->getSettings().getArtStyleForLevel(level.getInfo().difficulty));
	drawer.levelReloaded();

	overTurn = 0;

	turnPercent = 0.0f;

	displayText = FixedString { "(" } + descriptor->title + ")";
}

void SapphireBackgroundLayer::setDisplayLevel(const SapphireLevelDescriptor* descriptor) {
	if (descriptor == this->descriptor) {
		//already on this level
		return;
	}
	if (descriptor == nullptr) {
		if (this->descriptor != nullptr) {
			//load the next tutorial level
			loadNextLevelAndDemo();
		}
		return;
	}
	this->descriptor = descriptor;
	applyDisplayLevel(descriptor);
}
} // namespace userapp

