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
 * appmain.h
 *
 *  Created on: 2016. febr. 23.
 *      Author: sipka
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include <framework/audio/AudioManager.h>
#include <framework/geometry/Matrix.h>
#include <framework/geometry/Vector.h>
#include <framework/geometry/VertexDataHolder.h>
#include <framework/render/Renderer.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/resource/PackageResource.h>
#include <framework/io/gamepad/GamePadContext.h>
#include <framework/io/files/FileDescriptor.h>
#include <gen/types.h>
#include <gen/resources.h>
#include <gen/shader/SimpleColorShader.h>
#include <gen/shader/SimpleFontShader.h>
#include <gen/shader/SapphireTextureShader.h>
#include <gen/shader/SapphirePhongShader.h>
#include <gen/shader/SapphireTexturedPhongShader.h>
#include <StartConfiguration.h>
#include <sapphire/FrameAnimation.h>

#include <sapphire/sapphireconstants.h>
#include <sapphire/common/commonmain.h>

namespace userapp {
#undef max
#undef min

template<typename T>
const T& max(const T& a, const T& b) {
	return a > b ? a : b;
}
template<typename T>
const T& min(const T& a, const T& b) {
	return a < b ? a : b;
}

void swapRenderer();
void reloadRenderer();
void addNewWindow();

class QuadIndexBuffer;
class SapphireObjectBuffer;
class Sapphire3DObject;
class MaterialLibrary;

extern const char* DIFFICULTY_STRINGS[];
extern const char* CATEGORY_STRINGS[];

const char* difficultyToString(rhfw::SapphireDifficulty diff);
const char* categoryToString(rhfw::SapphireLevelCategory cat);
rhfw::ResId difficultyToAnimation(rhfw::SapphireDifficulty diff);
rhfw::Color difficultyToColor(rhfw::SapphireDifficulty diff);
rhfw::Color difficultyToSelectedColor(rhfw::SapphireDifficulty diff);
const char* difficultyColorToSkillLevelName(SapphireDifficulty diff);

extern rhfw::Resource<rhfw::audio::AudioManager> audioManager;
extern rhfw::Resource<rhfw::render::Renderer> renderer;
extern rhfw::AutoResource<rhfw::SimpleColorShader> simpleColorShader;
extern rhfw::AutoResource<rhfw::SimpleFontShader> simpleFontShader;

extern rhfw::AutoResource<rhfw::SapphireTextureShader> sapphireTextureShader;
extern rhfw::AutoResource<rhfw::SapphirePhongShader> sapphirePhongShader;
extern rhfw::AutoResource<rhfw::SapphireTexturedPhongShader> sapphireTexturedPhongShader;

extern QuadIndexBuffer quadIndexBuffer;

extern StartConfiguration startConfig;

extern GamePadContext* gamepadContext;

enum class SapphireMarkerAtlas {
	CATEGORY_START = 0,
	MARKER_TICK = 7,
	MARKER_QUESTION = 8,
	MARKER_SUSPEND = 9,
	MARKER_DUALPLAYER = 10,
};

rhfw::Resource<rhfw::render::Texture> getTexture(rhfw::ResId id);
rhfw::Resource<rhfw::Font> getFont(rhfw::ResId id);
rhfw::Resource<FrameAnimation> getAnimation(rhfw::ResId id);
template<typename T>
rhfw::Resource<T> getApplicationResource(rhfw::ResId id) {
	return PackageResource::getResourceOrFactory<T>(id, [&]() {
		return Resource<T> {new ResourceBlock(new T(id))};
	});
}

void drawRectangleColor(const rhfw::Matrix<3>& mvp, const rhfw::Color& color, const rhfw::Rectangle& target);
void drawRectangleColor(const rhfw::Matrix<4>& mvp, const rhfw::Color& color, const rhfw::Rectangle& target);

void prepareSapphireTextureDraw();
void drawSapphireTexturePrepared(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, float alpha, const rhfw::Rectangle& target,
		const rhfw::Rectangle& textureSource);
void drawSapphireTexturePrepared(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, const rhfw::Color& colormult,
		const rhfw::Rectangle& target, const rhfw::Rectangle& textureSource);
void drawSapphireTexture(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, float alpha, const rhfw::Rectangle& target,
		const rhfw::Rectangle& textureSource);
void drawSapphireTexture(const rhfw::Matrix<3>& mvp, rhfw::render::Texture& text, const rhfw::Color& colormult,
		const rhfw::Rectangle& target, const rhfw::Rectangle& textureSource);

float drawString(const rhfw::Matrix<3>& mvp, const char* str, rhfw::Font& font, const rhfw::Color& color, const rhfw::Vector2F& pos,
		float size, rhfw::Gravity gravity = rhfw::Gravity::LEFT | rhfw::Gravity::BASELINE);
float drawString(const rhfw::Matrix<3>& mvp, const char* begin, const char* end, rhfw::Font& font, const rhfw::Color& color,
		const rhfw::Vector2F& pos, float size, rhfw::Gravity gravity = rhfw::Gravity::LEFT | rhfw::Gravity::BASELINE);

void changeRenderer(rhfw::RenderConfig renderconfig);
void updateStartConfig(rhfw::RenderConfig renderconfig, rhfw::VSyncOptions vsync, unsigned int msaa);

FixedString numberToString(unsigned int i);
unsigned int stringToNumber(const char* str, unsigned int len, unsigned int defaultvalue);
unsigned int stringToNumber(const char* str, unsigned int len);
unsigned int stringToNumber(const FixedString& str);
unsigned int stringToNumber(const FixedString& str, unsigned int defaultvalue);

void showFatalDialogAndExit(const char* text, int exitvalue);

class StartConfiguration;
const StartConfiguration& getStartConfiguration();

} // namespace userapp

#endif /* APPMAIN_H_ */
