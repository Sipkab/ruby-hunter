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
 * HudDrawer.h
 *
 *  Created on: 2016. aug. 9.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_HUDDRAWER_H_
#define TEST_SAPPHIRE_LEVEL_HUDDRAWER_H_

#include <framework/core/Window.h>
#include <framework/geometry/Matrix.h>
#include <framework/resource/Resource.h>
#include <sapphire/level/Level.h>
#include <sapphire/SapphireScene.h>
#include <appmain.h>
#include <sapphire/FrameAnimation.h>
#include <gen/resources.h>

namespace userapp {
using namespace rhfw;
class LevelController;
class HudDrawer {
	core::WindowSize size;

	Size2F hudPaddings { 0, 0 };

	float hudHeight = 0.0f;
	float hudElemPadding = 0.0f;

	AutoResource<FrameAnimation> keyPanel = getAnimation(ResIds::gameres::game_sapphire::hud_keys);
	AutoResource<FrameAnimation> panel3D = getAnimation(ResIds::build::sipka_rh_texture_convert::statusicons_anim);
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	SapphireScene* scene = nullptr;

	float drawHudObject(const Matrix2D& mvp, const FrameAnimation::Element& elem, unsigned int value, float rightoffset, float alpha);
	float drawHudKey(const Matrix2D& mvp, const FrameAnimation::Element& elem, float rightoffset, float alpha);
public:
	HudDrawer();
	~HudDrawer();

	void draw(Level& level, float alpha, LevelController* controller = nullptr);

	void setSize(const core::WindowSize& size, float scale);
	void setScene(SapphireScene* scene);

	void setHudPaddings(const Size2F& hudPaddings) {
		this->hudPaddings = hudPaddings;
	}
	const Size2F& getHudPaddings() const {
		return hudPaddings;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_HUDDRAWER_H_ */
