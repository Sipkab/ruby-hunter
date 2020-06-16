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
 * LevelDrawer.h
 *
 *  Created on: 2016. jul. 29.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_LEVEL_LEVELDRAWER_H_
#define TEST_SAPPHIRE_LEVEL_LEVELDRAWER_H_

#include <framework/core/timing.h>
#include <framework/core/Window.h>
#include <appmain.h>
#include <sapphire/SapphireScene.h>
#include <gen/resources.h>

namespace userapp {
using namespace rhfw;
class SapphireScene;
class Level;

class LevelDrawer {
public:
	class DrawerImpl {
	public:
		virtual void draw(LevelDrawer& parent, float turnpercent, float alpha, const Size2UI& begin, const Size2UI& end,
				const Vector2F& mid, const Size2F& objectSize) = 0;

		virtual void levelReloaded() {
		}

		virtual ~DrawerImpl() = default;
	};
private:
	core::WindowSize size;

	Size2F objectSize { 0.0f, 0.0f };

	bool dispenserFullOpacity = false;

	SapphireArtStyle artStyle = SapphireArtStyle::UNUSED;

	bool needBackground = true;
public:
	Rectangle paddings { 0, 0, 0, 0 };
	Rectangle separator { 0, 0, 0, 0 };

	Vector2F lastMid[2] { { 0.0f, 0.0f }, { 0.0f, 0.0f } };

	Level* level;

	bool splitTwoPlayers = true;

private:
protected:
private:
	DrawerImpl* drawerImpl = nullptr;
public:
	LevelDrawer(Level* level);
	LevelDrawer(const LevelDrawer&) = delete;
	LevelDrawer(LevelDrawer&&) = delete;
	~LevelDrawer();

	DrawerImpl* setDrawer(SapphireArtStyle style);
	bool hasDrawer() {
		return drawerImpl != nullptr;
	}
	SapphireArtStyle getArtStyle() const {
		return artStyle;
	}
	DrawerImpl* getDrawerImpl() {
		return drawerImpl;
	}

	void levelReloaded() {
		drawerImpl->levelReloaded();
	}

	void draw(float turnpercent, float alpha);
	void draw(float turnpercent, float alpha, const Vector2F& mid);
	void draw(float turnpercent, float alpha, const Vector2F& mid, const Size2F& objectsize);

	void setSize(const core::WindowSize& size, float scale);
	const core::WindowSize& getSize() const {
		return size;
	}

	void setPaddings(const Rectangle& paddings) {
		this->paddings = paddings;
	}
	const Rectangle& getPaddings() const {
		return paddings;
	}
	Rectangle& getPaddings() {
		return paddings;
	}

	const Size2F& getObjectSize() const {
		return objectSize;
	}

	Vector2I getCoordinatesForPoint(const Vector2F& point, const Vector2F& mid);

	Rectangle getLevelRectangle(const Vector2F& middle) const;
	Rectangle getObjectRectangle(const Vector2F& middle, const Vector2UI& elemcoords) const;

	bool isSplittable() const;

	bool isDispenserFullOpacity() const {
		return dispenserFullOpacity;
	}
	void setDispenserFullOpacity(bool opaque) {
		dispenserFullOpacity = opaque;
	}

	void setNeedBackground(bool needbg) {
		this->needBackground = needbg;
	}
	bool isNeedBackground() const {
		return needBackground;
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_LEVEL_LEVELDRAWER_H_ */
