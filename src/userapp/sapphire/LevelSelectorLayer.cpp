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
 * LevelSelectorLayer.cpp
 *
 *  Created on: 2016. apr. 22.
 *      Author: sipka
 */

#include <framework/animation/Animation.h>
#include <framework/animation/PropertyAnimator.h>
#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <framework/geometry/Matrix.h>
#include <framework/io/key/KeyEvent.h>
#include <framework/io/touch/TouchEvent.h>
#include <framework/layer/Layer.h>
#include <framework/render/Renderer.h>
#include <framework/utils/LifeCycleChain.h>
#include <gen/types.h>
#include <gen/log.h>
#include <sapphire/dialogs/LevelDetailsLayer.h>
#include <sapphire/LevelSelectorLayer.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/SapphireBackgroundLayer.h>
#include <sapphire/DifficultySelectorLayer.h>
#include <sapphire/PlayerLayer.h>

#include <cstring>

namespace userapp {

#define MAX_TITLE_LENGTH 12

const char* DIFFICULTY_STRINGS[] { "Tutorial", "Simple", "Easy", "Moderate", "Normal", "Tricky", "Tough", "Difficult", "Hard", "M.A.D.",
		"Unrated", "All" };
const char* CATEGORY_STRINGS[] { "Fun", "Discovery", "Action", "Battle", "Puzzle", "Science", "Work", "None" };

#define SELECTED_COLOR_BLACK Color { 0, 0, 0, 1 }
#define SELECTED_COLOR_WHITE Color { 1, 1, 1, 1 }
#define SELECTED_COLOR_BLACK_INDEX 0
#define SELECTED_COLOR_WHITE_INDEX 1

static const Color DIFFICULTY_COLORS[] { //
//
Color { 87, 179, 255, 255 } / 255.0f, //
Color { 0, 135, 255, 255 } / 255.0f, //
Color { 16, 64, 255, 255 } / 255.0f, //
Color { 32, 136, 32, 255 } / 255.0f, //
Color { 50, 253, 0, 255 } / 255.0f, //
Color { 252, 248, 0, 255 } / 255.0f, //
Color { 254, 137, 0, 255 } / 255.0f, //
Color { 254, 50, 0, 255 } / 255.0f, //
Color { 253, 1, 1, 255 } / 255.0f, //
Color { 160, 0, 0, 255 } / 255.0f, //
Color { 160, 96, 0, 255 } / 255.0f, //
Color { 192, 192, 192, 255 } / 255.0f //
};
static const Color DIFFICULTY_SELECTED_COLORS[] { //
//
SELECTED_COLOR_BLACK,
SELECTED_COLOR_BLACK,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_BLACK,
SELECTED_COLOR_BLACK,
SELECTED_COLOR_BLACK,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_WHITE,
SELECTED_COLOR_BLACK,

};
static const rhfw::ResId DIFFICULTY_ANIMATIONS[] { ResIds::build::sipka_rh_texture_convert::_1man_anim,
		ResIds::build::sipka_rh_texture_convert::doorblue_anim, ResIds::build::sipka_rh_texture_convert::sapphire_anim,
		ResIds::build::sipka_rh_texture_convert::emerald_anim, ResIds::build::sipka_rh_texture_convert::swamp_anim,
		ResIds::build::sipka_rh_texture_convert::citrine_anim, ResIds::build::sipka_rh_texture_convert::yamyam_anim,
		ResIds::build::sipka_rh_texture_convert::ruby_anim, ResIds::build::sipka_rh_texture_convert::bomb_anim,
		ResIds::build::sipka_rh_texture_convert::tickbomb_anim, ResIds::build::sipka_rh_texture_convert::bag_anim,
		ResIds::build::sipka_rh_texture_convert::box, };

rhfw::ResId difficultyToAnimation(rhfw::SapphireDifficulty diff) {
	ASSERT(diff <= SapphireDifficulty::_count_of_entries);
	return DIFFICULTY_ANIMATIONS[(unsigned int) diff];
}
const char* difficultyToString(SapphireDifficulty diff) {
	ASSERT(diff <= SapphireDifficulty::_count_of_entries);
	return DIFFICULTY_STRINGS[(unsigned int) diff];
}
const char* categoryToString(SapphireLevelCategory cat) {
	ASSERT(cat < SapphireLevelCategory::_count_of_entries);
	return CATEGORY_STRINGS[(unsigned int) cat];
}
Color difficultyToColor(rhfw::SapphireDifficulty diff) {
	ASSERT(diff <= SapphireDifficulty::_count_of_entries);
	return DIFFICULTY_COLORS[(unsigned int) diff];
}
Color difficultyToSelectedColor(rhfw::SapphireDifficulty diff) {
	ASSERT(diff <= SapphireDifficulty::_count_of_entries);
	return DIFFICULTY_SELECTED_COLORS[(unsigned int) diff];
}

class BasicPartition: public LevelSelectorLayer::LevelPartition {
private:
	using LevelPtr = const SapphireLevelDescriptor*;
private:
	LevelPtr* levels = nullptr;
	unsigned int levelCount = 0;

	void init(FilterFunction filter, void* param) {
		unsigned int count = 0;
		//don't call filters in this loop, we can deal with this memory plus
		scene->iterateEveryLevel([&](const SapphireLevelDescriptor* desc) {
			if (includeLevel(desc)) {
				++count;
			}
		});

		levels = new LevelPtr[count];
		levelCount = 0;
		unsigned int ptr = 0;
		scene->iterateEveryLevel([&](const SapphireLevelDescriptor* desc) {
			if (includeLevel(desc) && filter(desc, param)) {
				levels[ptr++] = desc;
				++levelCount;
			}
		});
	}
protected:
	SapphireScene* scene;

	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) = 0;
public:
	BasicPartition(SapphireScene* scene)
			: scene(scene) {
	}
	~BasicPartition() {
		delete[] levels;
	}

	virtual void invalidateData() override {
		delete[] levels;
		levels = nullptr;
		levelCount = 0;
	}

	virtual bool initialized(FilterFunction func, void* param) override {
		if (levels != nullptr) {
			return true;
		}
		if (!scene->isLevelsLoaded()) {
			return false;
		}
		init(func, param);
		return true;
	}
	virtual bool descriptorRemoved(const SapphireLevelDescriptor* desc) override {
		int idx = getIndex(desc);
		if (idx < 0) {
			return false;
		}
		memmove(levels + idx, levels + idx + 1, sizeof(LevelPtr) * (levelCount - idx - 1));
		--levelCount;
		return true;
	}

	virtual unsigned int getCount() override {
		ASSERT(levels != nullptr);
		return levelCount;
	}
	virtual const SapphireLevelDescriptor* get(unsigned int index) override {
		ASSERT(levels != nullptr);
		ASSERT(index < getCount());
		return levels[index];
	}

	virtual bool applyFilter(FilterFunction func, void* param) override {
		if (!scene->isLevelsLoaded()) {
			return false;
		}
		invalidateData();
		init(func, param);
		return true;
	}
};
class DifficultyPartition: public BasicPartition {
protected:
	SapphireDifficulty difficulty;
	unsigned int playerCount;
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return (difficulty >= SapphireDifficulty::_count_of_entries || descriptor->difficulty == difficulty)
				&& descriptor->playerCount == playerCount;
	}
public:
	DifficultyPartition(SapphireScene* scene, SapphireDifficulty difficulty, unsigned int playercount)
			: BasicPartition(scene), difficulty(difficulty), playerCount(playercount) {
	}
};
class ClassicDifficultyPartition: public DifficultyPartition {
protected:
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return DifficultyPartition::includeLevel(descriptor) && !descriptor->communityLevel;
	}
public:
	using DifficultyPartition::DifficultyPartition;

	virtual const char* getTitle() override {
		return "Classic levels";
	}
};
class ExpansionDifficultyPartition: public DifficultyPartition {
protected:
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return DifficultyPartition::includeLevel(descriptor) && descriptor->communityLevel && !descriptor->serverSideAvailable
				&& !descriptor->isEditable() && descriptor->levelPack == nullptr;
	}
public:
	using DifficultyPartition::DifficultyPartition;

	virtual const char* getTitle() override {
		return "Expansion levels";
	}
};
class CommunityDifficultyPartition: public DifficultyPartition {
protected:
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return DifficultyPartition::includeLevel(descriptor) && descriptor->communityLevel && descriptor->serverSideAvailable
				&& descriptor->levelPack == nullptr;
	}
public:
	using DifficultyPartition::DifficultyPartition;

	virtual const char* getTitle() override {
		return "Community levels";
	}
};
class EditableDifficultyPartition: public DifficultyPartition {
protected:
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return DifficultyPartition::includeLevel(descriptor) && descriptor->communityLevel && !descriptor->serverSideAvailable
				&& descriptor->isEditable() && descriptor->levelPack == nullptr;
	}
public:
	using DifficultyPartition::DifficultyPartition;

	virtual const char* getTitle() override {
		return "My levels";
	}
};
class LevelPackDifficultyPartition: public DifficultyPartition {
protected:
	const char* levelPackName;
	virtual bool includeLevel(const SapphireLevelDescriptor* descriptor) override {
		return DifficultyPartition::includeLevel(descriptor) && descriptor->levelPack == levelPackName;
	}
public:
	LevelPackDifficultyPartition(SapphireScene* scene, SapphireDifficulty difficulty, unsigned int playercount, const char* levelpackname)
			: DifficultyPartition(scene, difficulty, playercount), levelPackName(levelpackname) {
	}

	virtual const char* getTitle() override {
		return levelPackName;
	}
};

LevelSelectorLayer::LevelSelectorLayer(SapphireUILayer* parent, SapphireDifficulty diff, unsigned int playercount)
		: SapphireUILayer(parent), titleText { difficultyToString(diff) } {
	this->returnText = "Return to levels";

	setupDefaultPartitions(static_cast<SapphireScene*>(parent->getScene()), diff, playercount);
}
LevelSelectorLayer::LevelSelectorLayer(SapphireUILayer* parent)
		: SapphireUILayer(parent) {
	this->returnText = "Return to levels";
}

void LevelSelectorLayer::drawImpl(float displayPercent) {
	renderer->setDepthTest(false);
	renderer->initDraw();

	auto* ss = static_cast<SapphireScene*>(getScene());

	auto size = ss->getUiSize();
	const auto mvp = Matrix2D { }.setScreenDimension(size.pixelSize);

	float alpha = displayPercent * displayPercent;

	fontDrawerPool.prepare(font, mvp);
	fontDrawer.prepare(4096);

	if (backPressed) {
		drawRectangleColor(mvp, Color { getUiColor().xyz(), alpha }, backButtonPos);
	}
	drawSapphireTexture(mvp, backIconWhite, backPressed ? Color { getUiSelectedColor().xyz(), 1 } : Color { getUiColor().xyz(), alpha },
			backButtonPos, Rectangle { 0, 0, 1, 1 });

	fontDrawer.add(titleText, Color { getUiColor().xyz(), alpha }, Vector2F { backButtonPos.right, backButtonPos.middle().y() },
			titleTextSize, Gravity::CENTER_VERTICAL | Gravity::LEFT);

	if (!checkForLevels()) {
		//XXX draw something about loading levels?
		fontDrawerPool.commit();
		return;
	}

	Vector2F itempos { gridRect.left + scrollDetector.getPosition().x(), gridRect.top };

	auto titlesize = gridItemSize.height() * 0.9f;

	float infotexty = titlesize / 2.0f;

	if (ss->getBackgroundLayer()->isPreviewMode()) {
		fontDrawer.add("Preview mode", Color { getUiColor().xyz(), alpha },
				Vector2F { size.pixelSize.width() - titlesize / 2.0f, infotexty }, gridItemSize.height(), Gravity::TOP | Gravity::RIGHT);
		infotexty += gridItemSize.height();
	}
	if (filterLength > 0) {
		char quoted[FILTER_MAX_LENGTH + 2 + 1];
		quoted[0] = '\"';
		memcpy(quoted + 1, filterData, filterLength);
		quoted[filterLength + 1] = '\"';
		quoted[filterLength + 2] = 0;
		fontDrawer.add(quoted, Color { getUiColor().xyz(), alpha }, Vector2F { size.pixelSize.width() - titlesize / 2.0f, infotexty },
				gridItemSize.height(), Gravity::TOP | Gravity::RIGHT);
		infotexty += gridItemSize.height();
	}
	if (gridColumnCount == 0) {
		fontDrawer.add(filterLength > 0 ? "No level matches filter" : "No levels available", Color { getUiColor().xyz(), alpha },
				size.pixelSize / 2.0f, gridItemSize.height() * 1.6f, Gravity::CENTER);
	} else {
		for (auto&& p : partitions) {
			auto colcount = getPartitionColumnCount(p, gridRowCount);
			if (colcount == 0) {
				continue;
			}
			unsigned int row = 0;
			itempos.y() = gridRect.top;
			Vector2F parttitlepos;
			parttitlepos.x() = min(max(gridRect.left, itempos.x()), itempos.x() + (colcount - 1) * gridItemSize.width());
			parttitlepos.y() = itempos.y() + gridItemSize.height() / 2.0f;
			fontDrawer.add(p->getTitle(), Color { getUiColor().xyz(), alpha }, parttitlepos, titlesize,
					Gravity::CENTER_VERTICAL | Gravity::LEFT);

			itempos.y() += gridItemSize.height();

			auto pcount = p->getCount();

			for (unsigned int i = 0; i < pcount;) {
				if (itempos.x() + gridItemSize.width() < 0.0f) {
					i += gridRowCount;
					itempos.x() += gridItemSize.width();
					continue;
				}
				auto* desc = p->get(i);
				++i;

				auto&& selectedcolor = difficultyToSelectedColor(desc->difficulty);
				auto&& diffcolor = difficultyToColor(desc->difficulty);

				bool selected = desc == this->selected.descriptor && p == this->selected.partition;
				if (selected) {
					drawRectangleColor(mvp, Color { diffcolor.rgb(), alpha }, Rectangle { itempos, itempos + gridItemSize });
				}

				auto titlecolor = Color { (selected ? selectedcolor : diffcolor).rgb(), alpha };

				float markerwidth = gridItemSize.height() * 2 / 3;
				float markersPaddingX = gridItemSize.height() * 0.05f;

				if (desc->state == LevelState::UNSEEN || desc->state == LevelState::COMPLETED) {
					auto& elem = markers->getAtIndex(
							(unsigned int) (
									desc->state == LevelState::COMPLETED ?
											SapphireMarkerAtlas::MARKER_TICK : SapphireMarkerAtlas::MARKER_QUESTION));
					drawSapphireTexture(mvp, elem, titlecolor,
							Rectangle { itempos, Vector2F { itempos.x() + markerwidth, itempos.y() + gridItemSize.height() } }.inset(
									markersPaddingX).fitInto(Rectangle { 0, 0, 14, 18 }), elem.getPosition());
				}

				if (desc->hasSuspendedGame) {
					auto& elem = markers->getAtIndex((unsigned int) SapphireMarkerAtlas::MARKER_SUSPEND);
					drawSapphireTexture(mvp, elem, titlecolor,
							Rectangle { itempos.x() + markerwidth, itempos.y(), itempos.x() + markerwidth * 2, itempos.y()
									+ gridItemSize.height() }.fitInto(Rectangle { 0, 0, 17, 23 }).inset(markersPaddingX),
							elem.getPosition());
				} else {
					drawRectangleColor(mvp, titlecolor,
							Rectangle { 0, 0, gridItemSize.height() / 5.0f, gridItemSize.height() / 5.0f }.centerInto(
									Rectangle { itempos.x() + markerwidth + markersPaddingX, itempos.y(), itempos.x() + markerwidth * 2
											+ markersPaddingX, itempos.y() + gridItemSize.height() }));
				}
				if (desc->category != SapphireLevelCategory::None) {
					float xpos = itempos.x() + markerwidth * 2;
					auto& elem = markers->getAtIndex((unsigned int) SapphireMarkerAtlas::CATEGORY_START + (unsigned int) desc->category);
					drawSapphireTexture(mvp, elem, titlecolor,
							Rectangle { xpos, itempos.y(), xpos + markerwidth, itempos.y() + gridItemSize.height() }.fitInto(Rectangle { 0,
									0, 17, 23 }).inset(markersPaddingX), elem.getPosition());
				}
				float itemleftpadding = markerwidth * 3;
				auto titlepos = Vector2F { itempos.x() + itemleftpadding, itempos.y() + gridItemSize.height() / 2.0f };
				const char* end = font->measureText(desc->title.begin(), titlesize, gridItemSize.width() - itemleftpadding);
				if (*end != 0) {
					if (selected) {
						float add = fontDrawer.add(desc->title.begin(), end - 2, titlecolor, titlepos, titlesize,
								Gravity::CENTER_VERTICAL | Gravity::LEFT);
						fontDrawer.add("..", titlecolor, Vector2F { titlepos.x() + add, titlepos.y() }, titlesize,
								Gravity::CENTER_VERTICAL | Gravity::LEFT);
					} else {
						float add = fontDrawer.add(desc->title.begin(), end - 2, titlecolor, titlepos, titlesize,
								Gravity::CENTER_VERTICAL | Gravity::LEFT);
						fontDrawer.add("..", titlecolor, Vector2F { titlepos.x() + add, titlepos.y() }, titlesize,
								Gravity::CENTER_VERTICAL | Gravity::LEFT);
					}
				} else {
					if (selected) {
						fontDrawer.add(desc->title, titlecolor, titlepos, titlesize, Gravity::CENTER_VERTICAL | Gravity::LEFT);
					} else {
						fontDrawer.add(desc->title, titlecolor, titlepos, titlesize, Gravity::CENTER_VERTICAL | Gravity::LEFT);
					}
				}

				++row;
				if (row == gridRowCount) {
					row = 0;
					itempos.y() = gridRect.top + gridItemSize.height();
					itempos.x() += gridItemSize.width();
					if (itempos.x() >= size.pixelSize.width()) {
						break;
					}
				} else {
					itempos.y() += gridItemSize.height();
				}
			}
			if (row != 0) {
				itempos.x() += gridItemSize.width();
			}
		}
	}

	fontDrawerPool.commit();
}

LevelSelectorLayer::LevelPartition* LevelSelectorLayer::getNextNonEmptyPartition(int index) {
	for (unsigned int i = index + 1; i < partitions.size(); ++i) {
		if (partitions.get(i)->getCount() != 0) {
			return partitions.get(i);
		}
	}
	return nullptr;
}
LevelSelectorLayer::LevelPartition* LevelSelectorLayer::getPrevNonEmptyPartition(unsigned int index) {
	for (unsigned int i = index - 1; i < partitions.size(); --i) {
		if (partitions.get(i)->getCount() != 0) {
			return partitions.get(i);
		}
	}
	return nullptr;
}
template<>
void LevelSelectorLayer::moveSelection<SapphireDirection::Left>() {
	int descriptorindex = selected.partition->getIndex(selected.descriptor);
	ASSERT(descriptorindex >= 0);
	if (descriptorindex >= gridRowCount) {
		setSelectedLevel(SelectedItem { selected.partition, selected.partition->get(descriptorindex - gridRowCount) });
	} else {
		int partindex = partitions.indexOf(selected.partition);
		ASSERT(partindex >= 0);

		LevelPartition* prev = getPrevNonEmptyPartition(partindex);
		if (prev != nullptr) {
			unsigned int pcount = prev->getCount();
			unsigned int index = pcount - pcount % gridRowCount + descriptorindex;
			if (index >= pcount) {
				unsigned int lastpagecount;
				if (pcount % gridRowCount == 0) {
					lastpagecount = gridRowCount;
				} else {
					lastpagecount = pcount % gridRowCount;
				}
				if (descriptorindex >= lastpagecount) {
					index = pcount - 1;
				} else {
					index = pcount - lastpagecount + descriptorindex;
				}
			}
			setSelectedLevel(SelectedItem { prev, prev->get(index) });
		}
	}
}
template<>
void LevelSelectorLayer::moveSelection<SapphireDirection::Right>() {
	int descriptorindex = selected.partition->getIndex(selected.descriptor);
	ASSERT(descriptorindex >= 0);

	if (descriptorindex + gridRowCount < selected.partition->getCount()) {
		setSelectedLevel(SelectedItem { selected.partition, selected.partition->get(descriptorindex + gridRowCount) });
	} else if (selected.partition->getCount() > (descriptorindex / gridRowCount + 1) * gridRowCount) {
		//can move at least one column to the right, but not directly right
		//select the last descriptor
		setSelectedLevel(SelectedItem { selected.partition, selected.partition->get(selected.partition->getCount() - 1) });
	} else {
		int partindex = partitions.indexOf(selected.partition);
		ASSERT(partindex >= 0);

		LevelPartition* next = getNextNonEmptyPartition(partindex);
		if (next != nullptr) {
			unsigned int pcount = next->getCount();
			unsigned int index = descriptorindex % gridRowCount;
			if (index >= pcount) {
				index = pcount - 1;
			}
			setSelectedLevel(SelectedItem { next, next->get(index) });
		}
	}
}
template<>
void LevelSelectorLayer::moveSelection<SapphireDirection::Up>() {
	int descriptorindex = selected.partition->getIndex(selected.descriptor);
	ASSERT(descriptorindex >= 0);
	if (descriptorindex > 0) {
		setSelectedLevel(SelectedItem { selected.partition, selected.partition->get(descriptorindex - 1) });
	} else {
		int partindex = partitions.indexOf(selected.partition);
		ASSERT(partindex >= 0);

		LevelPartition* prev = getPrevNonEmptyPartition(partindex);
		if (prev != nullptr) {
			setSelectedLevel(SelectedItem { prev, prev->get(prev->getCount() - 1) });
		}
	}
}
template<>
void LevelSelectorLayer::moveSelection<SapphireDirection::Down>() {
	int descriptorindex = selected.partition->getIndex(selected.descriptor);
	ASSERT(descriptorindex >= 0);
	if (descriptorindex + 1 < selected.partition->getCount()) {
		setSelectedLevel(SelectedItem { selected.partition, selected.partition->get(descriptorindex + 1) });
	} else {
		int partindex = partitions.indexOf(selected.partition);
		ASSERT(partindex >= 0);

		LevelPartition* prev = getNextNonEmptyPartition(partindex);
		if (prev != nullptr) {
			setSelectedLevel(SelectedItem { prev, prev->get(0) });
		}
	}
}
/**
 * Maps printable ascii characters to their lowecase equivalents
 */
static const char ASCII_LOWERCASE_MAP[256] { //
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0,		//
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
		97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 91,
		92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125,
		126,		//
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

};
bool LevelSelectorLayer::FilterFunction(const SapphireLevelDescriptor* desc, void* param) {
	LevelSelectorLayer* thiz = reinterpret_cast<LevelSelectorLayer*>(param);
	char title[SAPPHIRE_LEVEL_TITLE_MAX_LEN + 1];
	unsigned int len = desc->title.length();
	for (unsigned int i = 0; i < len; ++i) {
		title[i] = ASCII_LOWERCASE_MAP[(unsigned char) desc->title[i]];
	}
	title[len] = 0;

	return strstr(title, thiz->filterData) != nullptr;
}
void LevelSelectorLayer::addFilterChar(UnicodeCodePoint cp) {
	if (filterLength >= FILTER_MAX_LENGTH) {
		return;
	}
	filterData[filterLength++] = ASCII_LOWERCASE_MAP[(unsigned char) cp];
	filterData[filterLength] = 0;

	reloadLevels();
	setSelectedLevel(lastSelected);
	animateToItem(selected);
}
void LevelSelectorLayer::addFilterChars(UnicodeCodePoint cp, unsigned int count) {
	if (filterLength >= FILTER_MAX_LENGTH) {
		return;
	}
	while (count > 0 && filterLength < FILTER_MAX_LENGTH) {
		filterData[filterLength++] = ASCII_LOWERCASE_MAP[(unsigned char) cp];
		--count;
	}
	filterData[filterLength] = 0;
	reloadLevels();
	setSelectedLevel(lastSelected);
	animateToItem(selected);
}

#define BREAK_ON_NOT_DOWN()  if (KeyEvent::instance.getAction() != KeyAction::DOWN || gridColumnCount == 0) break
bool LevelSelectorLayer::onKeyEventImpl() {
	if (KeyEvent::instance.getAction() == KeyAction::UNICODE_SEQUENCE) {
		unsigned int len = KeyEvent::instance.getUnicodeSequenceLength();
		const UnicodeCodePoint* seq = KeyEvent::instance.getUnicodeSequence();
		for (unsigned int i = 0; i < len; ++i) {
			UnicodeCodePoint cp = seq[i];
			if (cp >= 32 && cp <= 126) {
				//only ascii
				addFilterChar(cp);
			}
		}
	} else if (KeyEvent::instance.getAction() == KeyAction::UNICODE_REPEAT) {
		const UnicodeCodePoint cp = KeyEvent::instance.getUnicodeRepeat();
		if (cp >= 32 && cp <= 126) {
			unsigned int count = KeyEvent::instance.getUnicodeRepeatCount();
			addFilterChars(cp, count);
		}
		return true;
	} else {
		switch (KeyEvent::instance.getKeycode()) {
			case KeyCode::KEY_GAMEPAD_LEFT_TRIGGER:
			case KeyCode::KEY_F1: {
				if (KeyEvent::instance.getAction() != KeyAction::UP || gridColumnCount == 0)
					break;
				auto* ss = static_cast<SapphireScene*>(getScene());
				bool preview = !ss->getBackgroundLayer()->isPreviewMode();
				ss->getBackgroundLayer()->setPreviewMode(preview);
				if (preview) {
					//preview turned off
					if (selected != nullptr) {
						ss->getBackgroundLayer()->setDisplayLevel(selected.descriptor);
					}
				} else {
					auto* ss = static_cast<SapphireScene*>(getScene());
					ss->getBackgroundLayer()->setDisplayLevel(nullptr);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_LEFT:
			case KeyCode::KEY_DIR_LEFT: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					moveSelection<SapphireDirection::Left>();
					animateToItem(selected);
				}

				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_RIGHT:
			case KeyCode::KEY_DIR_RIGHT: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					moveSelection<SapphireDirection::Right>();
					animateToItem(selected);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_UP:
			case KeyCode::KEY_DIR_UP: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					moveSelection<SapphireDirection::Up>();
					animateToItem(selected);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_DPAD_DOWN:
			case KeyCode::KEY_DIR_DOWN: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					moveSelection<SapphireDirection::Down>();
					animateToItem(selected);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_A:
			case KeyCode::KEY_ENTER: {
				BREAK_ON_NOT_DOWN();
				if(selected != nullptr) {
					if((KeyEvent::instance.getModifiers() & KeyModifiers::CTRL_ON_BOOL_MASK) != 0) {
						if((KeyEvent::instance.getModifiers() & KeyModifiers::SHIFT_ON_BOOL_MASK) != 0) {
							onLevelStartSuspendedSelected(selected.descriptor);
						} else {
							onLevelStartSelected(selected.descriptor);
						}
					} else {
						onLevelSelected(selected.descriptor);
					}
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_START: {
				BREAK_ON_NOT_DOWN();
				if(selected != nullptr) {
					onLevelStartSelected(selected.descriptor);
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_LEFT_SHOULDER:
			case KeyCode::KEY_HOME: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					int descriptorindex = selected.partition->getIndex(selected.descriptor);
					descriptorindex -= descriptorindex % gridRowCount;
					setSelectedLevel(SelectedItem {selected.partition, selected.partition->get(descriptorindex)});
				}
				break;
			}
			case KeyCode::KEY_GAMEPAD_RIGHT_SHOULDER:
			case KeyCode::KEY_END: {
				BREAK_ON_NOT_DOWN();
				if(selected == nullptr) {
					setSelectedLevel(lastSelected);
					animateToItem(selected);
				} else {
					int descriptorindex = selected.partition->getIndex(selected.descriptor);
					descriptorindex -= descriptorindex % gridRowCount;
					descriptorindex += gridRowCount-1;
					if(descriptorindex > selected.partition->getCount()) {
						descriptorindex = selected.partition->getCount() - 1;
					}
					setSelectedLevel(SelectedItem {selected.partition, selected.partition->get(descriptorindex)});

				}
				break;
			}
			case KeyCode::KEY_BACKSPACE: {
				if (KeyEvent::instance.getAction() != KeyAction::DOWN) {
					break;
				}
				if(filterLength > 0) {
					setFilterLength(filterLength-1);
				}
				break;
			}
			case KeyCode::KEY_ESC: {
				if (KeyEvent::instance.getAction() != KeyAction::DOWN) {
					break;
				}
				if(filterLength > 0) {
					setFilterLength(0);
				} else {
					return SapphireUILayer::onKeyEventImpl();
				}
				break;
			}
			default: {
				return SapphireUILayer::onKeyEventImpl();
			}
		}
	}
	return true;
}
void LevelSelectorLayer::setFilterLength(unsigned int length) {
	filterLength = length;
	reloadLevels();
	if (lastSelected == nullptr) {
		auto* p = getNextNonEmptyPartition(-1);
		if (p != nullptr) {
			lastSelected = SelectedItem { p, p->get(0) };
		}
	}
	setSelectedLevel(lastSelected);
	animateToItem(selected);
}
void LevelSelectorLayer::scrollTouch() {
	Vector2F middle = TouchEvent::instance.getCenter();
	if (TouchEvent::instance.isJustDown()) {
		scrollDownPos = middle;
	}
	bool applyscroll = TouchEvent::instance.getAction() == TouchAction::WHEEL || TouchEvent::instance.getAction() == TouchAction::SCROLL
			|| (scrollDownPos - middle).length() >= static_cast<SapphireScene*>(getScene())->getUiSize().toPixelsX(0.3f);
	scrollDetector.onTouch(applyscroll);
	if (applyscroll && TouchEvent::instance.getPointerCount() > 0) {
		clearSelected();
	}
}
LevelSelectorLayer::SelectedItem LevelSelectorLayer::getTouchedItem(const Vector2F& touchpos) {
	if (touchpos.y() >= 0.0f && touchpos.x() >= 0.0f && touchpos.y() < gridRowCount * gridItemSize.height()
			&& touchpos.x() < gridColumnCount * gridItemSize.width()) {
		unsigned int row = (unsigned int) (touchpos.y() / gridItemSize.height());
		unsigned int column = (unsigned int) (touchpos.x() / gridItemSize.width());
		for (auto&& p : partitions) {
			auto pcount = p->getCount();
			unsigned int offset = 0;
			while (pcount > 0) {
				if (column == 0) {
					if (row < pcount) {
						auto desc = p->get(offset + row);
						return SelectedItem { p, desc };
					} else {
						//dead zone
						return nullptr;
					}
				} else {
					--column;
					if (pcount > gridRowCount) {
						pcount -= gridRowCount;
						offset += gridRowCount;
					} else {
						break;
					}
				}
			}
		}
	}
	return nullptr;
}
bool LevelSelectorLayer::touchImpl() {
	if (!showing || !gainedInput || TouchEvent::instance.getAction() == TouchAction::CANCEL) {
		backPressed = false;
		clearSelected();
		return false;
	}

	scrollTouch();
	if (TouchEvent::instance.getPointerCount() <= 1) {
		switch (TouchEvent::instance.getAction()) {
			case TouchAction::DOWN: {
				clearSelected();
				backPressed = false;
				auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
				if (backButtonPos.isInside(touchpos)) {
					backPressed = true;
				} else if (gridColumnCount > 0) {
					touchpos -= gridRect.leftTop();
					touchpos.x() -= scrollDetector.getPosition().x();
					touchpos.y() -= gridItemSize.height();
					auto sel = getTouchedItem(touchpos);
					setSelectedLevel(sel);
				}
				break;
			}
			case TouchAction::MOVE_UPDATE: {
				auto touchpos = TouchEvent::instance.getAffectedPointer()->getPosition();
				if (backPressed && !backButtonPos.isInside(touchpos)) {
					backPressed = false;
				} else if (selected != nullptr) {
					touchpos -= gridRect.leftTop();
					touchpos.x() -= scrollDetector.getPosition().x();
					touchpos.y() -= gridItemSize.height();

					auto sel = getTouchedItem(touchpos);
					if (sel != selected) {
						selected = nullptr;
					}
				}
				break;
			}
			case TouchAction::UP: {
				if (backPressed) {
					backPressed = false;
					dismiss();
				} else if (selected != nullptr) {
					animateToItem(selected);
					onLevelSelected(selected);
					clearSelected();
				}
				break;
			}
			default: {
				break;
			}
		}
	} else {
		clearSelected();
	}

	return true;
}

void LevelSelectorLayer::animateToItem(const SelectedItem& selected) {
	if (selected == nullptr) {
		return;
	}
	unsigned int col = 0;
	for (auto&& p : partitions) {
		if (p == selected.partition) {
			int index = p->getIndex(selected.descriptor);
			ASSERT(index >= 0);
			col += index / gridRowCount;
			break;
		} else {
			col += getPartitionColumnCount(p, gridRowCount);
		}
	}
	startPageAnimator(col);
}

void LevelSelectorLayer::startPageAnimator(unsigned int page) {
	scrollDetector.animateTo(
			scrollDetector.getTargetScrollPosition(
					Rectangle { Vector2F { 0, 0 }, gridItemSize }.translate(Vector2F { gridItemSize.width() * page, 0 })));
}
void LevelSelectorLayer::setTitleText(FixedString title) {
	this->titleText = util::move(title);
	if (getScene() != nullptr) {
		auto size = static_cast<SapphireScene*>(getScene())->getUiSize();
		sizeChanged(size);
	}
}
void LevelSelectorLayer::sizeChanged(const rhfw::core::WindowSize& size) {
	float titleheight = size.pixelSize.height() * SAPPHIRE_TITLE_PERCENT;
	if (titleheight > size.toPixelsY(3.5f)) {
		titleheight = size.toPixelsY(3.5f);
	}
	float titlew = titleheight * 2 + font->measureText(titleText, titleheight);
	if (titlew > size.pixelSize.width() * 7.0f / 8.0f) {
		titleheight = titleheight / titlew * size.pixelSize.width() * 7.0f / 8.0f;
		titlew = size.pixelSize.width() * 7.0f / 8.0f;
	}
	titleTextSize = titleheight;
	backButtonPos = Rectangle { (size.pixelSize.width() - titlew) / 2.0f, 0, (size.pixelSize.width() - titlew) / 2.0f + titleheight,
			titleheight };

	gridLayoutDone = false;
}

bool LevelSelectorLayer::checkForLevels() {
	LevelPartition::FilterFunction filterfunc;
	if (filterLength > 0) {
		filterData[filterLength] = 0;
		filterfunc = FilterFunction;
	} else {
		filterfunc = LevelPartition::AcceptFilter;
	}
	for (auto&& p : partitions) {
		if (!p->initialized(filterfunc, this)) {
			return false;
		}
	}
	if (!gridLayoutDone) {
		gridLayoutDone = true;
		if (lastSelected == nullptr) {
			auto* p = getNextNonEmptyPartition(-1);
			if (p != nullptr) {
				lastSelected = SelectedItem { p, p->get(0) };
			}
		}
		layoutGrid(static_cast<SapphireScene*>(getScene())->getUiSize());
	}
	return true;
}
void LevelSelectorLayer::descriptorRemoved(const SapphireLevelDescriptor* desc) {
	int selectedindex = -1;
	LevelPartition* selectedpartition = nullptr;
	if (selected.descriptor == desc) {
		selectedpartition = selected.partition;
		selectedindex = selectedpartition->getIndex(desc);
		selected = nullptr;
	}
	if (lastSelected.descriptor == desc) {
		if (selectedpartition == nullptr) {
			selectedpartition = lastSelected.partition;
			selectedindex = selectedpartition->getIndex(desc);
		}
		lastSelected = nullptr;
	}
	bool relayout = false;
	for (auto&& p : partitions) {
		if (p->descriptorRemoved(desc)) {
			relayout = true;
		}
	}
	auto* ss = static_cast<SapphireScene*>(getScene());
	if (relayout) {
		layoutGrid(ss->getUiSize());
	}
	if (selectedindex >= 0) {
		if (selectedpartition->getCount() > 0) {
			if (selectedindex >= selectedpartition->getCount()) {
				selectedindex = selectedpartition->getCount() - 1;
			}
			setSelectedLevel(SelectedItem { selectedpartition, selectedpartition->get(selectedindex) });
		} else {
			//partition became empty
			int partindex = partitions.indexOf(selectedpartition);
			auto* npart = getPrevNonEmptyPartition(partindex);
			if (npart != nullptr) {
				setSelectedLevel(SelectedItem { npart, npart->get(npart->getCount() - 1) });
			} else {
				npart = getNextNonEmptyPartition(partindex);
				if (npart != nullptr) {
					setSelectedLevel(SelectedItem { npart, npart->get(0) });
				} else {
					setSelectedLevel(nullptr);
				}
			}
		}
		animateToItem(selected);
	}
}
void LevelSelectorLayer::clearPartitions() {
	selected = nullptr;
	lastSelected = nullptr;
	partitions.clear();
}
void LevelSelectorLayer::setupDefaultPartitions(SapphireScene* ss, SapphireDifficulty diff, unsigned int playercount, FixedString title) {
	clearPartitions();
	partitions.add(new ClassicDifficultyPartition(ss, diff, playercount));
	partitions.add(new ExpansionDifficultyPartition(ss, diff, playercount));
	partitions.add(new CommunityDifficultyPartition(ss, diff, playercount));
	partitions.add(new EditableDifficultyPartition(ss, diff, playercount));

	for (auto&& lp : ss->getLevelPackNames()) {
		partitions.add(new LevelPackDifficultyPartition(ss, diff, playercount, *lp));
	}

	gridLayoutDone = false;
	setColors(diff);
	setTitleText(util::move(title));
}
void LevelSelectorLayer::setupDefaultPartitions(SapphireScene* scene, SapphireDifficulty diff, unsigned int playercount) {
	setupDefaultPartitions(scene, diff, playercount, difficultyToString(diff));
}
void LevelSelectorLayer::displaySelection(const SapphireLevelDescriptor* desc) {
	if (!checkForLevels()) {
		return;
	}
	LevelPartition* test = selected.partition;
	if (test == nullptr) {
		test = lastSelected.partition;
	}
	if (test != nullptr) {
		int idx = test->getIndex(desc);
		if (idx >= 0) {
			setSelectedLevel(SelectedItem { test, test->get(idx) });
			animateToItem(selected);
			return;
		}
	}
	for (auto&& p : partitions) {
		int idx = p->getIndex(desc);
		if (idx >= 0) {
			setSelectedLevel(SelectedItem { p, p->get(idx) });
			animateToItem(selected);
			return;
		}
	}
	if (filterLength > 0) {
		//try with reloading levels
		filterLength = 0;
		reloadLevels();
		displaySelection(desc);
		return;
	}
	//set the default partitions
	setupDefaultPartitions(static_cast<SapphireScene*>(getScene()), desc->difficulty, desc->playerCount);
	displaySelection(desc);
}

void LevelSelectorLayer::setSelectedLevel(const SelectedItem& selected) {
	if (this->selected == selected) {
		return;
	}
	this->lastSelected = selected;
	this->selected = selected;
	auto* ss = static_cast<SapphireScene*>(getScene());
	if (ss->getBackgroundLayer()->isPreviewMode()) {
		if (selected != nullptr) {
			ss->getBackgroundLayer()->setDisplayLevel(selected.descriptor);
		} else {
			ss->getBackgroundLayer()->setDisplayLevel(nullptr);
			ss->getBackgroundLayer()->setPreviewMode(false);
		}
	}
}

unsigned int LevelSelectorLayer::getPartitionColumnCount(LevelPartition* p, unsigned int rowcount) {
	auto count = p->getCount();
	if (count == 0) {
		return 0;
	}
	unsigned int result = count / rowcount;
	if (count % rowcount != 0) {
		result++;
	}
	return result;
}
unsigned int LevelSelectorLayer::countColumns(unsigned int rowcount) {
	unsigned int result = 0;
	for (auto&& p : partitions) {
		result += getPartitionColumnCount(p, rowcount);
	}
	return result;
}
LevelSelectorLayer::LevelPartition* LevelSelectorLayer::findPartitionForDescriptor(const SapphireLevelDescriptor* desc) {
	for (auto&& p : partitions) {
		if (p->hasDescriptor(desc)) {
			return p;
		}
	}
	return nullptr;
}
void LevelSelectorLayer::layoutGrid(const rhfw::core::WindowSize& size) {
	gridRect = Rectangle { min(size.toPixelsX(0.75f), size.pixelSize.width() / 20.0f), titleTextSize * SAPPHIRE_TITLE_PADDING_PERCENT,
			(float) size.pixelSize.width(), (float) size.pixelSize.height() - size.toPixelsY(0.5f) };

	const int MIN_ROWS_A_COLUMN = 8;

	float maxheight = size.toPixelsY(0.8f);
	float minheight = gridRect.height() / MIN_ROWS_A_COLUMN;
	if (minheight < maxheight) {
		gridItemSize.height() = minheight;
		gridRowCount = MIN_ROWS_A_COLUMN;
	} else {
		gridItemSize.height() = maxheight;
		gridRowCount = (unsigned int) (gridRect.height() / maxheight);
	}
	//decrease row count by 1, as partition headers will go to the first line
	--gridRowCount;

	gridItemSize.width() = min(gridItemSize.height() * (MAX_TITLE_LENGTH + 1), gridRect.width() * 0.9f);
	gridColumnCount = countColumns(gridRowCount);

	scrollDetector.setSize(gridRect.widthHeight(),
			Vector2F { gridColumnCount * gridItemSize.width(), (gridRowCount + 1) * gridItemSize.height() });
	scrollDetector.setWheelMultiplier(Vector2F { gridItemSize.width(), 0.0f });
	if (selected != nullptr) {
		animateToItem(selected);
	}
}

void LevelSelectorLayer::onSceneSizeInitialized() {
	if (checkForLevels()) {
		auto* ss = static_cast<SapphireScene*>(getScene());
		auto* bg = ss->getBackgroundLayer();
		auto* desc = bg->getDescriptor();
		if (desc != nullptr && bg->isPreviewMode()) {
			LevelPartition* p = findPartitionForDescriptor(desc);
			if (p != nullptr) {
				lastSelected = SelectedItem { p, desc };
				animateToItem(lastSelected);
			}
		}
	}
}

void LevelSelectorLayer::showLevelLoadFailedDialog() {
	DialogLayer* info = new DialogLayer(this);
	info->setTitle("Loading failed");
	info->addDialogItem(
			new TextDialogItem("Failed to load level. Please make sure you have the latest version of " SAPPHIRE_GAME_NAME "."));
	info->addDialogItem(new EmptyDialogItem(0.5f));
	info->addDialogItem(new CommandDialogItem("Back", [=] {
		info->dismiss();
	}));
	info->show(getScene(), true);
}

void LevelSelectorLayer::setScene(Scene* scene) {
	SapphireUILayer::setScene(scene);
}

void LevelSelectorLayer::onLevelSelected(const SapphireLevelDescriptor* desc) {
	if (DifficultySelectorLayer::showLockedLevelDialog(this, desc->difficulty)) {
		return;
	}
	Level level;
	if (!level.loadLevel(desc->getFileDescriptor())) {
		showLevelLoadFailedDialog();
	} else {
		auto* layer = new LevelDetailsLayer(this, desc, util::move(level));
		layer->setSelectorLayer(this);
		layer->showDialog(getScene());
	}
}
void LevelSelectorLayer::onLevelSelected(const SelectedItem& selection) {
	onLevelSelected(selection.descriptor);
}
void LevelSelectorLayer::onLevelStartSelected(const SapphireLevelDescriptor* desc) {
	Level level;
	if (!level.loadLevel(desc->getFileDescriptor())) {
		showLevelLoadFailedDialog();
		return;
	}
	level.setRandomSeed((unsigned int) core::MonotonicTime::getCurrent());

	auto* player = new PlayerLayer(this, desc, util::move(level));
	player->setSelectorLayer(this);
	player->show(getScene());
}
void LevelSelectorLayer::onLevelStartSuspendedSelected(const SapphireLevelDescriptor* desc) {
	if (!desc->hasSuspendedGame) {
		onLevelStartSelected(desc);
		return;
	}
	auto* ss = static_cast<SapphireScene*>(getScene());
	Level level;
	if (!level.loadLevel(desc->getFileDescriptor()) || !ss->loadSuspendedLevel(desc, level)) {
		showLevelLoadFailedDialog();
		return;
	}
	auto* player = new PlayerLayer(this, desc, util::move(level));
	player->setSuspendedLevel();
	player->setSelectorLayer(this);
	player->show(getScene());
}

void LevelSelectorLayer::reloadLevels() {
	SelectedItem sel = selected;
	if (sel == nullptr) {
		sel = lastSelected;
	}
	for (auto&& p : partitions) {
		p->invalidateData();
	}
	selected = nullptr;
	lastSelected = nullptr;
	gridLayoutDone = false;

	if (!checkForLevels()) {
		return;
	}
	if (sel != nullptr) {
		int idx = sel.partition->getIndex(sel.descriptor);
		if (idx >= 0) {
			setSelectedLevel(SelectedItem { sel.partition, sel.partition->get(idx) });
			animateToItem(selected);
			return;
		}
		for (auto&& p : partitions) {
			int idx = p->getIndex(sel.descriptor);
			if (idx >= 0) {
				setSelectedLevel(SelectedItem { p, p->get(idx) });
				animateToItem(selected);
				return;
			}
		}
	}
}
}  // namespace userapp

