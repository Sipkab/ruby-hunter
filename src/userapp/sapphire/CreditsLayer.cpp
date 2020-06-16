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
 * CreditsLayer.cpp
 *
 *  Created on: 2016. maj. 20.
 *      Author: sipka
 */

#include <sapphire/CreditsLayer.h>
#include <sapphire/SapphireScene.h>

#include <appmain.h>

namespace userapp {

CreditsLayer::CreditsLayer(SapphireUILayer* parent)
		: SapphireUILayer(parent) {
	setNeedBackground(false);
	AssetFileDescriptor asset { RAssets::gameres::game_sapphire::credits_txt };
	creditsBuffer = asset.readFully(&creditsLength);
	creditsPointer = creditsBuffer;
}
CreditsLayer::~CreditsLayer() {
	delete[] creditsBuffer;
}

void CreditsLayer::onLosingInput() {
	SapphireUILayer::onLosingInput();
	scrolling = false;
}

void CreditsLayer::onGainingInput() {
	SapphireUILayer::onGainingInput();
	scrolling = true;
	lastScroll = getScene()->getWindow()->getWindowForegroundTime();
}

void CreditsLayer::sizeChanged(const core::WindowSize& size) {
	stepBySec = size.pixelSize.height() / 12.0f;
	SapphireUILayer::sizeChanged(size);
	float oldsize = textSize;
	textSize = min(size.toPixelsY(1.0f), size.pixelSize.height() / 14.0f);

	float longestline = longestLineWidth();
	float sizepad = min(size.pixelSize.width() * 0.1f, size.toPixelsX(1.0f));
	if (longestline > size.pixelSize.width() - sizepad) {
		textSize = textSize / longestline * (size.pixelSize.width() - sizepad);
	}

	leading = font->getLeading(textSize);

	if (oldsize != 0.0f) {
		scrolledPos = scrolledPos * textSize / oldsize;
		linesScrolledOver = linesScrolledOver * textSize / oldsize;
	} else {
		scrolledPos = size.pixelSize.height() + textSize / 2.0f;
		linesScrolledOver = 0.0f;
	}
	if (scrolledPos > size.pixelSize.height() + textSize / 2.0f) {
		scrolledPos = size.pixelSize.height() + textSize / 2.0f;
	}
}

void CreditsLayer::drawImpl(float displayPercent) {
	if (scrolling) {
		core::time_millis time = getScene()->getWindow()->getWindowForegroundTime();
		scrolledPos -= (time - lastScroll) / core::time_millis { 1000 } * stepBySec;
		lastScroll = time;
	}
	auto size = static_cast<SapphireScene*>(getScene())->getUiSize();
	Matrix2D mvp;
	mvp.setScreenDimension(size.pixelSize);

	fontDrawerPool.prepare(font, mvp);
	fontDrawer.prepare(creditsLength);

	const char* start = creditsPointer;
	float ypos = scrolledPos + linesScrolledOver;
	while (true) {
		float displayTextSize = textSize;
		if (*start == '!') {
			++start;
			displayTextSize *= 1.3f;
		}
		const char* end = start;
		while (end < creditsBuffer + creditsLength && *end != '\n') {
			++end;
		}
		if (start != end) {
			//draw
			fontDrawer.add(start, end, Color { 1, 1, 1, displayPercent }, Vector2F { size.pixelSize.width() / 2.0f, ypos }, displayTextSize,
					Gravity::CENTER);
			//drawString(mvp, start, end, font, Color { 1, 1, 1, displayPercent }, Vector2F { size.pixelSize.width() / 2.0f, ypos },
			//	displayTextSize, Gravity::CENTER);
		}
		if (ypos + textSize / 2.0f < 0) {
			creditsPointer = end + 1;
			linesScrolledOver += displayTextSize + leading;
			if (creditsPointer >= creditsBuffer + creditsLength) {
				dismiss();
			}
		}
		ypos += displayTextSize + leading;

		if (end >= creditsBuffer + creditsLength) {
			break;
		}
		start = end + 1;
	}

	fontDrawerPool.commit();
}

float CreditsLayer::longestLineWidth() {
	const char* start = creditsBuffer;
	float max = 0.0f;
	while (true) {
		float displayTextSize = textSize;
		if (*start == '!') {
			++start;
			displayTextSize *= 1.3f;
		}
		const char* end = start;
		while (end < creditsBuffer + creditsLength && *end != '\n') {
			++end;
		}
		if (start != end) {
			//draw
			float measured = font->measureText(start, end, textSize);
			if (measured > max) {
				max = measured;
			}
		}

		if (end >= creditsBuffer + creditsLength) {
			break;
		}
		start = end + 1;
	}
	return max;
}

bool CreditsLayer::touchImpl() {
	dismiss();
	return false;
}

void CreditsLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
}

} // namespace userapp
