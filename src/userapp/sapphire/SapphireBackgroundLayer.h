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
 * SapphireBackgroundLayer.h
 *
 *  Created on: 2016. apr. 21.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_SAPPHIREBACKGROUNDLAYER_H_
#define TEST_SAPPHIRE_SAPPHIREBACKGROUNDLAYER_H_

#include <framework/core/timing.h>
#include <framework/layer/Layer.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <gen/resources.h>
#include <appmain.h>
#include <sapphire/level/Demo.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/levelrender/HudDrawer.h>
#include <sapphire/level/SapphireRandom.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/levelrender/Level3DBackground.h>

namespace userapp {
class SapphireLevelDescriptor;
} // namespace userapp

namespace userapp {

class SapphireBackgroundLayer: public rhfw::Layer, public SapphireScene::SettinsChangedListener, private core::TimeListener {
private:
//	AutoResource<FrameAnimation> loadingSapphireAnimation = getAnimation(ResIds::gameres::game_sapphire::art::sapphire_anim);
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	SapphireRandom random;
	const SapphireLevelDescriptor* descriptor = nullptr;
	Level level;
	LevelDrawer drawer { &level };
	HudDrawer hudDrawer;
	float turnPercent = 0.0f;

	DemoPlayer demo;
	unsigned int overTurn = 0;

	float textSize = 0.0f;
	rhfw::FixedString displayText;

	bool previewMode = false;

	void loadNextLevelAndDemo();

	void drawLoadingIndicator(const core::WindowSize& windowsize, const Matrix2D& mvp, const FixedString& str);

	virtual void onTimeChanged(const core::time_millis& time, const core::time_millis& previous) override;

	void applyDisplayLevel(const SapphireLevelDescriptor* descriptor);
public:
	SapphireBackgroundLayer();
	~SapphireBackgroundLayer();

	virtual void draw() override;

	virtual void sizeChanged(const core::WindowSize& size) override;

	virtual void setScene(Scene* scene) override;

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	void setDisplayLevel(const SapphireLevelDescriptor* descriptor);

	void setPreviewMode(bool preview) {
		this->previewMode = preview;
	}
	bool isPreviewMode() const {
		return this->previewMode;
	}

	const SapphireLevelDescriptor* getDescriptor() const {
		return descriptor;
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_SAPPHIREBACKGROUNDLAYER_H_ */
