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
 * LevelRatingDialogItem.cpp
 *
 *  Created on: 2016. nov. 12.
 *      Author: sipka
 */

#include <sapphire/dialogs/items/LevelRatingDialogItem.h>
#include <sapphire/level/SapphireLevelDescriptor.h>
#include <sapphire/SapphireScene.h>

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break

#define RATING_TEXT "Your rating"

namespace userapp {

using namespace rhfw;

LevelRatingDialogItem::LevelRatingDialogItem(const SapphireLevelDescriptor* descriptor)
		: DialogItem(1.0f), descriptor(descriptor), currentRating(descriptor->userRating) {
	ASSERT(descriptor->userRating >= 0);
	setHighlightable(true);
}
LevelRatingDialogItem::~LevelRatingDialogItem() {
	if (currentRating != descriptor->userRating) {
		static_cast<SapphireScene*>(dialog->getScene())->setLevelUserRating(descriptor, currentRating);
	}
}

void LevelRatingDialogItem::draw(const rhfw::Matrix2D& mvp) {
	const float textsize = dialog->getSharedTextSize();
	const bool highlighted = dialog->isHighlighted(this);
	Color color = highlighted ? dialog->getUiSelectedColor() : dialog->getUiColor();

	Vector2F pos { (getRect().leftTop() + getRect().leftBottom()) / 2 };
	float textw = drawString(mvp, RATING_TEXT, dialog->getFont(), color, pos, dialog->getSharedTextSize(),
			Gravity::LEFT | Gravity::CENTER_VERTICAL);

	const float free = max(0.0f, getRect().width() - textw - textsize * 6) / 2.0f;

	for (unsigned int i = 0; i < 5; ++i) {
		drawSapphireTexture(mvp, (5 - i) <= currentRating ? starFilled : starOutline, color, getStarRectangle(i, textsize, free),
				Rectangle { 0, 0, 1, 1 });
	}
}

Rectangle LevelRatingDialogItem::getStarRectangle(unsigned int index, float textsize, float free) {
	return Rectangle { getRect().right - textsize * (index + 1) - free, getRect().top, getRect().right - textsize * index - free,
			getRect().bottom };
}

float LevelRatingDialogItem::measureTextWidth(float textsize, float maxwidth) {
	return dialog->getFont()->measureText(RATING_TEXT, textsize) + 6 * textsize;
}

bool LevelRatingDialogItem::onKeyEvent() {
	switch (KeyEvent::instance.getKeycode()) {
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
			BREAK_ON_NOT_DOWN();
			if (currentRating == 0) {
				currentRating = 1;
			} else if (currentRating > 1) {
				--currentRating;
			}
			dialog->invalidate();
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
		case KeyCode::KEY_DIR_RIGHT: {
			BREAK_ON_NOT_DOWN();
			if (currentRating == 0) {
				currentRating = 5;
			} else if (currentRating < 5) {
				++currentRating;
			}
			dialog->invalidate();
			break;
		}
		default: {
			return false;
		}
	}
	return true;
}

void LevelRatingDialogItem::onSelected(const Vector2F* pointer) {
	if (pointer == nullptr) {
		if (currentRating == 0) {
			currentRating = 5;
		} else {
			if (++currentRating > 5) {
				currentRating = 1;
			}
		}
	} else {
		int index = -1;
		const float textsize = dialog->getSharedTextSize();
		const float textw = dialog->getFont()->measureText(RATING_TEXT, textsize);
		const float free = max(0.0f, getRect().width() - textw - textsize * 6) / 2.0f;
		for (unsigned int i = 0; i < 5; ++i) {
			if (getStarRectangle(i, textsize, free).isInside(*pointer)) {
				index = 5 - i - 1;
				break;
			}
		}
		if (currentRating == 0) {
			if (index < 0 || index > 4) {
				currentRating = 5;
			} else {
				currentRating = index + 1;
			}
		} else {
			if (index < 0 || index > 4) {
				if (++currentRating > 5) {
					currentRating = 1;
				}
			} else {
				currentRating = index + 1;
			}
		}
	}
}

} // namespace userapp

