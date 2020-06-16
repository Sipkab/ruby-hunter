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
 * LevelEditorLayer.cpp
 *
 *  Created on: 2016. aug. 8.
 *      Author: sipka
 */

#include <sapphire/LevelEditorLayer.h>
#include <sapphire/PlayerLayer.h>
#include <sapphire/dialogs/SettingsLayer.h>
#include <sapphire/dialogs/items/EditTextDialogItem.h>
#include <sapphire/LevelSelectorLayer.h>
#include <sapphire/DemoReplayerLayer.h>
#include <sapphire/DemoRecorderLayer.h>
#include <sapphire/SapphireScene.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/files/StorageFileDescriptor.h>
#include <framework/io/stream/OutputStream.h>
#include <framework/io/gamepad/GamePadContext.h>
#include <framework/io/gamepad/GamePad.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <gen/platform.h>

namespace userapp {

template<typename T>
static T stringToNumberT(const char* str, unsigned int len, T defaultvalue) {
	if (len == 0) {
		return util::move(defaultvalue);
	}
	T result = str[0] - '0';
	for (unsigned int i = 1; i < len; ++i) {
		result = result * 10 + (str[i] - '0');
	}
	return result;
}
template<typename T>
static T stringToNumberT(const FixedString& fs, T defaultvalue) {
	return stringToNumberT((const char*) fs, fs.length(), util::move(defaultvalue));
}

LevelEditorLayer::LevelEditorLayer(SapphireUILayer* parent, base_constructor_param)
		: SapphireUILayer(parent) {
	setNeedBackground(false);
	drawer.splitTwoPlayers = false;
	yamyamDrawer.splitTwoPlayers = false;
	toolbarDrawer.splitTwoPlayers = false;

	drawer.setNeedBackground(false);
	yamyamDrawer.setNeedBackground(false);
	toolbarDrawer.setNeedBackground(false);

#define PALETTE_HEIGHT 10
#define COORDINATE_AIR_X 2
#define COORDINATE_AIR_Y (PALETTE_HEIGHT - 1)
#define COORDINATE_AIR_VEC (Vector2UI{COORDINATE_AIR_X, COORDINATE_AIR_Y})

	toolbarLevel.resize(6, PALETTE_HEIGHT);
	selectedObject = &toolbarLevel.get(4, PALETTE_HEIGHT - 1); //Player
	toolbarLevel.setObject(0, PALETTE_HEIGHT - 1, SapphireObject::Earth);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 1, SapphireObject::Wall);
	toolbarLevel.setObject(COORDINATE_AIR_X, COORDINATE_AIR_Y, SapphireObject::Air);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 1, SapphireObject::StoneWall);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 1, SapphireObject::Player);
#if defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE)
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 1, SapphireObject::Player).setPlayerId(1);
#endif /* defined(SAPPHIRE_DUAL_PLAYER_AVAILABLE) */

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 2, SapphireObject::Glass);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 2, SapphireObject::Sand);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 2, SapphireObject::SandRock);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 2, SapphireObject::RoundStoneWall);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 2, SapphireObject::InvisibleStoneWall);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 2, SapphireObject::Exit);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 3, SapphireObject::Emerald);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 3, SapphireObject::Sapphire);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 3, SapphireObject::Ruby);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 3, SapphireObject::Citrine);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 3, SapphireObject::Bag);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 3, SapphireObject::Rock);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 4, SapphireObject::Bomb);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 4, SapphireObject::TimeBomb);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 4, SapphireObject::TickBomb);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 4, SapphireObject::TNT);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 4, SapphireObject::Elevator);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 4, SapphireObject::Converter);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 5, SapphireObject::Safe);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 5, SapphireObject::Dispenser);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 5, SapphireObject::Swamp);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 5, SapphireObject::Drop);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 5, SapphireObject::Acid);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 5, SapphireObject::Cushion);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 6, SapphireObject::DoorRed);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 6, SapphireObject::DoorGreen);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 6, SapphireObject::DoorBlue);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 6, SapphireObject::DoorYellow);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 6, SapphireObject::PusherLeft);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 6, SapphireObject::PusherRight);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 7, SapphireObject::KeyRed);
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 7, SapphireObject::KeyGreen);
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 7, SapphireObject::KeyBlue);
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 7, SapphireObject::KeyYellow);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 7, SapphireObject::Lorry, SapphireDirection::Up).state = SapphireState::Moving;
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 7, SapphireObject::Lorry, SapphireDirection::Right).state = SapphireState::Moving;

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 8, SapphireObject::Bug, SapphireDirection::Left).state = SapphireState::Moving;
	toolbarLevel.setObject(1, PALETTE_HEIGHT - 8, SapphireObject::Bug, SapphireDirection::Up).state = SapphireState::Moving;
	toolbarLevel.setObject(2, PALETTE_HEIGHT - 8, SapphireObject::Bug, SapphireDirection::Right).state = SapphireState::Moving;
	toolbarLevel.setObject(3, PALETTE_HEIGHT - 8, SapphireObject::Bug, SapphireDirection::Down).state = SapphireState::Moving;
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 8, SapphireObject::Lorry, SapphireDirection::Left).state = SapphireState::Moving;
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 8, SapphireObject::Lorry, SapphireDirection::Down).state = SapphireState::Moving;

	auto& yl = toolbarLevel.setObject(0, PALETTE_HEIGHT - 9, SapphireObject::YamYam, SapphireDirection::Left);
	auto& yt = toolbarLevel.setObject(1, PALETTE_HEIGHT - 9, SapphireObject::YamYam, SapphireDirection::Up);
	auto& yr = toolbarLevel.setObject(2, PALETTE_HEIGHT - 9, SapphireObject::YamYam, SapphireDirection::Right);
	auto& yb = toolbarLevel.setObject(3, PALETTE_HEIGHT - 9, SapphireObject::YamYam, SapphireDirection::Down);
	toolbarLevel.setObject(4, PALETTE_HEIGHT - 9, SapphireObject::Robot);
	toolbarLevel.setObject(5, PALETTE_HEIGHT - 9, SapphireObject::Wheel);

	toolbarLevel.setObject(0, PALETTE_HEIGHT - 10, SapphireObject::DoorOneTime);

	toolbarDrawer.setDispenserFullOpacity(true);
	drawer.setDispenserFullOpacity(true);

	yl.setYamYamOldDirection(yl.direction);
	yt.setYamYamOldDirection(yt.direction);
	yr.setYamYamOldDirection(yr.direction);
	yb.setYamYamOldDirection(yb.direction);
	yl.state = SapphireState::Moving;
	yt.state = SapphireState::Moving;
	yr.state = SapphireState::Moving;
	yb.state = SapphireState::Moving;

	toolbarLevel.resetState();
	yamyamLevel.resetState();

	this->returnText = "Return to level editor";
}
LevelEditorLayer::LevelEditorLayer(SapphireUILayer* parent, Level copylevel)
		: LevelEditorLayer(parent, base_constructor_param { }) {
	this->level = util::move(copylevel);

	middle = this->level.getSize() / 2.0f;
	this->level.getInfo().uuid = SapphireUUID { };

	unsigned int count = level.getYamYamRemainderCount();
	if (count == 0) {
		yamyamLevel.resize(1, 1);
		yamyamLevel.setObject(0, 0, SapphireObject::Air);
	} else {
		yamyamLevel.resize(count * 3 + (count - 1), 3);
		for (int i = 0; i < count * 9; ++i) {
			int region = i / 9;
			yamyamLevel.setObject(region * 4 + i % 3, 2 - ((i % 9) / 3), level.getYamYamRemainders()[i]);
		}
	}

	updateGrid();
	setColors(level.getInfo().difficulty);
}
LevelEditorLayer::LevelEditorLayer(SapphireUILayer* parent)
		: LevelEditorLayer(parent, base_constructor_param { }) {

	level.getInfo().title = "Custom level";
	level.resize(8, 8);
	level.resetState();
	middle = level.getSize() / 2.0f;

	updateGrid();
	setColors(level.getInfo().difficulty);
}
LevelEditorLayer::LevelEditorLayer(SapphireUILayer* parent, const SapphireLevelDescriptor* desc)
		: LevelEditorLayer(parent, base_constructor_param { }) {
	descriptor = desc;

	this->level.loadLevel(desc->getFileDescriptor());
	middle = this->level.getSize() / 2.0f;

	unsigned int count = level.getYamYamRemainderCount();
	if (count == 0) {
		yamyamLevel.resize(1, 1);
		yamyamLevel.setObject(0, 0, SapphireObject::Air);
	} else {
		yamyamLevel.resize(count * 3 + (count - 1), 3);
		for (int i = 0; i < count * 9; ++i) {
			int region = i / 9;
			yamyamLevel.setObject(region * 4 + i % 3, 2 - ((i % 9) / 3), level.getYamYamRemainders()[i]);
		}
	}

	updateGrid();
	setColors(level.getInfo().difficulty);
}
LevelEditorLayer::~LevelEditorLayer() {
}

void LevelEditorLayer::drawImpl(float displaypercent) {

	auto* ss = static_cast<SapphireScene*>(getScene());
	if (gamepadContext != nullptr) {
		if (ss->hasRecognizedGamePadAttached()) {
			for (unsigned int i = 0; i < SAPPHIRE_MAX_PLAYER_COUNT; ++i) {
				auto* state = ss->getGamePadStateForPlayerId(i);
				auto* gp = ss->getGamePadForPlayerId(i);
				if (state != nullptr) {

				}
			}
		}
	}

	auto& pxsize = drawer.getSize().pixelSize;

	Matrix2D mvp;
	mvp.setScreenDimension(pxsize);

	renderer->setDepthTest(false);
	renderer->initDraw();

	const Color backgroundcolor { 0.5f, 0.5f, 0.5f, displaypercent };
	auto levelrect = drawer.getLevelRectangle(middle);
	auto visiblelevelrect = Rectangle { Vector2F { 0, 0 }, pxsize }.intersection(levelrect);
	if (displaypercent >= 1.0f && visiblelevelrect.area() < (pxsize.width() * pxsize.height()) * 0.4) {
		renderer->clearColor(backgroundcolor);
		drawRectangleColor(mvp, Color { 0, 0, 0, displaypercent }, visiblelevelrect);
	} else {
		drawBorder(mvp, Rectangle { Vector2F { 0, 0 }, pxsize }, levelrect, backgroundcolor);
	}

	Vector2F yamyammiddle = getYamYamMiddle();
	if (level.getYamYamRemainderCount() > 0) {
		Rectangle yamrect = yamyamDrawer.getLevelRectangle(yamyammiddle);
		float objwidth = yamyamDrawer.getObjectSize().width();
		//draw rectangles, yamyam level
		for (int i = 0; i < level.getYamYamRemainderCount(); ++i) {
			Rectangle rect { yamrect.left + i * objwidth * 4, yamrect.top, yamrect.left + (3 * (i + 1) + i) * objwidth, yamrect.bottom };
			if (rect.right < 0.0f) {
				//not on screen yet
				continue;
			}
			if (rect.left > pxsize.width()) {
				//went off screen
				break;
			}
			drawRectangleColor(mvp, Color { 0, 0, 0, displaypercent }, rect);
		}
		yamyamDrawer.draw(0.0f, displaypercent, yamyammiddle);
	}

	drawer.draw(0.0f, displaypercent, middle);

	renderer->setDepthTest(false);
	renderer->initDraw();

	if (displayGrid && (level.getWidth() > 1 || level.getHeight() > 1)) {
		//draw grid
		gridColorColor->update( { backgroundcolor });
		gridColorMvp->update(
				{
						Matrix3D().setMatrix(
								Matrix2D().setTranslate(-middle.x(), middle.y() - level.getHeight()).multScale(drawer.getObjectSize()).multTranslate(
										Vector2F { pxsize / 2.0f }) *= mvp) });
		renderer->setTopology(Topology::TRIANGLES);
		quadIndexBuffer.activate();

		gridInputLayout->activate();

		simpleColorShader->useProgram();
		simpleColorShader->set(gridColorColor);
		simpleColorShader->set(gridColorMvp);

		simpleColorShader->drawIndexed(0, 6 * (level.getWidth() - 1) + 6 * (level.getHeight() - 1));
	}

	Color difcol = getUiColor();
	Color seldifcol = getUiSelectedColor();

	switch (displayEditedLevelItem) {
		case DISPLAY_EDITED_SHOW_LEVEL: {
			Rectangle edititemrect = drawer.getObjectRectangle(middle, editedLevelItem);
			drawBorder(mvp, edititemrect.inset(drawer.getSize().toPixels(Vector2F { -0.15f, -0.15f })), edititemrect, Color { difcol.rgb(),
					displaypercent });
			break;
		}
		case DISPLAY_EDITED_SHOW_YAMYAM: {
			Rectangle edititemrect = yamyamDrawer.getObjectRectangle(yamyammiddle, editedLevelItem);
			drawBorder(mvp, edititemrect.inset(yamyamDrawer.getSize().toPixels(Vector2F { -0.15f, -0.15f })), edititemrect,
					Color { difcol.rgb(), displaypercent });
			break;
		}
		default: {
			break;
		}
	}

	drawString(mvp, level.getInfo().title, font, Color { difcol.rgb(), displaypercent },
			Vector2F { menuRect.width() * 0.2f, pxsize.height() - menuRect.height() * 0.2f }, menuRect.height() * 0.5f,
			Gravity::LEFT | Gravity::BASELINE);

	Vector2F toolbarrb = Size2F(toolbarDrawer.getSize().pixelSize) - toolbarScroll * toolbarDrawer.getObjectSize();
	toolbarrb.y() = toolbarDrawer.getSize().pixelSize.height() * 2 - toolbarrb.y();
	Rectangle toolbarRect = { toolbarrb - Size2F(toolbarLevel.getSize()) * toolbarDrawer.getObjectSize(), toolbarrb };
	drawRectangleColor(mvp, Color { 0, 0, 0, displaypercent }, toolbarRect);

	toolbarDrawer.draw(0.0f, displaypercent, toolbarMiddle + toolbarScroll);

	renderer->setDepthTest(false);
	renderer->initDraw();

	drawBorder(mvp, toolbarRect.inset(toolbarDrawer.getSize().toPixels(Vector2F { -0.1f, -0.1f })), toolbarRect, Color { 0.3f, 0.3f, 0.3f,
			displaypercent });

	Rectangle selecteditemrect { toolbarRect.left + selectedObject->x * toolbarDrawer.getObjectSize().height(), toolbarRect.top
			+ (toolbarLevel.getHeight() - selectedObject->y - 1) * toolbarDrawer.getObjectSize().height(), 0, 0 };
	selecteditemrect.bottom = selecteditemrect.top + toolbarDrawer.getObjectSize().height();
	selecteditemrect.right = selecteditemrect.left + toolbarDrawer.getObjectSize().width();
	drawBorder(mvp, selecteditemrect.inset(toolbarDrawer.getSize().toPixels(Vector2F { -0.07f, -0.07f })),
			selecteditemrect.inset(toolbarDrawer.getSize().toPixels(Vector2F { 0.07f, 0.07f })), Color { difcol.rgb(), displaypercent });

	if (menuTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, menuRect);
	}
	if (settingsTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, settingsRect);
	}
	if (infoTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, infoRect);
	}
	if (gridTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, gridRect);
	}
	if (playTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, playRect);
	}
	if (expandTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, expandRect);
	}
	if (shrinkTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, shrinkRect);
	}
	if (demosTouch != nullptr) {
		drawRectangleColor(mvp, Color { difcol.rgb(), displaypercent }, demosRect);
	}
	drawSapphireTexture(mvp, menuTexture, menuTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																			displaypercent }, menuRect, Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, settingsTexture, settingsTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																					displaypercent }, settingsRect,
			Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, infoTexture, infoTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																			displaypercent }, infoRect, Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, displayGrid ? gridOnTexture : gridOffTexture,
			gridTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(), displaypercent }, gridRect, Rectangle {
					0, 0, 1, 1 });
	drawSapphireTexture(mvp, playTexture, playTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																			displaypercent }, playRect, Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, expandTexture, expandTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																				displaypercent }, expandRect, Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, shrinkTexture, shrinkTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																				displaypercent }, shrinkRect, Rectangle { 0, 0, 1, 1 });
	drawSapphireTexture(mvp, demosTexture, demosTouch != nullptr ? Color { seldifcol.rgb(), displaypercent } : Color { difcol.rgb(),
																			displaypercent }, demosRect, Rectangle { 0, 0, 1, 1 });
}
void LevelEditorLayer::drawBorder(const Matrix2D& mvp, const Rectangle& out, const Rectangle& in, const Color& color) {
	if (out.left < in.left) {
		drawRectangleColor(mvp, color, Rectangle { out.left, out.top, in.left, out.bottom });
	}
	if (out.right > in.right) {
		drawRectangleColor(mvp, color, Rectangle { in.right, out.top, out.right, out.bottom });
	}
	if (out.bottom > in.bottom) {
		drawRectangleColor(mvp, color, Rectangle { in.left, in.bottom, in.right, out.bottom });
	}
	if (out.top < in.top) {
		drawRectangleColor(mvp, color, Rectangle { in.left, out.top, in.right, in.top });
	}
}

void LevelEditorLayer::displayKeyboardSelection() {
	//checkEditedLevelDisplayed();
}

static void putObjectTo(Level& level, unsigned int x, unsigned int y, const Level::GameObject& obj) {
	Level::GameObject& target = level.get(x, y);
	switch (target.object) {
		case SapphireObject::StoneWall: {
			if ((obj.object == SapphireObject::Emerald || obj.object == SapphireObject::Sapphire || obj.object == SapphireObject::Ruby
					|| obj.object == SapphireObject::Citrine)
					&& (!target.isStoneWallWithObject() || target.getStoneWallObject() != obj.object)) {
				target.setStoneWallObject(obj.object);
				return;
			}
			break;
		}
		case SapphireObject::Dispenser: {
			if ((obj.object == SapphireObject::Emerald || obj.object == SapphireObject::Sapphire || obj.object == SapphireObject::Ruby
					|| obj.object == SapphireObject::Citrine || obj.object == SapphireObject::Bomb || obj.object == SapphireObject::Bag
					|| obj.object == SapphireObject::Drop) && target.getDispenserObject() != obj.object) {
				target.setDispenserObject(obj.object);
				return;
			}
			break;
		}
		case SapphireObject::Sand: {
			if (obj.object == SapphireObject::Rock) {
				//TODO rather keet it in dynamic
				level.setObject(x, y, SapphireObject::SandRock);
				return;
			}
			break;
		}
		default: {
			break;
		}
	}
	level.setObject(x, y, obj);
}

bool LevelEditorLayer::touchImpl() {
	checkEditedLevelHidden();
	if (!showing || !gainedInput || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		clearTouchInteractions();
		return false;
	}
	auto* affected = TouchEvent::instance.getAffectedPointer();

	switch (TouchEvent::instance.getAction()) {
		case TouchAction::DOWN: {
			scrollMiddle = TouchEvent::instance.getCenter();
			if (drawingTouch != nullptr) {
				//more than one pointer down
				drawingTouch = nullptr;
			}

			auto touchpos = affected->getPosition();
			if (menuTouch == nullptr && menuRect.isInside(touchpos)) {
				menuTouch = affected;
			} else if (playTouch == nullptr && playRect.isInside(touchpos)) {
				playTouch = affected;
			} else if (settingsTouch == nullptr && settingsRect.isInside(touchpos)) {
				settingsTouch = affected;
			} else if (infoTouch == nullptr && infoRect.isInside(touchpos)) {
				infoTouch = affected;
			} else if (gridTouch == nullptr && gridRect.isInside(touchpos)) {
				gridTouch = affected;
			} else if (expandTouch == nullptr && expandRect.isInside(touchpos)) {
				expandTouch = affected;
			} else if (shrinkTouch == nullptr && shrinkRect.isInside(touchpos)) {
				shrinkTouch = affected;
			} else if (demosTouch == nullptr && demosRect.isInside(touchpos)) {
				demosTouch = affected;
			} else if (TouchEvent::instance.getPointerCount() == 1) {
				if (auto* toolbarobj = toolbarLevel.getOptional(
						toolbarDrawer.getCoordinatesForPoint(affected->getPosition(), toolbarMiddle + toolbarScroll))) {
					selectedObject = toolbarobj;
					toolbarTouch = affected;
					toolbarTouchDown = touchpos;
					toolbarScrolling = true;
				} else {
					drawingTouchDown = touchpos;
					drawingTouch = affected;
					paintCoords = Vector2UI { SAPPHIRE_MAX_LEVEL_DIMENSION, SAPPHIRE_MAX_LEVEL_DIMENSION };
					if (paintDown) {
						Level* levelout;
						if (tryPutObject(affected->getPosition(), &paintCoords, &levelout)) {
							editedLevelItem = paintCoords;
							displayEditedLevelItem = levelToDisplayHideEdited(levelout);
						}
					}
				}
			}
			break;
		}
		case TouchAction::MOVE_UPDATE: {
			if (menuTouch != nullptr && !menuRect.isInside(menuTouch->getPosition())) {
				menuTouch = nullptr;
			}
			if (playTouch != nullptr && !playRect.isInside(playTouch->getPosition())) {
				playTouch = nullptr;
			}
			if (settingsTouch != nullptr && !settingsRect.isInside(settingsTouch->getPosition())) {
				settingsTouch = nullptr;
			}
			if (infoTouch != nullptr && !infoRect.isInside(infoTouch->getPosition())) {
				infoTouch = nullptr;
			}
			if (gridTouch != nullptr && !gridRect.isInside(gridTouch->getPosition())) {
				gridTouch = nullptr;
			}
			if (expandTouch != nullptr && !expandRect.isInside(expandTouch->getPosition())) {
				expandTouch = nullptr;
			}
			if (shrinkTouch != nullptr && !shrinkRect.isInside(shrinkTouch->getPosition())) {
				shrinkTouch = nullptr;
			}
			if (demosTouch != nullptr && !demosRect.isInside(demosTouch->getPosition())) {
				demosTouch = nullptr;
			}
			if (drawingTouch != nullptr) {
				if (paintDown) {
					Level* levelout;
					if (tryPutObject(affected->getPosition(), &paintCoords, &levelout)) {
						editedLevelItem = paintCoords;
						displayEditedLevelItem = levelToDisplayHideEdited(levelout);
					}
				} else {
					if ((drawingTouchDown - drawingTouch->getPosition()).length()
							> getScene()->getWindow()->getWindowSize().toPixels(Vector2F { 0.3f, 0.3f }).length()) {
						drawingTouch = nullptr;
					}
				}
			} else if (!toolbarScrolling) {
				//no drawing touch, scroll instead
				Vector2F scrolldiff = TouchEvent::instance.getCenter() - scrollMiddle;
				//transfer to level coordinates
				scrolldiff.y() *= -1;
				middle -= scrolldiff / drawer.getObjectSize();
				checkMapScrollValue();
			}
			if (toolbarTouch != nullptr) {
				if ((toolbarTouchDown - toolbarTouch->getPosition()).length() > getScene()->getWindow()->getWindowSize().toPixels(Vector2F {
						0.3f, 0.3f }).length()) {
					toolbarTouch = nullptr;
				}
			} else if (toolbarScrolling) {
				//no drawing touch, scroll instead
				Vector2F scrolldiff = TouchEvent::instance.getCenter() - scrollMiddle;
				//transfer to level coordinates
				scrolldiff.y() *= -1;
				toolbarScroll -= scrolldiff / toolbarDrawer.getObjectSize();
				checkToolbarScrollValue();
			}
			scrollMiddle = TouchEvent::instance.getCenter();
			break;
		}
		case TouchAction::UP: {
			scrollMiddle = TouchEvent::instance.getCenter();
			if (affected == menuTouch) {
				menuTouch = nullptr;
				showMenuDialog();
			} else if (affected == settingsTouch) {
				settingsTouch = nullptr;
				showSettingsDialog();
			} else if (affected == infoTouch) {
				infoTouch = nullptr;
				showInformationDialog();
			} else if (affected == gridTouch) {
				gridTouch = nullptr;
				displayGrid = !displayGrid;
			} else if (affected == expandTouch) {
				expandTouch = nullptr;
				showExpandDialog();
			} else if (affected == shrinkTouch) {
				shrinkTouch = nullptr;
				showShrinkDialog();
			} else if (affected == playTouch) {
				playTouch = nullptr;
				testLevel();
			} else if (affected == demosTouch) {
				demosTouch = nullptr;
				showDemosDialog();
			} else if (affected == drawingTouch) {
				Level* levelout;
				if (tryPutObject(affected->getPosition(), &paintCoords, &levelout)) {
					editedLevelItem = paintCoords;
					displayEditedLevelItem = levelToDisplayHideEdited(levelout);
				}
				drawingTouch = nullptr;
			}
			if (TouchEvent::instance.getPointerCount() == 0) {
				toolbarScrolling = false;
			}
			break;
		}
		default: {
			break;
		}
	}
	return true;
}

void LevelEditorLayer::checkToolbarScrollValue() {
	if (toolbarScroll.x() < -(int) toolbarLevel.getWidth() + 1) {
		toolbarScroll.x() = -(int) toolbarLevel.getWidth() + 1;
	}
	if (toolbarScroll.x() > toolbarDrawer.getSize().pixelSize.width() / toolbarDrawer.getObjectSize().width() - 1) {
		toolbarScroll.x() = toolbarDrawer.getSize().pixelSize.width() / toolbarDrawer.getObjectSize().width() - 1;
	}
	if (toolbarScroll.y() > (int) toolbarLevel.getHeight() - 1) {
		toolbarScroll.y() = (int) toolbarLevel.getHeight() - 1;
	}
	if (toolbarScroll.y() < -(toolbarDrawer.getSize().pixelSize.height() / toolbarDrawer.getObjectSize().height()) + 1) {
		toolbarScroll.y() = -(toolbarDrawer.getSize().pixelSize.height() / toolbarDrawer.getObjectSize().height()) + 1;
	}
}
void LevelEditorLayer::checkMapScrollValue() {
	int maxw = max(level.getWidth(), yamyamLevel.getWidth());
	int miny = 0;
	if (level.getYamYamRemainderCount() > 0) {
		miny -= 4;
	}
	if (middle.x() > maxw) {
		middle.x() = maxw;
	}
	if (middle.y() > level.getHeight()) {
		middle.y() = level.getHeight();
	}
	if (middle.x() < 0) {
		middle.x() = 0;
	}
	if (middle.y() < miny) {
		middle.y() = miny;
	}
}

bool LevelEditorLayer::checkEditedLevelDisplayed() {
	if (displayEditedLevelItem <= DISPLAY_EDITED_HIDE_MAX && displayEditedLevelItem >= DISPLAY_EDITED_HIDE_MIN) {
		displayEditedLevelItem = displayEditedLevelItem - DISPLAY_EDITED_HIDE_MIN + DISPLAY_EDITED_SHOW_MIN;
		checkLevelSelectionVisibility();
		return true;
	}
	return false;
}
bool LevelEditorLayer::checkEditedLevelHidden() {
	if (displayEditedLevelItem <= DISPLAY_EDITED_SHOW_MAX && displayEditedLevelItem >= DISPLAY_EDITED_SHOW_MIN) {
		displayEditedLevelItem = displayEditedLevelItem - DISPLAY_EDITED_SHOW_MIN + DISPLAY_EDITED_HIDE_MIN;
		return true;
	}
	return false;
}
int LevelEditorLayer::levelToDisplayHideEdited(Level* level) {
	if (level == &this->level) {
		return DISPLAY_EDITED_HIDE_LEVEL;
	}
	ASSERT(level == &this->yamyamLevel);
	return DISPLAY_EDITED_HIDE_YAMYAM;
}

template<>
void LevelEditorLayer::moveToolbarSelection<SapphireDirection::Up>() {
	unsigned short ny = selectedObject->y + 1;
	if (ny >= toolbarLevel.getHeight()) {
		ny = 0;
	}
	selectedObject = &toolbarLevel.get(selectedObject->x, ny);
}
template<>
void LevelEditorLayer::moveToolbarSelection<SapphireDirection::Down>() {
	unsigned short ny = selectedObject->y - 1;
	if (ny >= toolbarLevel.getHeight()) {
		ny = toolbarLevel.getHeight() - 1;
	}
	selectedObject = &toolbarLevel.get(selectedObject->x, ny);
}
template<>
void LevelEditorLayer::moveToolbarSelection<SapphireDirection::Left>() {
	unsigned short nx = selectedObject->x - 1;
	if (nx >= toolbarLevel.getWidth()) {
		nx = toolbarLevel.getWidth() - 1;
	}
	selectedObject = &toolbarLevel.get(nx, selectedObject->y);
}
template<>
void LevelEditorLayer::moveToolbarSelection<SapphireDirection::Right>() {
	unsigned short nx = selectedObject->x + 1;
	if (nx >= toolbarLevel.getWidth()) {
		nx = 0;
	}
	selectedObject = &toolbarLevel.get(nx, selectedObject->y);
}
template<>
void LevelEditorLayer::moveLevelSelection<SapphireDirection::Left>() {
	if (editedLevelItem.x() > 0) {
		--editedLevelItem.x();
		switch (displayEditedLevelItem) {
			case DISPLAY_EDITED_SHOW_LEVEL: {
				if (paintDown) {
					putObject(editedLevelItem);
				}
				break;
			}
			case DISPLAY_EDITED_SHOW_YAMYAM: {
				if (editedLevelItem.x() % 4 == 3) {
					--editedLevelItem.x();
				}
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
				break;
			}
			default: {
				THROW();
				break;
			}
		}
		checkLevelSelectionVisibility();
	}
}
template<>
void LevelEditorLayer::moveLevelSelection<SapphireDirection::Right>() {
	Level& level = displayEditedLevelItem == DISPLAY_EDITED_SHOW_LEVEL ? this->level : this->yamyamLevel;
	if (editedLevelItem.x() + 1 < level.getWidth()) {
		++editedLevelItem.x();
		switch (displayEditedLevelItem) {
			case DISPLAY_EDITED_SHOW_LEVEL: {
				if (paintDown) {
					putObject(editedLevelItem);
				}
				break;
			}
			case DISPLAY_EDITED_SHOW_YAMYAM: {
				if (editedLevelItem.x() % 4 == 3) {
					++editedLevelItem.x();
				}
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
				break;
			}
			default: {
				THROW();
				break;
			}
		}
		checkLevelSelectionVisibility();
	}
}
template<>
void LevelEditorLayer::moveLevelSelection<SapphireDirection::Up>() {
	switch (displayEditedLevelItem) {
		case DISPLAY_EDITED_SHOW_LEVEL: {
			if (editedLevelItem.y() + 1 < level.getHeight()) {
				++editedLevelItem.y();
				checkLevelSelectionVisibility();
				if (paintDown) {
					putObject(editedLevelItem);
				}
			}
			break;
		}
		case DISPLAY_EDITED_SHOW_YAMYAM: {
			if (editedLevelItem.y() + 1 < yamyamLevel.getHeight()) {
				++editedLevelItem.y();
				checkLevelSelectionVisibility();
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
			} else {
				displayEditedLevelItem = DISPLAY_EDITED_SHOW_LEVEL;
				editedLevelItem.y() = 0;
				if (editedLevelItem.x() >= level.getWidth()) {
					editedLevelItem.x() = level.getWidth() - 1;
				}
				checkLevelSelectionVisibility();
				if (paintDown) {
					putObject(editedLevelItem);
				}
			}
			break;
		}
		default: {
			THROW();
			break;
		}
	}
}
template<>
void LevelEditorLayer::moveLevelSelection<SapphireDirection::Down>() {
	switch (displayEditedLevelItem) {
		case DISPLAY_EDITED_SHOW_LEVEL: {
			if (editedLevelItem.y() > 0) {
				--editedLevelItem.y();
				checkLevelSelectionVisibility();
				if (paintDown) {
					putObject(editedLevelItem);
				}
			} else if (level.getYamYamRemainderCount() > 0) {
				displayEditedLevelItem = DISPLAY_EDITED_SHOW_YAMYAM;
				editedLevelItem.y() = yamyamLevel.getHeight() - 1;
				if (editedLevelItem.x() >= yamyamLevel.getWidth()) {
					editedLevelItem.x() = yamyamLevel.getWidth() - 1;
				}
				if (editedLevelItem.x() % 4 == 3) {
					--editedLevelItem.x();
				}
				checkLevelSelectionVisibility();
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
			}
			break;
		}
		case DISPLAY_EDITED_SHOW_YAMYAM: {
			if (editedLevelItem.y() > 0) {
				--editedLevelItem.y();
				checkLevelSelectionVisibility();
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
			}
			break;
		}
		default: {
			THROW();
			break;
		}
	}
}
void LevelEditorLayer::checkLevelSelectionVisibility() {
	auto&& size = getScene()->getWindow()->getWindowSize().pixelSize;
	Rectangle screen = Rectangle { 0, 0, (float) size.width(), (float) size.height() }.inset(drawer.getObjectSize());
	Rectangle selectedrect;
	if (displayEditedLevelItem == DISPLAY_EDITED_SHOW_LEVEL) {
		selectedrect = drawer.getObjectRectangle(middle, editedLevelItem);
	} else {
		selectedrect = yamyamDrawer.getObjectRectangle(getYamYamMiddle(), editedLevelItem);
	}
	if (selectedrect.left < screen.left) {
		middle.x() -= (screen.left - selectedrect.left) / drawer.getObjectSize().width();
	} else if (selectedrect.right > screen.right) {
		middle.x() += (selectedrect.right - screen.right) / drawer.getObjectSize().width();
	}
	if (selectedrect.top < screen.top) {
		middle.y() += (screen.top - selectedrect.top) / drawer.getObjectSize().height();
	} else if (selectedrect.bottom > screen.bottom) {
		middle.y() -= (selectedrect.bottom - screen.bottom) / drawer.getObjectSize().height();
	}
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN) break
#define BREAK_ON_NOT_UP()  if (KeyEvent::instance.getAction() != KeyAction::UP) break
bool LevelEditorLayer::onKeyEventImpl() {
	auto kc = KeyEvent::instance.getKeycode();
	if (kc >= KeyCode::KEY_0 && kc <= KeyCode::KEY_9) {
		if (KeyEvent::instance.getAction() != KeyAction::DOWN)
			return true;

		unsigned int num = (unsigned int) kc - (unsigned int) KeyCode::KEY_0;
		if (num == 0) {
			num = 10;
		}

		unsigned int y = (toolbarLevel.getHeight() - num);
		if (y < toolbarLevel.getHeight()) {
			if (selectedObject->y == y) {
				if ((KeyEvent::instance.getModifiers() & KeyModifiers::SHIFT_ON_BOOL_MASK) != 0) {
					moveToolbarSelection<SapphireDirection::Left>();
				} else {
					moveToolbarSelection<SapphireDirection::Right>();
				}
			} else {
				if ((KeyEvent::instance.getModifiers() & KeyModifiers::SHIFT_ON_BOOL_MASK) != 0) {
					selectedObject = &toolbarLevel.get(toolbarLevel.getWidth() - 1, y);
				} else {
					selectedObject = &toolbarLevel.get(0, y);
				}
			}
		}
	}
	auto* ss = static_cast<SapphireScene*>(getScene());
	if (kc != KeyCode::KEY_UNKNOWN
			&& (kc == ss->getKeyMap()[SapphireKeyCode::KEY_EDITOR_PAINT] || kc == ss->getGamePadKeyMap()[SapphireKeyCode::KEY_EDITOR_PAINT])) {
		paintDown = KeyEvent::instance.getAction() == KeyAction::DOWN;
		switch (displayEditedLevelItem) {
			case DISPLAY_EDITED_SHOW_LEVEL: {
				if (paintDown) {
					putObject(editedLevelItem);
				}
				break;
			}
			case DISPLAY_EDITED_SHOW_YAMYAM: {
				if (paintDown) {
					putYamYamObject(editedLevelItem);
				}
				break;
			}
			default: {
				break;
			}
		}
	}
	switch (kc) {
		case KeyCode::KEY_GAMEPAD_Y:
		case KeyCode::KEY_RIGHT_CTRL:
		case KeyCode::KEY_LEFT_CTRL:
		case KeyCode::KEY_CTRL: {
			toolbarKeyDirectionModifierDown = KeyEvent::instance.getAction() == KeyAction::DOWN;
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_UP:
		case KeyCode::KEY_DIR_UP: {
			BREAK_ON_NOT_DOWN();
			if(toolbarKeyDirectionModifierDown) {
				moveToolbarSelection<SapphireDirection::Up>();
			} else if(!checkEditedLevelDisplayed()) {
				moveLevelSelection<SapphireDirection::Up>();

			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
		case KeyCode::KEY_DIR_DOWN: {
			BREAK_ON_NOT_DOWN();
			if(toolbarKeyDirectionModifierDown) {
				moveToolbarSelection<SapphireDirection::Down>();
			} else if(!checkEditedLevelDisplayed()) {
				moveLevelSelection<SapphireDirection::Down>();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
		case KeyCode::KEY_DIR_LEFT: {
			BREAK_ON_NOT_DOWN();
			if(toolbarKeyDirectionModifierDown) {
				moveToolbarSelection<SapphireDirection::Left>();
			} else if(!checkEditedLevelDisplayed()) {
				moveLevelSelection<SapphireDirection::Left>();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
		case KeyCode::KEY_DIR_RIGHT: {
			BREAK_ON_NOT_DOWN();
			if(toolbarKeyDirectionModifierDown) {
				moveToolbarSelection<SapphireDirection::Right>();
			} else if(!checkEditedLevelDisplayed()) {
				moveLevelSelection<SapphireDirection::Right>();
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_A:
		case KeyCode::KEY_ENTER:
		case KeyCode::KEY_SPACE: {
			BREAK_ON_NOT_DOWN();
			if (!checkEditedLevelDisplayed()) {
				switch (displayEditedLevelItem) {
					case DISPLAY_EDITED_SHOW_LEVEL: {
						putObject(editedLevelItem);
						break;
					}
					case DISPLAY_EDITED_SHOW_YAMYAM: {
						putYamYamObject(editedLevelItem);
						break;
					}
					default: {
						THROW();
						break;
					}
				}
			}
			break;
		}
		case KeyCode::KEY_GAMEPAD_X:
		case KeyCode::KEY_DELETE: {
			BREAK_ON_NOT_DOWN();
			if (!checkEditedLevelDisplayed()) {
				switch (displayEditedLevelItem) {
					case DISPLAY_EDITED_SHOW_LEVEL: {
						putObjectTo(level, editedLevelItem.x(), editedLevelItem.y(), toolbarLevel.get(COORDINATE_AIR_X, COORDINATE_AIR_Y));
						break;
					}
					case DISPLAY_EDITED_SHOW_YAMYAM: {
						putObjectTo(yamyamLevel, editedLevelItem.x(), editedLevelItem.y(), toolbarLevel.get(COORDINATE_AIR_X, COORDINATE_AIR_Y));
						break;
					}
					default: {
						THROW();
						break;
					}
				}
			}
			break;
		}
		case KeyCode::KEY_W: {
			BREAK_ON_NOT_DOWN();
			moveToolbarSelection<SapphireDirection::Up>();
			break;
		}
		case KeyCode::KEY_A: {
			BREAK_ON_NOT_DOWN();
			moveToolbarSelection<SapphireDirection::Left>();
			break;
		}
		case KeyCode::KEY_S: {
			BREAK_ON_NOT_DOWN();
			moveToolbarSelection<SapphireDirection::Down>();
			break;
		}
		case KeyCode::KEY_D: {
			BREAK_ON_NOT_DOWN();
			moveToolbarSelection<SapphireDirection::Right>();
			break;
		}
		case KeyCode::KEY_F1: {
			BREAK_ON_NOT_UP();
			showMenuDialog();
			break;
		}
		case KeyCode::KEY_GAMEPAD_START:
		case KeyCode::KEY_F2: {
			BREAK_ON_NOT_UP();
			testLevel();
			break;
		}
		case KeyCode::KEY_F3: {
			BREAK_ON_NOT_UP();
			showSettingsDialog();
			break;
		}
		case KeyCode::KEY_F4: {
			BREAK_ON_NOT_UP();
			showInformationDialog();
			break;
		}
		case KeyCode::KEY_F5: {
			BREAK_ON_NOT_UP();
			displayGrid = !displayGrid;
			break;
		}
		case KeyCode::KEY_F6: {
			BREAK_ON_NOT_UP();
			showExpandDialog();
			break;
		}
		case KeyCode::KEY_F7: {
			BREAK_ON_NOT_UP();
			showShrinkDialog();
			break;
		}
		case KeyCode::KEY_F8: {
			BREAK_ON_NOT_UP();
			showDemosDialog();
			break;
		}
		default: {
			return SapphireUILayer::onKeyEventImpl();
		}
	}
	return true;
}

bool LevelEditorLayer::onBackRequested() {
	showMenuDialog();
	return true;
}

void LevelEditorLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
	SapphireScene* ss = static_cast<SapphireScene*>(scene);

	if (gamepadContext != nullptr) {
		gamepadStateUpdatedListener =
				SapphireScene::GamePadStateUpdatedListener::make_listener(
						[=](GamePad* gp, unsigned int playerid, const GamePadState& prevstate, const GamePadState& currentstate, core::time_millis milliselapsed) {
							if(!hasInputFocus()) {
								return;
							}
							uint32 thumbmid = (gp->getThumbMax() - gp->getThumbMin()) / 2;
							if(currentstate.thumbLeftX < thumbmid - gp->getLeftThumbDeadzone() || currentstate.thumbLeftX > thumbmid + gp->getLeftThumbDeadzone()) {
								float lthumbx = ((currentstate.thumbLeftX - gp->getThumbMin()) / (float) (gp->getThumbMax() - gp->getThumbMin())) * 2 - 1.0f;
								middle.x() += lthumbx * ((long long)milliselapsed) / 1000.0f * 16;
							}
							if(currentstate.thumbLeftY < thumbmid - gp->getLeftThumbDeadzone() || currentstate.thumbLeftY > thumbmid + gp->getLeftThumbDeadzone()) {
								float lthumby = ((currentstate.thumbLeftY - gp->getThumbMin()) / (float) (gp->getThumbMax() - gp->getThumbMin())) * 2 - 1.0f;
								middle.y() += lthumby * ((long long)milliselapsed) / 1000.0f * 16;
							}
							checkMapScrollValue();
						});
		ss->gamepadStateUpdatedEvents += gamepadStateUpdatedListener;
	}

	SapphireArtStyle artstyle;
	if (ss->getSettings().is3DArtStyle()) {
		artstyle = SapphireArtStyle::ORTHO_3D;
	} else {
		artstyle = SapphireArtStyle::RETRO_2D;
	}

	if (!drawer.hasDrawer()) {
		auto* tbd = toolbarDrawer.setDrawer(artstyle);
		auto* dd = drawer.setDrawer(artstyle);
		auto* yyd = yamyamDrawer.setDrawer(artstyle);
		if (artstyle == SapphireArtStyle::ORTHO_3D) {
			static_cast<LevelDrawer3D*>(tbd)->setFixedLighting(Size2F(toolbarLevel.getSize()) / 2.0f);
		}
	}
	ss->settingsChangedListeners += *this;

	if (descriptor == nullptr) {
		level.getInfo().author = ss->getCurrentUserName();
	}
}

void LevelEditorLayer::testLevel() {
	PlayerLayer* layer = new PlayerLayer(this, this);
	layer->show(getScene());
}

void LevelEditorLayer::showMenuDialog() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Level editor");
	if (ss->hasRecognizedGamePadAttached()) {
		layer->addDialogItem(new CommandDialogItem("Test level", [this,layer] {
			layer->dismiss();
			testLevel();
		}));
		layer->addDialogItem(new CommandDialogItem("Level settings", [this,layer] {
			layer->dismiss();
			showSettingsDialog();
		}));
		layer->addDialogItem(new CommandDialogItem("Level information", [this,layer] {
			layer->dismiss();
			showInformationDialog();
		}));
		layer->addDialogItem(new CommandDialogItem("Expand level", [this,layer] {
			layer->dismiss();
			showExpandDialog();
		}));
		layer->addDialogItem(new CommandDialogItem("Shrink level", [this,layer] {
			layer->dismiss();
			showShrinkDialog();
		}));
		layer->addDialogItem(new CommandDialogItem("Level demos", [this,layer] {
			layer->dismiss();
			showDemosDialog();
		}));
		layer->addDialogItem(new TickDialogItem("Show grid", displayGrid, [=] {
			displayGrid = !displayGrid;
		}));
		layer->addDialogItem(new EmptyDialogItem(0.5f));
	}
	layer->addDialogItem(new CommandDialogItem { "Save", [this,layer] {
		if(saveLevel(layer)) {
			layer->dismiss();
		}
	} });
	layer->addDialogItem(new CommandDialogItem { "Save as...", [this,layer] {
		auto* titleitem = new EditTextDialogItem("Level name:", level.getInfo().title);
		titleitem->setContentMaximumLength(SAPPHIRE_LEVEL_TITLE_MAX_LEN);

		DialogLayer* titledialog = new DialogLayer(layer);
		titledialog->setTitle("Save as...");
		titledialog->addDialogItem(new TextDialogItem("Please enter a new title for the level."));
		titledialog->addDialogItem(titleitem);
		titledialog->addDialogItem(new EmptyDialogItem {0.5f});
		titledialog->addDialogItem(new CommandDialogItem {"Save",
					[this, layer, titledialog, titleitem] {
						level.getInfo().title = FixedString {titleitem->getContent(), titleitem->getContentLength()};
						level.getInfo().uuid = SapphireUUID {};
						descriptor = nullptr;
						if(saveLevel(titledialog)) {
							titledialog->dismiss();
							layer->dismiss();
						}
					}});
		titledialog->addDialogItem(new CommandDialogItem {"Cancel",
					[this, layer, titledialog] {
						titledialog->dismiss();
					}});
		titledialog->show(getScene(), true);
	} });
	layer->addDialogItem(new CommandDialogItem { "Save & exit", [this,layer] {
		if(saveLevel(layer)) {
			layer->dismiss();
			layer->getParent()->dismiss();
		}
	} });
	layer->addDialogItem(new CommandDialogItem { "Exit", [this,layer] {
		if(hasUnmodifiedChanges()) {
			DialogLayer* asksave = new DialogLayer(layer);
			asksave->setTitle("Unsaved changes");
			asksave->addDialogItem(new TextDialogItem("Some changes have not been saved."));
			asksave->addDialogItem(new EmptyDialogItem {0.5f});
			asksave->addDialogItem(new CommandDialogItem {"Save & exit", [this,asksave, layer] {
							if(saveLevel(asksave)) {
								asksave->dismiss();
								layer->dismiss();
								layer->getParent()->dismiss();
							}
						}});
			asksave->addDialogItem(new CommandDialogItem {"Exit", [this,asksave, layer] {
							asksave->dismiss();
							layer->dismiss();
							layer->getParent()->dismiss();
						}});
			asksave->addDialogItem(new CommandDialogItem {"Cancel", [this,asksave, layer] {
							asksave->dismiss();
						}});
			asksave->show(getScene(), true);
		} else {
			layer->dismiss();
			layer->getParent()->dismiss();
		}
	} });
	if (descriptor != nullptr) {
		layer->addDialogItem(new EmptyDialogItem { 0.25f });
		layer->addDialogItem(new CommandDialogItem { "Delete level", [this,layer] {
			DialogLayer* askdelete= new DialogLayer(layer);
			askdelete->setTitle("Delete level");
			askdelete->addDialogItem(new TextDialogItem("Are you sure you want to delete this level?"));
			askdelete->addDialogItem(new EmptyDialogItem {0.5f});
			askdelete->addDialogItem(new CommandDialogItem {"No",
						[this, askdelete] {
							askdelete->dismiss();
						}});
			askdelete->addDialogItem(new CommandDialogItem {"Yes",
						[this, askdelete, layer] {
							deleteLevel();
							askdelete->dismiss();
							layer->dismiss();
							resetLevel();
						}});
			askdelete->show(getScene(), true);
		} });
	}
	if (toolbarScroll != Vector2F { 1, -1 }) {
		layer->addDialogItem(new EmptyDialogItem { 0.25f });
		layer->addDialogItem(new CommandDialogItem { "Reset toolbar", [this, layer] {
			toolbarScroll = Vector2F {1,-1};
			layer->dismiss();
		} });
	}
	layer->addDialogItem(new EmptyDialogItem { 0.5f });
	layer->addDialogItem(new CommandDialogItem { "Settings", [this, layer] {
		auto* settingslayer = new SettingsLayer(layer, static_cast<SapphireScene*>(getScene())->getSettings());
		settingslayer->showDialog(getScene());
	} });
	layer->show(getScene(), true);
}

void LevelEditorLayer::showSettingsDialog() {
	auto* lootitem = new EditTextDialogItem("Loot required:",
			level.getProperties().loot >= 0 ? numberToString(level.getProperties().loot) : "", 4.0f);
	auto* lootdestructitem = new EditTextDialogItem("Loot destructable:",
			level.getProperties().maxLootLose >= 0 ? numberToString(level.getProperties().maxLootLose) : "", 4.0f);
	auto* swampitem = new EditTextDialogItem("Swamp spread rate:", numberToString(level.getProperties().swampRate), 4.0f);
	auto* pushitem = new EditTextDialogItem("Push probability:", numberToString(level.getProperties().pushRate), 4.0f);
	auto* dispenseritem = new EditTextDialogItem("Dispenser speed:", numberToString(level.getProperties().dispenserSpeed), 4.0f);
	auto* elevatoritem = new EditTextDialogItem("Elevator speed:", numberToString(level.getProperties().elevatorSpeed), 4.0f);
	auto* wheelitem = new EditTextDialogItem("Wheel time:", numberToString(level.getProperties().wheelTurnTime), 4.0f);
	auto* robotitem = new EditTextDialogItem("Robot speed:", numberToString(level.getProperties().robotMoveRate), 4.0f);
	auto* yamyamitem = new EditTextDialogItem("Yam-Yam remainders:", numberToString(level.getYamYamRemainderCount()), 4.0f);

	auto* maxtimeitem = new EditTextDialogItem("Time limit:",
			level.getProperties().maxTime > 0 ? numberToString(level.getProperties().maxTime) : "", 4.0f);
	auto* maxstepitem = new EditTextDialogItem("Step limit:",
			level.getProperties().maxSteps > 0 ? numberToString(level.getProperties().maxSteps) : "", 4.0f);

	lootitem->setNumericOnly(true);
	lootdestructitem->setNumericOnly(true);
	swampitem->setNumericOnly(true);
	pushitem->setNumericOnly(true);
	dispenseritem->setNumericOnly(true);
	elevatoritem->setNumericOnly(true);
	wheelitem->setNumericOnly(true);
	robotitem->setNumericOnly(true);
	yamyamitem->setNumericOnly(true);

	maxtimeitem->setNumericOnly(true);
	maxstepitem->setNumericOnly(true);

	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Level settings");
	layer->addDialogItem(lootitem);
	layer->addDialogItem(lootdestructitem);
	layer->addDialogItem(swampitem);
	layer->addDialogItem(pushitem);
	layer->addDialogItem(dispenseritem);
	layer->addDialogItem(elevatoritem);
	layer->addDialogItem(wheelitem);
	layer->addDialogItem(robotitem);
	layer->addDialogItem(yamyamitem);
	layer->addDialogItem(maxtimeitem);
	layer->addDialogItem(maxstepitem);
	layer->addDialogItem(new EmptyDialogItem { 0.5f });
	layer->addDialogItem(
			new CommandDialogItem("Apply",
					[=] {
						unsigned int yamyamrem = stringToNumberT(yamyamitem->getContent(), yamyamitem->getContentLength(), 0);
						if(yamyamrem > SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT) {
							DialogLayer* dialog = new DialogLayer(layer);
							dialog->setTitle("Too many remainders");
							dialog->addDialogItem(new TextDialogItem("You can specify up to " STRINGIZE(SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT) " Yam-Yam remainders."));
							dialog->addDialogItem(new EmptyDialogItem {0.5f});
							dialog->addDialogItem(new CommandDialogItem( "Back", [=] {
												yamyamitem->setContent(STRINGIZE(SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT));
												dialog->dismiss();
											}));

							dialog->show(getScene(), true);
							return;
						}

						level.getProperties().loot = stringToNumberT(
								lootitem->getContent(), lootitem->getContentLength(),
								Level::LevelProperties::LOOT_DEFAULT);
						level.getProperties().maxLootLose = stringToNumberT(
								lootdestructitem->getContent(), lootdestructitem->getContentLength(),
								Level::LevelProperties::MAXLOOTLOSE_DEFAULT);

						level.getProperties().swampRate = stringToNumberT(
								swampitem->getContent(), swampitem->getContentLength(),
								Level::LevelProperties::SWAMPRATE_DEFAULT);
						level.getProperties().pushRate = stringToNumberT(
								pushitem->getContent(), pushitem->getContentLength(),
								Level::LevelProperties::PUSHRATE_DEFAULT);
						level.getProperties().dispenserSpeed = stringToNumberT(
								dispenseritem->getContent(), dispenseritem->getContentLength(),
								Level::LevelProperties::DISPENSERSPEED_DEFAULT);
						level.getProperties().elevatorSpeed = stringToNumberT(
								elevatoritem->getContent(), elevatoritem->getContentLength(),
								Level::LevelProperties::ELEVATORSPEED_DEFAULT);
						level.getProperties().wheelTurnTime = stringToNumberT(
								wheelitem->getContent(), wheelitem->getContentLength(),
								Level::LevelProperties::WHEELTURNTIME_DEFAULT);
						level.getProperties().robotMoveRate = stringToNumberT(
								robotitem->getContent(), robotitem->getContentLength(),
								Level::LevelProperties::ROBOTMOVERATE_DEFAULT);

						level.getProperties().maxTime = stringToNumberT(
								maxtimeitem->getContent(), maxtimeitem->getContentLength(),
								Level::LevelProperties::MAXTIME_DEFAULT);
						level.getProperties().maxSteps = stringToNumberT(
								maxstepitem->getContent(), maxstepitem->getContentLength(),
								Level::LevelProperties::MAXSTEPS_DEFAULT);

						setYamYamRemainderCount(yamyamrem);

						++lastModification;
						layer->dismiss();
					}));
	layer->addDialogItem(new CommandDialogItem("Cancel", [layer] {
		layer->dismiss();
	}));
	layer->show(getScene(), true);
}
static const char* EDITOR_DIFFICULTY_STRINGS[] { "Unrated", "Tutorial", "Simple", "Easy", "Moderate", "Normal", "Tricky", "Tough",
		"Difficult", "Hard", "M.A.D." };
#define APPLY_FLAG(value, flag, condition) (condition ? SET_FLAG(value, flag) : CLEAR_FLAG(value, flag))

void LevelEditorLayer::showInformationDialog() {
	auto* ss = static_cast<SapphireScene*>(getScene());
	auto* nameitem = new EditTextDialogItem("Level name:", level.getInfo().title);
	auto* authoritem = new EditTextDialogItem("Author:", level.getInfo().author.getUserName());
	auto* infoitem = new EditTextDialogItem("Info:", level.getInfo().description);
	auto* difficultyitem = new EnumDialogItem(1.0f, "Difficulty:", EDITOR_DIFFICULTY_STRINGS,
			(level.getInfo().difficulty == SapphireDifficulty::Unrated ? 0 : ((unsigned int) level.getInfo().difficulty + 1)),
			1 + (unsigned int) ss->getMaxAllowedDifficulty() + 1);
	auto* categoryitem = new EnumDialogItem(1.0f, "Category:", CATEGORY_STRINGS, (unsigned int) level.getInfo().category,
			(unsigned int) SapphireLevelCategory::_count_of_entries);
	auto startoutmusicindex = ss->getMusicIndexForName(level.getMusicName());
	auto* musicitem = new EnumDialogItem(1.0f, "Music:", ss->getAvailableMusicItems(), startoutmusicindex,
			ss->getAvailableMusicCount() + 1);
	auto* silentyamyam = new TickDialogItem("Silent yam-yams", level.getProperties().silentYamYam);
	auto* silentexplosions = new TickDialogItem("Silent explosions", level.getProperties().silentExplosion);
	musicitem->setSelectionListener([=](unsigned int sel) {
		ss->setBackgroundMusic(sel == ss->getAvailableMusicCount() ? nullptr : ss->getAvailableMusicItems()[sel]);
	});

	nameitem->setContentMaximumLength(SAPPHIRE_LEVEL_TITLE_MAX_LEN);
	authoritem->setContentMaximumLength(SAPPHIRE_LEVEL_AUTHOR_MAX_LEN);
	infoitem->setContentMaximumLength(SAPPHIRE_LEVEL_DESCRIPTION_MAX_LEN);

	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Level information");
	layer->addDialogItem(nameitem);
	layer->addDialogItem(authoritem);
	layer->addDialogItem(infoitem);
	layer->addDialogItem(difficultyitem);
	layer->addDialogItem(categoryitem);
	layer->addDialogItem(musicitem);
	layer->addDialogItem(silentyamyam);
	layer->addDialogItem(silentexplosions);
	layer->addDialogItem(
			new CommandDialogItem("Leaderboard settings",
					[=] {
						DialogLayer* lblayer = new DialogLayer(layer);
						lblayer->setTitle("Leaderboard settings");
						lblayer->addDialogItem(new TextDialogItem("Specify the available leaderboard types for this level."));
						lblayer->addDialogItem(new EmptyDialogItem(0.5f));
						auto* mostgems = new TickDialogItem("Most gems collected", HAS_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::MostGems));
						auto* leasttime = new TickDialogItem("Fastest completion", HAS_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::LeastTime));
						auto* leaststeps = new TickDialogItem("Least steps taken", HAS_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::LeastSteps));
						lblayer->addDialogItem(mostgems);
						lblayer->addDialogItem(leasttime);
						lblayer->addDialogItem(leaststeps);
						lblayer->addDialogItem(new EmptyDialogItem(0.5f));
						lblayer->addDialogItem(new CommandDialogItem( "Apply", [=] {
											APPLY_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::MostGems, mostgems->isTicked());
											APPLY_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::LeastTime, leasttime->isTicked());
											APPLY_FLAG(level.getProperties().leaderboards, SapphireLeaderboards::LeastSteps, leaststeps->isTicked());
											lblayer->dismiss();
										}));
						lblayer->addDialogItem(new CommandDialogItem( "Cancel", [=] {
											lblayer->dismiss();
										}));
						lblayer->show(scene, true);
					}));
	layer->addDialogItem(new EmptyDialogItem { 0.5f });
	layer->addDialogItem(new CommandDialogItem { "Apply", [=] {
		if(nameitem->getContentLength() == 0) {
			DialogLayer* infolayer = new DialogLayer(layer);
			infolayer->setTitle("No title");
			infolayer->addDialogItem(new TextDialogItem("Title cannot be empty."));
			infolayer->addDialogItem(new EmptyDialogItem {0.5f});
			infolayer->addDialogItem(new CommandDialogItem {"Back",
						[=] () {
							infolayer->dismiss();
						}});
			infolayer->show(getScene(), true);
			layer->setHighlighted(nameitem);
			return;
		}
		level.getInfo().title = nameitem->getContentString();
		level.getInfo().author.getUserName() = authoritem->getContentString();
		level.getInfo().description = infoitem->getContentString();
		level.getInfo().difficulty = (difficultyitem->getSelected() == 0 ?
				SapphireDifficulty::Unrated : (SapphireDifficulty)(difficultyitem->getSelected() - 1));
		level.getInfo().category = (SapphireLevelCategory)categoryitem->getSelected();
		if (musicitem->getSelected() != startoutmusicindex) {
			/*only change music, if the user changed it. else stay the same, as non available music couldve been selected*/
			level.setMusicName(musicitem->getSelected() == ss->getAvailableMusicCount() ?
					nullptr : ss->getAvailableMusicItems()[musicitem->getSelected()]);
		}
		level.getProperties().silentYamYam = silentyamyam->isTicked();
		level.getProperties().silentExplosion = silentexplosions->isTicked();
		++lastModification;
		layer->dismiss();
		setColors(level.getInfo().difficulty);
	} });
	layer->addDialogItem(new CommandDialogItem { "Cancel", [layer] {
		layer->dismiss();
	} });
	layer->show(getScene(), true);
}

void LevelEditorLayer::showExpandDialog() {
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Expand level");
	auto* topitem = new EditTextDialogItem("Add rows to top:", "", 5.0f);
	auto* bottomitem = new EditTextDialogItem("Add rows to bottom:", "", 5.0f);
	auto* leftitem = new EditTextDialogItem("Add columns to left:", "", 5.0f);
	auto* rightitem = new EditTextDialogItem("Add columns to right:", "", 5.0f);

	topitem->setNumericOnly(true);
	bottomitem->setNumericOnly(true);
	leftitem->setNumericOnly(true);
	rightitem->setNumericOnly(true);
	topitem->setContentMaximumLength(10);
	bottomitem->setContentMaximumLength(10);
	leftitem->setContentMaximumLength(10);
	rightitem->setContentMaximumLength(10);

	layer->addDialogItem(
			new TextDialogItem(
					FixedString { "Dimensions: " } + numberToString(level.getWidth()) + "x" + numberToString(level.getHeight())));

	layer->addDialogItem(topitem);
	layer->addDialogItem(bottomitem);
	layer->addDialogItem(leftitem);
	layer->addDialogItem(rightitem);
	layer->addDialogItem(new EmptyDialogItem { 0.25f });
	layer->addDialogItem(new CommandDialogItem { "Add wall border", [=] {
			layer->dismiss();
			executeAddWallBorder();
		} });
	layer->addDialogItem(new EmptyDialogItem { 0.5f });
	layer->addDialogItem(new CommandDialogItem("Apply", [=] {
		unsigned int top = stringToNumberT<unsigned int>(topitem->getContent(), topitem->getContentLength(), 0);
		unsigned int bottom = stringToNumberT<unsigned int>(bottomitem->getContent(), bottomitem->getContentLength(), 0);
		unsigned int left = stringToNumberT<unsigned int>(leftitem->getContent(), leftitem->getContentLength(), 0);
		unsigned int right = stringToNumberT<unsigned int>(rightitem->getContent(), rightitem->getContentLength(), 0);
		if(top != 0 || bottom != 0 || left != 0 || right != 0) {
			if(level.getWidth() + left + right > SAPPHIRE_MAX_LEVEL_DIMENSION) {
				DialogLayer* notiflayer = new DialogLayer(layer);
				notiflayer->setTitle("Level too large");
				notiflayer->addDialogItem(new TextDialogItem(
								"Maximum width is " STRINGIZE(SAPPHIRE_MAX_LEVEL_DIMENSION) ". Please make your level smaller."));
				notiflayer->addDialogItem(new EmptyDialogItem {0.5f});
				notiflayer->addDialogItem(new CommandDialogItem {"Ok", [notiflayer] {
								notiflayer->dismiss();
							}});

				notiflayer->show(getScene(), true);
			} else if(level.getHeight() + top + bottom > SAPPHIRE_MAX_LEVEL_DIMENSION) {
				DialogLayer* notiflayer = new DialogLayer(layer);
				notiflayer->setTitle("Level too large");
				notiflayer->addDialogItem(new TextDialogItem(
								"Maximum height is " STRINGIZE(SAPPHIRE_MAX_LEVEL_DIMENSION) ". Please make your level smaller."));
				notiflayer->addDialogItem(new EmptyDialogItem {0.5f});
				notiflayer->addDialogItem(new CommandDialogItem {"Ok", [notiflayer] {
								notiflayer->dismiss();
							}});

				notiflayer->show(getScene(), true);
			} else {
				executeExpand(left, top, right, bottom);
				layer->dismiss();
			}
		} else {
			layer->dismiss();
		}
	}));
	layer->addDialogItem(new CommandDialogItem { "Cancel", [layer] {
		layer->dismiss();
	} });
	layer->show(getScene(), true);
}
void LevelEditorLayer::executeExpand(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom) {
	if (displayEditedLevelItem == DISPLAY_EDITED_HIDE_LEVEL || displayEditedLevelItem == DISPLAY_EDITED_SHOW_LEVEL) {
		editedLevelItem.x() += left;
		editedLevelItem.y() += bottom;
	}
	middle.x() += left;
	middle.y() += bottom;
	level.expand(left, top, right, bottom);
	++lastModification;
	updateGrid();
}
void LevelEditorLayer::executeAddWallBorder() {
	middle.x() += 1;
	middle.y() += 1;
	level.expand(1, 1, 1, 1);

	for (int i = 0; i < level.getWidth(); ++i) {
		level.setObject(i, 0, SapphireObject::Wall);
		level.setObject(i, level.getHeight() - 1, SapphireObject::Wall);
	}
	for (int j = 1; j < level.getHeight() - 1; ++j) {
		level.setObject(0, j, SapphireObject::Wall);
		level.setObject(level.getWidth() - 1, j, SapphireObject::Wall);
	}

	++lastModification;
	updateGrid();
}

void LevelEditorLayer::showShrinkDialog() {
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Shrink level");
	auto* topitem = new EditTextDialogItem("Remove rows from top:", "", 5.0f);
	auto* bottomitem = new EditTextDialogItem("Remove rows from bottom:", "", 5.0f);
	auto* leftitem = new EditTextDialogItem("Remove columns from left:", "", 5.0f);
	auto* rightitem = new EditTextDialogItem("Remove columns from right:", "", 5.0f);

	topitem->setNumericOnly(true);
	bottomitem->setNumericOnly(true);
	leftitem->setNumericOnly(true);
	rightitem->setNumericOnly(true);

	topitem->setContentMaximumLength(10);
	bottomitem->setContentMaximumLength(10);
	leftitem->setContentMaximumLength(10);
	rightitem->setContentMaximumLength(10);

	layer->addDialogItem(
			new TextDialogItem(
					FixedString { "Dimensions: " } + numberToString(level.getWidth()) + "x" + numberToString(level.getHeight())));
	layer->addDialogItem(topitem);
	layer->addDialogItem(bottomitem);
	layer->addDialogItem(leftitem);
	layer->addDialogItem(rightitem);
	layer->addDialogItem(new EmptyDialogItem { 0.5f });
	layer->addDialogItem(new CommandDialogItem { "Apply", [=] {
		unsigned int top = stringToNumberT<unsigned int>(topitem->getContent(), topitem->getContentLength(), 0);
		unsigned int bottom = stringToNumberT<unsigned int>(bottomitem->getContent(), bottomitem->getContentLength(), 0);
		unsigned int left = stringToNumberT<unsigned int>(leftitem->getContent(), leftitem->getContentLength(), 0);
		unsigned int right = stringToNumberT<unsigned int>(rightitem->getContent(), rightitem->getContentLength(), 0);
		if(top != 0 || bottom != 0 || left != 0 || right != 0) {
			if(left + right >= level.getWidth()) {
				DialogLayer* infolayer = new DialogLayer(layer);
				infolayer->setTitle("Invalid width");
				infolayer->addDialogItem(new TextDialogItem("Level must be at least 1 column wide."));
				infolayer->addDialogItem(new EmptyDialogItem {0.5f});
				infolayer->addDialogItem(new CommandDialogItem {"Back",
							[=] {
								infolayer->dismiss();
							}});
				infolayer->show(getScene(), true);
				layer->setHighlighted(leftitem);
				return;
			}
			if(bottom + top >= level.getHeight()) {
				DialogLayer* infolayer = new DialogLayer(layer);
				infolayer->setTitle("Invalid height");
				infolayer->addDialogItem(new TextDialogItem("Level must be at least 1 row tall."));
				infolayer->addDialogItem(new EmptyDialogItem {0.5f});
				infolayer->addDialogItem(new CommandDialogItem {"Back",
							[=] {
								infolayer->dismiss();
							}});
				infolayer->show(getScene(), true);
				layer->setHighlighted(topitem);
				return;
			}
			level.shrink(left, top, right, bottom);

			if (displayEditedLevelItem == DISPLAY_EDITED_HIDE_LEVEL || displayEditedLevelItem == DISPLAY_EDITED_SHOW_LEVEL) {
				editedLevelItem.x() = max<int>(editedLevelItem.x() - left, 0);
				editedLevelItem.y() = max<int>(editedLevelItem.y() - bottom, 0);
			}

			middle.x() -= left;
			middle.y() -= bottom;
			checkMapScrollValue();
			++lastModification;
			updateGrid();
		}
		layer->dismiss();
	} });
	layer->addDialogItem(new CommandDialogItem { "Cancel", [layer] {
		layer->dismiss();
	} });
	layer->show(getScene(), true);
}

void LevelEditorLayer::showMinerMissingDialog(SapphireUILayer* parent) {
	//show no miners dialog
	DialogLayer* infolayer = new DialogLayer(parent);
	infolayer->setTitle("Miner missing");
	infolayer->addDialogItem(new TextDialogItem("Please add at least one miner to the level."));
	infolayer->addDialogItem(new EmptyDialogItem(0.5f));
	infolayer->addDialogItem(new CommandDialogItem("Back", [=] {
		infolayer->dismiss();
	}));
	infolayer->show(getScene(), true);
}

bool LevelEditorLayer::saveLevel(SapphireUILayer* parent) {
	//not checking modification, always saving

	level.resetState();
	if (level.getMinersTotal() == 0) {
		showMinerMissingDialog(parent);
		return false;
	}

	lastModificationSave = lastModification;
	auto* ss = static_cast<SapphireScene*>(getScene());
	ss->getUserLevelsDirectory().create();

	descriptor = ss->updateLevel(level, descriptor);
	if (selectorLayer != nullptr) {
		selectorLayer->reloadLevels();
		selectorLayer->displaySelection(descriptor);
	}

	LOGTRACE()<< "Saved level ";
	level.updateLevelVersion();
	level.saveLevel(descriptor->getFileDescriptor());

	return true;
}
void LevelEditorLayer::deleteLevel() {
	if (descriptor != nullptr) {
		SapphireScene* ss = static_cast<SapphireScene*>(getScene());
		if (selectorLayer != nullptr) {
			selectorLayer->descriptorRemoved(descriptor);
		}
		descriptor->getFileDescriptor().remove();
		ss->removeLevel(descriptor);
		descriptor = nullptr;
	}
}

void LevelEditorLayer::resetToolbarMiddle() {
	toolbarMiddle = Size2F(toolbarDrawer.getSize().pixelSize) / 2.0f / toolbarDrawer.getObjectSize();
	toolbarMiddle.x() = toolbarLevel.getSize().x() - toolbarMiddle.x();
}

void LevelEditorLayer::sizeChanged(const core::WindowSize& size) {
	SapphireUILayer::sizeChanged(size);

	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
	yamyamDrawer.setSize(ss->getGameSize(), ss->getGameUserScale());

	toolbarDrawer.setSize(ss->getUiSize(), ss->getUiUserScale());
	resetToolbarMiddle();

	float menuicondimcm = min(min(size.getPhysicalWidth() / 10.0f, size.getPhysicalHeight() / 8.0f), 1.5f);
	Size2F menuicondim = size.toPixels(Size2F { menuicondimcm, menuicondimcm });
	menuRect = Rectangle { size.pixelSize.width() - menuicondim.width(), 0, (float) size.pixelSize.width(), menuicondim.height() };
	playRect = menuRect.translate(Vector2F { 0, menuRect.height() });
	settingsRect = playRect.translate(Vector2F { 0, menuRect.height() });
	infoRect = settingsRect.translate(Vector2F { 0, menuRect.height() });
	gridRect = infoRect.translate(Vector2F { 0, menuRect.height() });
	expandRect = gridRect.translate(Vector2F { 0, menuRect.height() });
	shrinkRect = expandRect.translate(Vector2F { 0, menuRect.height() });
	demosRect = shrinkRect.translate(Vector2F { 0, menuRect.height() });

	checkToolbarScrollValue();
	checkMapScrollValue();
}

void LevelEditorLayer::updateGrid() {
	unsigned int width = level.getWidth();
	unsigned int height = level.getHeight();
	unsigned int alloccount = max((unsigned int) 1, 4 * (width - 1) + 4 * (height - 1));
	gridBuffer->setBufferInitializer<SimpleColorShader::VertexInput>([width, height](SimpleColorShader::VertexInput* ptr) {
		const float offset = 0.025f;
		for (int i = 1; i < width; ++i) {
			//lb rb lt rt
			ptr[0] = {Vector4F {i-offset,height,0,1}};
			ptr[1] = {Vector4F {i+offset,height,0,1}};
			ptr[2] = {Vector4F {i-offset,0,0,1}};
			ptr[3] = {Vector4F {i+offset,0,0,1}};
			ptr += 4;
		}
		for (int j = 1; j < height; ++j) {
			//lb rb lt rt

			ptr[0] = {Vector4F {0,j+offset,0,1}};
			ptr[1] = {Vector4F {width,j+offset,0,1}};
			ptr[2] = {Vector4F {0,j-offset,0,1}};
			ptr[3] = {Vector4F {width,j-offset,0,1}};
			ptr += 4;
		}
	}, alloccount);
	quadIndexBuffer.ensureLength((width - 1) + (height - 1));
//allocate at least 1 vertex, zero will throw an error on width == 1 && height == 1
	if (!gridBuffer->isInitialized()) {
		gridBuffer->initialize(BufferType::IMMUTABLE);
	}
}

void LevelEditorLayer::resetLevel() {
	lastModification = lastModificationSave = 0;
	Level res;
	res.getInfo().title = "Custom level";
	res.resize(8, 8);
	res.resetState();
	middle = res.getSize() / 2.0f;
	level = util::move(res);

	setYamYamRemainderCount(0);

	updateGrid();
}

void LevelEditorLayer::setYamYamRemainderCount(unsigned int count) {
	ASSERT(count <= SAPPHIRE_MAX_YAMYAMREMAINDER_COUNT) << count;
	unsigned int oldcount = level.getYamYamRemainderCount();
	level.setYamYamRemainderCount(count);
	if (count == 0) {
		yamyamLevel.resize(1, 1);
		yamyamLevel.setObject(0, 0, SapphireObject::Air);
	} else {
		//this keeps the old values
		yamyamLevel.resize(count * 3 + (count - 1), 3);
	}
	checkMapScrollValue();
}

void LevelEditorLayer::onSettingsChanged(const SapphireSettings& settings) {
	SapphireArtStyle artstyle;
	if (settings.is3DArtStyle()) {
		artstyle = SapphireArtStyle::ORTHO_3D;
	} else {
		artstyle = SapphireArtStyle::RETRO_2D;
	}

	auto* tbd = toolbarDrawer.setDrawer(artstyle);
	auto* dd = drawer.setDrawer(artstyle);
	auto* yyd = yamyamDrawer.setDrawer(artstyle);
	if (artstyle == SapphireArtStyle::ORTHO_3D) {
		static_cast<LevelDrawer3D*>(tbd)->setFixedLighting(Size2F(toolbarLevel.getSize()) / 2.0f);
	}

	auto* ss = static_cast<SapphireScene*>(getScene());
	drawer.setSize(ss->getGameSize(), ss->getGameUserScale());
	yamyamDrawer.setSize(ss->getGameSize(), ss->getGameUserScale());

	auto oldobjsize = toolbarDrawer.getObjectSize();
	toolbarDrawer.setSize(ss->getUiSize(), ss->getUiUserScale());

	resetToolbarMiddle();

	toolbarScroll += Vector2F { toolbarLevel.getWidth(), -(int) toolbarLevel.getHeight() } / 2.0f;
	toolbarScroll = toolbarScroll * oldobjsize / toolbarDrawer.getObjectSize();
	toolbarScroll -= Vector2F { toolbarLevel.getWidth(), -(int) toolbarLevel.getHeight() } / 2.0f;

	checkToolbarScrollValue();
	checkMapScrollValue();
}

class DemoCommandDialogItem: public CommandDialogItem {
	LevelEditorLayer* editor;
	const Demo* demo;
public:

	template<typename Handler>
	DemoCommandDialogItem(rhfw::FixedString text, Handler&& handler, LevelEditorLayer* editor, const Demo* demo)
			: CommandDialogItem(1.0f, util::move(text), util::forward<Handler>(handler)), editor(editor), demo(demo) {
	}

	virtual bool onKeyEvent() {
		switch (KeyEvent::instance.getKeycode()) {
			case KeyCode::KEY_DELETE: {
				if (KeyEvent::instance.getAction() != KeyAction::DOWN) {
					break;
				}
				DialogLayer* layer = new DialogLayer(dialog);
				layer->setTitle(demo->info.title);
				layer->addDialogItem(new TextDialogItem("Are you sure you want to delete this demo? "));
				layer->addDialogItem(new EmptyDialogItem(0.5f));
				layer->addDialogItem(new CommandDialogItem("No", [=] {
					layer->dismiss();
				}));
				layer->addDialogItem(new CommandDialogItem("Yes", [=] {
					layer->dismiss();
					editor->removeDemo(demo);
					if(editor->getLevel().getDemoCount() == 0) {
						dialog->addDialogItemFront(new TextDialogItem("No demos recorded."));
					}
					dialog->removeDialogItem(this);
					delete this;
				}));
				layer->show(dialog->getScene(), true);
				break;
			}
			default: {
				return false;
			}
		}
		return true;
	}
};

void LevelEditorLayer::showDemosDialog() {
	DialogLayer* layer = new DialogLayer(this);
	layer->setTitle("Level demos");
	if (level.getDemoCount() > 0) {
		for (unsigned int i = 0; i < level.getDemoCount(); ++i) {
			auto* d = level.getDemo(i);
			layer->addDialogItem(new DemoCommandDialogItem(FixedString { "View: " } + d->info.title, [=] {
				layer->dismiss();

				level.resetState();

				auto* demolayer = new DemoReplayerLayer(this, this->descriptor, level, i);
				demolayer->setSelectorLayer(selectorLayer);
				demolayer->setLevelEditorLayer(this);
				demolayer->show(getScene());
			}, this, d));
		}
	} else {
		layer->addDialogItem(new TextDialogItem("No demos recorded."));
	}
	layer->addDialogItem(new EmptyDialogItem(0.5f));
	layer->addDialogItem(new CommandDialogItem("Record new demo", [=] {
		level.resetState();

		if (level.getMinersTotal() == 0) {
			showMinerMissingDialog(layer);
		} else {
			layer->dismiss();
			auto* reclayer = new DemoRecorderLayer(this, this->descriptor, level);
			reclayer->setSelectorLayer(selectorLayer);
			reclayer->setLevelEditorLayer(this);
			reclayer->show(getScene());
		}
	}));
	layer->show(getScene(), true);
}

void LevelEditorLayer::addDemo(Demo* demo) {
	level.addDemo(demo);
	++lastModification;
}
void LevelEditorLayer::removeDemo(unsigned int demoindex) {
	level.removeDemo(demoindex);
	++lastModification;
}
void LevelEditorLayer::removeDemo(const Demo* demo) {
	level.removeDemo(demo);
	++lastModification;
}

bool LevelEditorLayer::tryPutObject(const Vector2F& point, Vector2UI* coordout, Level** levelout) {
	Vector2UI coords = drawer.getCoordinatesForPoint(point, middle);
	if (coords == *coordout) {
		return false;
	}
	if (putObject(coords)) {
		*coordout = coords;
		*levelout = &level;
		return true;
	}
	Vector2UI yamcoords = yamyamDrawer.getCoordinatesForPoint(point, getYamYamMiddle());
	if (putYamYamObject(yamcoords)) {
		if (yamcoords == *coordout) {
			return false;
		}
		*coordout = yamcoords;
		*levelout = &yamyamLevel;
		return true;
	}
	return false;
}
bool LevelEditorLayer::tryPutObject(const Vector2F& point, Vector2UI* coordout) {
	Level* level;
	return tryPutObject(point, coordout, &level);
}

bool LevelEditorLayer::putObject(const Vector2UI& coords) {
	if (coords.x() < level.getWidth() && coords.y() < level.getHeight()) {
		++lastModification;
		putObjectTo(level, coords.x(), coords.y(), *selectedObject);
		return true;
	}
	return false;
}

bool LevelEditorLayer::putYamYamObject(const Vector2UI& yamcoords) {
	if (yamcoords.x() < yamyamLevel.getWidth() && yamcoords.y() < yamyamLevel.getHeight()) {
		if ((yamcoords.x()) % 4 != 3) {
			putObjectTo(yamyamLevel, yamcoords.x(), yamcoords.y(), *selectedObject);
			level.getYamYamRemainders()[9 * (yamcoords.x() / 4) + (2 - yamcoords.y()) * 3 + (yamcoords.x() % 4)] =
					selectedObject->mapToIdentifier();
			++lastModification;
			return true;
		}
	}
	return false;
}
} // namespace userapp

