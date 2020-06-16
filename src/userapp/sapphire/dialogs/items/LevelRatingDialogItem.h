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
 * LevelRatingDialogItem.h
 *
 *  Created on: 2016. nov. 12.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_DIALOGS_ITEMS_LEVELRATINGDIALOGITEM_H_
#define TEST_SAPPHIRE_DIALOGS_ITEMS_LEVELRATINGDIALOGITEM_H_

#include <framework/geometry/Matrix.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <appmain.h>
#include <sapphire/dialogs/DialogLayer.h>

#include <gen/types.h>
#include <gen/resources.h>

namespace userapp {
class SapphireLevelDescriptor;
} // namespace userapp

namespace userapp {

using namespace rhfw;

class LevelRatingDialogItem: public DialogItem {
	const SapphireLevelDescriptor* descriptor;

	unsigned int currentRating = 0;

	AutoResource<render::Texture> starFilled = getTexture(ResIds::gameres::game_sapphire::art::ic_star_filled);
	AutoResource<render::Texture> starOutline = getTexture(ResIds::gameres::game_sapphire::art::ic_star_outline);

	Rectangle getStarRectangle(unsigned int index, float textsize, float free);
public:
	LevelRatingDialogItem(const SapphireLevelDescriptor* descriptor);
	~LevelRatingDialogItem();

	virtual float measureTextHeight(float textsize, float maxwidth) override {
		return minLines;
	}
	virtual float measureTextWidth(float textsize, float maxwidth) override;
	virtual void onSelected(const Vector2F* pointer) override;
	virtual bool onKeyEvent() override;
	virtual void draw(const rhfw::Matrix2D& mvp) override;
};
} // namespace userapp

#endif /* TEST_SAPPHIRE_DIALOGS_ITEMS_LEVELRATINGDIALOGITEM_H_ */
