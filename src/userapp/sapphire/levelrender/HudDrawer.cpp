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
 * HudDrawer.cpp
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#include <sapphire/levelrender/HudDrawer.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/LevelController.h>

#include <stdio.h>

namespace userapp {

enum class Panel2DAtlas {
	ZERO = 0,
	NINE = 9,
	X = 10,
	TIMEBOMB = 11,
	TIMEBOMB_ON = 11,
	TIMEBOMB_OFF = 11,
	EMERALD = 12,
	TIME = 13,
	KEY_RED = 14,
	KEY_GREEN = 15,
	KEY_BLUE = 16,
	KEY_YELLOW = 17,
	MOVES = 18
};
enum class Panel3DAtlas {
	TIMEBOMB = 0,
	TIMEBOMB_OFF = 0,
	TIMEBOMB_ON = 1,
	EMERALD = 2,
	MOVES = 3,
	TIME = 4,
};

HudDrawer::HudDrawer() {
}

HudDrawer::~HudDrawer() {
}

//#define IS_3D_ARTSTYLE() ((((unsigned int)artStyle) & ((unsigned int)SapphireArtStyle::FLAG_3D)) != 0)
#define IS_3D_ARTSTYLE() true

void HudDrawer::draw(Level& level, float alpha, LevelController* controller) {
#ifdef SAPPHIRE_SCREENSHOT_MODE
#ifndef SAPPHIRE_SCREENSHOT_INCLUDE_HUD

	return;
#endif /* !defined(SAPPHIRE_SCREENSHOT_INCLUDE_HUD) */
#endif /* defined(SAPPHIRE_SCREENSHOT_MODE) */

	Matrix2D amvp;
	amvp.setScreenDimension(size.pixelSize);

	renderer->setDepthTest(false);
	renderer->initDraw();

	int time;
	int moves;
	if (level.getProperties().maxTime > 0) {
		time = level.getProperties().maxTime - (unsigned int) level.getTurn();
		if (time < 0) {
			time = 0;
		}
	} else {
		time = level.getTurn();
	}
	if (level.getProperties().maxSteps > 0) {
		moves = level.getProperties().maxSteps - level.getMoveCount();
		if (moves < 0) {
			moves = 0;
		}
	} else {
		moves = level.getMoveCount();
	}

	SapphireArtStyle artStyle = scene->getSettings().getArtStyleForLevel(level.getInfo().difficulty);

#define GET_ICON(name) panel3D->getAtIndex((unsigned int) Panel3DAtlas::name)
#define GET_BOMB_ICON(plrindex) (controller == nullptr ? GET_ICON(TIMEBOMB) : \
		((controller->hasBombDown(plrindex) || controller->isBombDown(plrindex)) ? GET_ICON(TIMEBOMB_ON) : GET_ICON(TIMEBOMB_OFF)))

#define INDEX_RED 0
#define INDEX_GREEN 1
#define INDEX_BLUE 2
#define INDEX_YELLOW 3

	float rightoffset = 0;
	float leftoffset = 0;
	rightoffset += drawHudObject(amvp, GET_ICON(MOVES), moves, rightoffset, alpha);
	rightoffset += drawHudObject(amvp, GET_ICON(TIME), time, rightoffset, alpha);
	rightoffset += drawHudObject(amvp, GET_ICON(EMERALD), level.getRemainingLoot(), rightoffset, alpha);
	rightoffset += drawHudObject(amvp, GET_BOMB_ICON(0), level.getCollectedBombCount(0), rightoffset, alpha);
	leftoffset = size.pixelSize.width() - hudHeight * 10.0f;
	if (level.isKeyPicked(0, SapphireDynamic::KeyRed)) {
		rightoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_RED), rightoffset, alpha);
	}
	if (level.isKeyPicked(0, SapphireDynamic::KeyGreen)) {
		rightoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_GREEN), rightoffset, alpha);
	}
	if (level.isKeyPicked(0, SapphireDynamic::KeyBlue)) {
		rightoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_BLUE), rightoffset, alpha);
	}
	if (level.isKeyPicked(0, SapphireDynamic::KeyYellow)) {
		rightoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_YELLOW), rightoffset, alpha);
	}
	if (level.isGameLost()) {
		Vector2F rightbottom { size.pixelSize.width() - hudPaddings.width() - rightoffset - hudElemPadding, size.pixelSize.height()
				- hudPaddings.height() - hudHeight / 2.0f };
		rightoffset += drawString(amvp, "LOST", font, Color { 1, 1, 1, alpha }, rightbottom, hudHeight * 3.0f / 4.0f,
				Gravity::RIGHT | Gravity::CENTER_VERTICAL);
	}

	if (level.getPlayerCount() > 1) {
		//dual player level
		leftoffset += drawHudObject(amvp, GET_BOMB_ICON(1), level.getCollectedBombCount(1), leftoffset, alpha);
		if (level.isKeyPicked(1, SapphireDynamic::KeyRed)) {
			leftoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_RED), leftoffset, alpha);
		}
		if (level.isKeyPicked(1, SapphireDynamic::KeyGreen)) {
			leftoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_GREEN), leftoffset, alpha);
		}
		if (level.isKeyPicked(1, SapphireDynamic::KeyBlue)) {
			leftoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_BLUE), leftoffset, alpha);
		}
		if (level.isKeyPicked(1, SapphireDynamic::KeyYellow)) {
			leftoffset += drawHudKey(amvp, keyPanel->getAtIndex(INDEX_YELLOW), leftoffset, alpha);
		}
	}
}

float HudDrawer::drawHudObject(const Matrix2D& mvp, const FrameAnimation::Element& elem, unsigned int value, float rightoffset,
		float alpha) {
	Vector2F lefttop { size.pixelSize.width() - hudPaddings.width() - hudHeight - rightoffset - hudElemPadding, size.pixelSize.height()
			- hudPaddings.height() - hudHeight };
	Vector2F rightbottom { size.pixelSize.width() - hudPaddings.width() - rightoffset - hudElemPadding, size.pixelSize.height()
			- hudPaddings.height() };

	char str[32];
	if (value >= 1000) {
		snprintf(str, 32, "%u", value);
	} else {
		//poor mans itoa
		str[0] = '0' + (value / 100);
		str[1] = '0' + ((value % 100) / 10);
		str[2] = '0' + (value % 10);
		str[3] = 0;
	}
	float width = drawString(mvp, str, font, Color { 1, 1, 1, alpha },
			Vector2F { rightbottom.x(), rightbottom.y() - (rightbottom.y() - lefttop.y()) / 2.0f }, hudHeight,
			Gravity::RIGHT | Gravity::CENTER_VERTICAL);

	lefttop.x() -= width;
	rightbottom.x() -= width;

	drawSapphireTexture(mvp, elem, alpha, Rectangle { lefttop, rightbottom }, Rectangle { elem.getPosition() });
	return width + rightbottom.x() - lefttop.x() + hudElemPadding * 2;
}

float HudDrawer::drawHudKey(const Matrix2D& mvp, const FrameAnimation::Element& elem, float rightoffset, float alpha) {
	float scale = hudHeight / elem.getPosition().height() * ((float) elem.getTexture()->getWidth() / elem.getTexture()->getHeight());

	Vector2F lefttop { size.pixelSize.width() - hudPaddings.width() - elem.getPosition().width() * scale - rightoffset - hudElemPadding,
			size.pixelSize.height() - hudPaddings.height() - hudHeight };
	Vector2F rightbottom { size.pixelSize.width() - hudPaddings.width() - rightoffset - hudElemPadding, size.pixelSize.height()
			- hudPaddings.height() };

	drawSapphireTexture(mvp, elem, alpha, Rectangle { lefttop, rightbottom }, Rectangle { elem.getPosition() });

	return rightbottom.x() - lefttop.x();
}

void HudDrawer::setSize(const core::WindowSize& size, float scale) {
	this->size = size;

	hudHeight = size.toPixelsY(min(size.getPhysicalSize().height() / (18 / scale), 1.0f * scale));
	hudElemPadding = hudHeight / 10.0f;
}

void HudDrawer::setScene(SapphireScene* scene) {
	this->scene = scene;
}
} // namespace userapp

