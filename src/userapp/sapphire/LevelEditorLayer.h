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
 * LevelEditorLayer.h
 *
 *  Created on: 2016. aug. 8.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVELEDITORLAYER_H_
#define TEST_SAPPHIRE_LEVELEDITORLAYER_H_

#include <sapphire/SapphireUILayer.h>
#include <sapphire/level/Level.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/SapphireScene.h>
#include <appmain.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/geometry/Rectangle.h>
#include <gen/resources.h>

namespace userapp {
using namespace rhfw;

class LevelSelectorLayer;
class LevelEditorLayer: public SapphireUILayer, public SapphireScene::SettinsChangedListener {
private:
	AutoResource<render::VertexBuffer> gridBuffer = renderer->createVertexBuffer();
	AutoResource<SimpleColorShader::InputLayout> gridInputLayout = Resource<SimpleColorShader::InputLayout> {
			simpleColorShader->createInputLayout(), [&](SimpleColorShader::InputLayout* il) {
				il->setLayout<SimpleColorShader::VertexInput>(gridBuffer);
			} };
	AutoResource<SimpleColorShader::MVP> gridColorMvp = simpleColorShader->createUniform_MVP();
	AutoResource<SimpleColorShader::ColorUniform> gridColorColor = simpleColorShader->createUniform_ColorUniform();

	AutoResource<render::Texture> menuTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_menu_white);
	AutoResource<render::Texture> settingsTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_settings);
	AutoResource<render::Texture> infoTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_information);
	AutoResource<render::Texture> playTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_play);

	AutoResource<render::Texture> gridOnTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_grid);
	AutoResource<render::Texture> gridOffTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_grid_off);

	AutoResource<render::Texture> expandTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_arrow_expand_all);
	AutoResource<render::Texture> shrinkTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_shrink);

	AutoResource<render::Texture> demosTexture = getTexture(ResIds::gameres::game_sapphire::art::ic_replay_white);

	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	Level toolbarLevel;
	LevelDrawer toolbarDrawer { &toolbarLevel };

	Level level;
	LevelDrawer drawer { &level };

	Level yamyamLevel;
	LevelDrawer yamyamDrawer { &yamyamLevel };

	Vector2F middle { 0.0f, 0.0f };

	Rectangle menuRect;
	TouchPointer* menuTouch = nullptr;
	Rectangle settingsRect;
	TouchPointer* settingsTouch = nullptr;
	Rectangle infoRect;
	TouchPointer* infoTouch = nullptr;
	Rectangle gridRect;
	TouchPointer* gridTouch = nullptr;
	Rectangle expandRect;
	TouchPointer* expandTouch = nullptr;
	Rectangle shrinkRect;
	TouchPointer* shrinkTouch = nullptr;
	Rectangle playRect;
	TouchPointer* playTouch = nullptr;
	Rectangle demosRect;
	TouchPointer* demosTouch = nullptr;

	TouchPointer* drawingTouch = nullptr;
	Vector2F drawingTouchDown { 0, 0 };
	TouchPointer* toolbarTouch = nullptr;
	Vector2F toolbarTouchDown { 0, 0 };

	Vector2F scrollMiddle { 0, 0 };

	Vector2F toolbarMiddle { 0, 0 };
	Vector2F toolbarScroll { 1, -1 };

	bool displayGrid = true;

	const Level::GameObject* selectedObject;
	Vector2I editedLevelItem { 0, 0 };
	static const int DISPLAY_EDITED_HIDE_MIN = 0;
	static const int DISPLAY_EDITED_HIDE_LEVEL = 0;
	static const int DISPLAY_EDITED_HIDE_YAMYAM = 1;
	static const int DISPLAY_EDITED_HIDE_MAX = 1;
	static const int DISPLAY_EDITED_SHOW_MIN = 2;
	static const int DISPLAY_EDITED_SHOW_LEVEL = 2;
	static const int DISPLAY_EDITED_SHOW_YAMYAM = 3;
	static const int DISPLAY_EDITED_SHOW_MAX = 3;
	int displayEditedLevelItem = DISPLAY_EDITED_HIDE_LEVEL;

	bool toolbarScrolling = false;
	unsigned int lastModificationSave = 0;
	unsigned int lastModification = 0;

	const SapphireLevelDescriptor* descriptor = nullptr;

	LevelSelectorLayer* selectorLayer = nullptr;

	bool paintDown = false;

	Vector2UI paintCoords { SAPPHIRE_MAX_LEVEL_DIMENSION, SAPPHIRE_MAX_LEVEL_DIMENSION };

	bool toolbarKeyDirectionModifierDown = false;

	SapphireScene::GamePadStateUpdatedListener::Listener gamepadStateUpdatedListener;

	Vector2F getYamYamMiddle() const {
		return {this->middle.x(), this->middle.y() + 4};
	}

	bool hasUnmodifiedChanges() {
		return lastModification != lastModificationSave;
	}

	void setYamYamRemainderCount(unsigned int count);

	void drawBorder(const Matrix2D& mvp, const Rectangle& out, const Rectangle& in, const Color& color);

	void testLevel();

	void resetToolbarMiddle();

	void executeAddWallBorder();
	void executeExpand(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom);

	void showMenuDialog();
	void showSettingsDialog();
	void showInformationDialog();
	void showExpandDialog();
	void showShrinkDialog();
	void showDemosDialog();

	void checkToolbarScrollValue();
	void checkMapScrollValue();

	bool saveLevel(SapphireUILayer* parent);
	void deleteLevel();

	void resetLevel();

	void clearTouchInteractions() {
		menuTouch = nullptr;
		settingsTouch = nullptr;
		playTouch = nullptr;

		drawingTouch = nullptr;
		toolbarTouch = nullptr;

		paintDown = false;
		toolbarKeyDirectionModifierDown = false;
	}

	template<SapphireDirection Direction>
	void moveToolbarSelection();
	template<SapphireDirection Direction>
	void moveLevelSelection();

	void checkLevelSelectionVisibility();

	void updateGrid();

	class base_constructor_param {
	};
	LevelEditorLayer(SapphireUILayer* parent, base_constructor_param);

	void showMinerMissingDialog(SapphireUILayer* parent);

	bool tryPutObject(const Vector2F& point, Vector2UI* coordout);
	bool tryPutObject(const Vector2F& point, Vector2UI* coordout, Level** levelout);
	bool putObject(const Vector2UI& coords);
	bool putYamYamObject(const Vector2UI& coords);

	bool checkEditedLevelDisplayed();
	bool checkEditedLevelHidden();
	int levelToDisplayHideEdited(Level* level);
protected:
	virtual void onLosingInput() override {
		SapphireUILayer::onLosingInput();
		clearTouchInteractions();
	}

	virtual void drawImpl(float displaypercent) override;

	virtual bool onBackRequested() override;
public:
	LevelEditorLayer(SapphireUILayer* parent);
	LevelEditorLayer(SapphireUILayer* parent, Level copylevel);
	LevelEditorLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc);
	~LevelEditorLayer();

	void setLevelSelectorLayer(LevelSelectorLayer* layer) {
		this->selectorLayer = layer;
	}

	virtual void displayKeyboardSelection() override;
	virtual void hideKeyboardSelection() override {
	}

	virtual void sizeChanged(const core::WindowSize& size) override;

	virtual bool touchImpl() override;
	virtual bool onKeyEventImpl() override;

	virtual void setScene(Scene* scene) override;

	virtual void onSettingsChanged(const SapphireSettings& settings) override;

	virtual void onVisibilityToUserChanged(core::Window& window, bool visible) override {
		clearTouchInteractions();
	}
	virtual void onInputFocusChanged(core::Window& window, bool inputFocused) override {
		clearTouchInteractions();
	}

	void levelDemosChanged() {
		++lastModification;
	}

	void addDemo(Demo* demo);

	void removeDemo(unsigned int demoindex);
	void removeDemo(const Demo* demo);

	const Level& getLevel() const {
		return level;
	}

	const SapphireLevelDescriptor* getDescriptor() {
		return descriptor;
	}
};

} // namespace userapp

#endif /* TEST_SAPPHIRE_LEVELEDITORLAYER_H_ */
