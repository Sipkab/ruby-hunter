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
 * LevelSelectorLayer.h
 *
 *  Created on: 2016. apr. 22.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVELSELECTORLAYER_H_
#define TEST_SAPPHIRE_LEVELSELECTORLAYER_H_

#include <framework/animation/Animation.h>
#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/FixedString.h>
#include <framework/utils/LifeCycleChain.h>
#include <framework/utils/LinkedList.h>
#include <framework/utils/LinkedNode.h>
#include <framework/utils/ArrayList.h>
#include <framework/io/touch/gesture/scroll/ScrollGestureDetector.h>
#include <gen/types.h>
#include <gen/resources.h>
#include <gen/shader/SimpleFontShader.h>
#include <appmain.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireUILayer.h>
#include <QuadIndexBuffer.h>
#include <sapphire/FastFontDrawer.h>

namespace userapp {
using namespace rhfw;
class SapphireLevelDescriptor;

class LevelSelectorLayer: public SapphireUILayer {
public:
	class LevelPartition: public LinkedNode<LevelPartition> {
	public:
		typedef bool (*FilterFunction)(const SapphireLevelDescriptor* desc, void* param);
		static bool AcceptFilter(const SapphireLevelDescriptor* desc, void* param) {
			return true;
		}
		virtual ~LevelPartition() = default;

		virtual void invalidateData() = 0;

		virtual bool initialized(FilterFunction func, void* param) = 0;
		virtual bool hasDescriptor(const SapphireLevelDescriptor* desc) {
			return getIndex(desc) >= 0;
		}
		virtual int getIndex(const SapphireLevelDescriptor* desc) {
			auto count = getCount();
			for (unsigned int i = 0; i < count; ++i) {
				if (get(i) == desc) {
					return i;
				}
			}
			return -1;
		}
		virtual bool descriptorRemoved(const SapphireLevelDescriptor* desc) = 0;

		virtual unsigned int getCount() = 0;
		virtual const SapphireLevelDescriptor* get(unsigned int index) = 0;

		virtual const char* getTitle() = 0;

		virtual LevelPartition* get() {
			return this;
		}
		virtual bool applyFilter(FilterFunction func, void* param) = 0;
	};
private:
	static bool FilterFunction(const SapphireLevelDescriptor* desc, void* param);
	class SelectedItem {
	public:
		LevelPartition* partition = nullptr;
		const SapphireLevelDescriptor* descriptor = nullptr;
		SelectedItem(LevelPartition* partition, const SapphireLevelDescriptor* descriptor)
				: partition(partition), descriptor(descriptor) {
		}
		SelectedItem() {
		}
		SelectedItem(NULLPTR_TYPE) {
		}

		bool operator==(const SelectedItem& item) const {
			return partition == item.partition && descriptor == item.descriptor;
		}
		bool operator!=(const SelectedItem& item) const {
			return !(*this == item);
		}

		bool operator==(NULLPTR_TYPE)const {
			return partition == nullptr;
		}
		bool operator!=(NULLPTR_TYPE)const {
			return partition != nullptr;
		}
		SelectedItem& operator=(NULLPTR_TYPE) {
			partition = nullptr;
			descriptor = nullptr;
			return *this;
		}
	};
	rhfw::AutoResource<rhfw::render::Texture> backIconWhite = getTexture(rhfw::ResIds::gameres::game_sapphire::art::ic_arrow_back_white);
	AutoResource<Font> font = getFont(ResIds::build::sipka_rh_font_convert::gameres::Consolas_ttf);

	LifeCycleChain<Animation> pageAnimationHolder;

	ArrayList<LevelPartition> partitions;
	SelectedItem selected;
	SelectedItem lastSelected;

	Rectangle backButtonPos;
	bool backPressed = false;

	float titleTextSize = 0.0f;

	FixedString titleText;

	Rectangle gridRect;
	bool gridLayoutDone = false;

	unsigned int gridRowCount = 0;
	unsigned int gridColumnCount = 0;
	Size2F gridItemSize;

	AutoResource<FrameAnimation> markers = getAnimation(ResIds::gameres::game_sapphire::level_markers);

	/**
	 * original scrolling down position
	 */
	Vector2F scrollDownPos;

	FastFontDrawerPool fontDrawerPool;
	FastFontDrawer fontDrawer {fontDrawerPool};

	ScrollGestureDetector scrollDetector;

	static const unsigned int FILTER_MAX_LENGTH = 32;
	char filterData[FILTER_MAX_LENGTH + 1] {0};
	unsigned int filterLength = 0;

	void scrollTouch();

	void setSelectedLevel(const SelectedItem& selected);

	unsigned int getPartitionColumnCount(LevelPartition* partition, unsigned int rowcount);
	unsigned int countColumns(unsigned int rowcount);
	LevelPartition* findPartitionForDescriptor(const SapphireLevelDescriptor* desc);

	void layoutGrid(const rhfw::core::WindowSize& size);
	bool checkForLevels();

	void startPageAnimator(unsigned int page);

	void showLevelLoadFailedDialog();

	void onLevelSelected(const SapphireLevelDescriptor* desc);
	void onLevelSelected(const SelectedItem& selection);
	void onLevelStartSelected(const SapphireLevelDescriptor* desc);
	void onLevelStartSuspendedSelected(const SapphireLevelDescriptor* desc);

	void clearSelected() {
		if (selected != nullptr) {
			lastSelected = selected;
			selected = nullptr;
		}
	}

	void animateToItem(const SelectedItem& selected);

	SelectedItem getTouchedItem(const Vector2F& touchpos);

	template<SapphireDirection Direction>
	void moveSelection();

	LevelPartition* getNextNonEmptyPartition(int index);
	LevelPartition* getPrevNonEmptyPartition(unsigned int index);

	void clearPartitions();

	void setTitleText(FixedString title);

	void addFilterChar(UnicodeCodePoint cp);
	void addFilterChars(UnicodeCodePoint cp, unsigned int count);

	void setFilterLength(unsigned int length);
protected:
	virtual void onSceneSizeInitialized() override;
public:
	LevelSelectorLayer(SapphireUILayer* parent, SapphireDifficulty diff, unsigned int playercount = 1);
	LevelSelectorLayer(SapphireUILayer* parent);

	virtual void drawImpl(float displaypercent) override;

	virtual bool onKeyEventImpl() override;

	virtual void displayKeyboardSelection() override {
		if (selected == nullptr) {
			setSelectedLevel(lastSelected);
			animateToItem(selected);
		}
	}
	virtual void hideKeyboardSelection() override {
		clearSelected();
	}

	void displaySelection(const SapphireLevelDescriptor* desc);
	void descriptorRemoved(const SapphireLevelDescriptor* desc);

	virtual bool touchImpl() override;

	virtual void sizeChanged(const rhfw::core::WindowSize& size) override;

	virtual void setScene(Scene* scene) override;

	void setupDefaultPartitions(SapphireScene* scene, SapphireDifficulty diff, unsigned int playercount, FixedString title);
	void setupDefaultPartitions(SapphireScene* scene, SapphireDifficulty diff, unsigned int playercount);

	void reloadLevels();
};

}
 // namespace userapp

#endif /* TEST_SAPPHIRE_LEVELSELECTORLAYER_H_ */
