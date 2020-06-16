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
 * LevelDrawer.cpp
 *
 *  Created on: 2016. jul. 29.
 *      Author: sipka
 */

#include <framework/geometry/Rectangle.h>
#include <framework/geometry/Vector.h>
#include <framework/render/Renderer.h>
#include <sapphire/levelrender/LevelDrawer.h>
#include <sapphire/level/Level.h>
#include <sapphire/FrameAnimation.h>
#include <sapphire/SapphireScene.h>
#include <sapphire/sapphireconstants.h>
#include <sapphire/levelrender/LevelDrawer3D.h>
#include <gen/log.h>

#include <sapphire/levelrender/LevelDrawer2D.h>

namespace userapp {

LevelDrawer::LevelDrawer(Level* level)
		: level(level) {
	for (int i = 0; i < 2; ++i) {
		lastMid[i] = level->getSize();
		lastMid[i] /= 2.0f;
	}
}
LevelDrawer::~LevelDrawer() {
	delete drawerImpl;
}

void LevelDrawer::draw(float turnpercent, float alpha) {
	Size2UI levelsize { level->getSize() };

	Vector2F mid[3] { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };
	unsigned int midcount[2] { 0, 0 };

	for (int i = 0; i < levelsize.width(); ++i) {
		for (int j = 0; j < levelsize.height(); ++j) {
			const Level::GameObject& o = level->get(i, j);
			if (o.object == SapphireObject::Player) {
				unsigned int playerid = o.getPlayerId();
				++midcount[playerid];
				mid[playerid].x() += i;
				mid[playerid].y() += j;
				if (o.isMoving()) {
					float percent = o.isPlayerUsingDoor() ? turnpercent * 2 - 2 : turnpercent - 1;
					switch (o.direction) {
						case SapphireDirection::Down:
							mid[playerid].y() -= percent;
							break;
						case SapphireDirection::Up:
							mid[playerid].y() += percent;
							break;
						case SapphireDirection::Left:
							mid[playerid].x() -= percent;
							break;
						case SapphireDirection::Right:
							mid[playerid].x() += percent;
							break;
						default:
							break;
					}
				}
			}
		}
	}

	for (unsigned int playerid = 0; playerid < 2; ++playerid) {
		if (midcount[playerid] != 0) {
			mid[playerid] = (mid[playerid] + midcount[playerid] * 0.5f) / midcount[playerid];
			lastMid[playerid] = mid[playerid];
		} else {
			mid[playerid] = lastMid[playerid];
		}
	}

	Size2F fieldsize = objectSize * levelsize;

	Size2F paddingsize = size.pixelSize - paddings.leftTop() - paddings.rightBottom();
	Size2F fieldpad = paddingsize - fieldsize;
	Size2F nobjectsize { objectSize };
	bool splitscreen = false;
	if (splitTwoPlayers && level->getPlayerCount() > 1 && (fieldpad.height() < 0 ||
	//use the following for auto-split.
	//looks ugly in 3D
	//(fabsf(mid[0].x() - mid[1].x()) * objectSize.width() > size.pixelSize.width() / 2.0f)
			fieldpad.width() < 0)) {
		nobjectsize.width() *= 2;
		splitscreen = true;
		fieldsize = nobjectsize * levelsize;
		fieldpad = paddingsize - fieldsize;
	}

	mid[2] = (mid[0] + mid[1]) / 2.0f;

	for (unsigned int middleid = 0; middleid < 3; ++middleid) {
		if (fieldpad.width() >= 0) {
			mid[middleid].x() = levelsize.width() / 2.0f;
		} else {
			float minx = paddingsize.width() / nobjectsize.width() / 2.0f;
			float maxx = levelsize.width() - paddingsize.width() / 2.0f / nobjectsize.width();
			if (mid[middleid].x() < minx) {
				mid[middleid].x() = minx;
			} else if (mid[middleid].x() > maxx) {
				mid[middleid].x() = maxx;
			}
		}
		if (fieldpad.height() >= 0) {
			mid[middleid].y() = levelsize.height() / 2.0f;
		} else {
			float miny = paddingsize.height() / nobjectsize.height() / 2.0f;
			float maxy = levelsize.height() - paddingsize.height() / 2.0f / nobjectsize.height();
			if (mid[middleid].y() < miny) {
				mid[middleid].y() = miny;
			} else if (mid[middleid].y() > maxy) {
				mid[middleid].y() = maxy;
			}
		}
	}

	if (splitscreen) {
		auto vp = renderer->getViewPort();
		renderer->setViewPort(render::ViewPort { 0, 0, size.pixelSize.width() / 2, size.pixelSize.height() });
		draw(turnpercent, alpha, mid[1], nobjectsize);
		renderer->setViewPort(render::ViewPort { size.pixelSize.width() / 2, 0, size.pixelSize.width() / 2, size.pixelSize.height() });
		draw(turnpercent, alpha, mid[0], nobjectsize);
		renderer->setViewPort(vp);

		renderer->setDepthTest(false);
		renderer->initDraw();
		Color sepcolor = difficultyToColor(level->getInfo().difficulty);
		drawRectangleColor(Matrix2D { }.setScreenDimension(size.pixelSize), Color { sepcolor.rgb(), alpha }, separator);
	} else {
		if (midcount[1] > 0) {
			draw(turnpercent, alpha, mid[2], nobjectsize);
		} else {
			draw(turnpercent, alpha, mid[0], nobjectsize);
		}
	}

}

void LevelDrawer::draw(float turnpercent, float alpha, const Vector2F& mid) {
	draw(turnpercent, alpha, mid, objectSize);
}
void LevelDrawer::draw(float turnpercent, float alpha, const Vector2F& mid, const Size2F& objectsize) {
//add 2 to get moving elements on side, add 1 to correctly ceil
	Size2F elemcounts = size.pixelSize / objectsize + 2 + 1;

	Size2UI maxiter = (mid + elemcounts / 2.0f + 1.0f);
	maxiter.x() = min(level->getWidth(), maxiter.x());
	maxiter.y() = min(level->getHeight(), maxiter.y());
	Size2UI miniter = { max(0, (int) (mid.x() - elemcounts.x() / 2.0f)), max(0, (int) (mid.y() - elemcounts.y() / 2.0f)) };

	if (miniter.x() < maxiter.x() && miniter.y() < maxiter.y()) {
		drawerImpl->draw(*this, turnpercent, alpha, miniter, maxiter, mid, objectsize);
	}
}

void LevelDrawer::setSize(const core::WindowSize& size, float scale) {
	this->size = size;

	float dim = min(min(size.getPhysicalSize().width(), size.getPhysicalSize().height()) / (9 / scale), 1.6f * scale);
	objectSize = size.toPixels(Size2F { dim, dim });

	float sepwidth = min(size.toPixelsX(0.15f), size.pixelSize.width() / 50.0f);
	separator.left = size.pixelSize.width() / 2.0f - sepwidth / 2.0f;
	separator.right = separator.left + sepwidth;
	separator.top = 0;
	separator.bottom = size.pixelSize.height();
}

static int floorint(float f) {
	return f >= 0.0f ? int(f) : int(f - 1);
}

Vector2I LevelDrawer::getCoordinatesForPoint(const Vector2F& point, const Vector2F& mid) {
	Vector2F modpoint { point.x(), size.pixelSize.height() - point.y() };
	Size2F fieldsize = size.pixelSize / objectSize;
	Vector2F localpoint = (modpoint) / objectSize - fieldsize / 2.0f + mid;
//TODO do not use floor?
	return Vector2I { floorint(localpoint.x()), floorint(localpoint.y()) };
}

Rectangle LevelDrawer::getLevelRectangle(const Vector2F& middle) const {
	Rectangle levelrect { Vector2F { 0, 0 }, level->getSize() * getObjectSize() };
	levelrect = levelrect.translate(
			Vector2F { getSize().pixelSize.width() / 2.0f - middle.x() * getObjectSize().width(), getSize().pixelSize.height() / 2.0f
					- (level->getHeight() - middle.y()) * getObjectSize().height() });
	return levelrect;
}
Rectangle LevelDrawer::getObjectRectangle(const Vector2F& middle, const Vector2UI& elemcoords) const {
	Rectangle levelrect = getLevelRectangle(middle);
	return Rectangle { levelrect.leftTop(), levelrect.leftTop() + getObjectSize() }.translate(
			Vector2F { elemcoords.x() * getObjectSize().width(), (level->getHeight() - 1 - elemcoords.y()) * getObjectSize().height() });
}

bool LevelDrawer::isSplittable() const {
	Size2F fieldsize = objectSize * level->getSize();
	Size2F paddingsize = size.pixelSize - paddings.leftTop() - paddings.rightBottom();
	Size2F fieldpad = paddingsize - fieldsize;
	return level->getPlayerCount() > 1 && (fieldpad.height() < 0 || fieldpad.width() < 0);
}

LevelDrawer::DrawerImpl* LevelDrawer::setDrawer(SapphireArtStyle style) {
	if (this->artStyle == style) {
		return drawerImpl;
	}
	switch (style) {
		case SapphireArtStyle::RETRO_2D: {
			delete drawerImpl;
			this->drawerImpl = new LevelDrawer2D(*this, level);
			break;
		}
		case SapphireArtStyle::PERSPECTIVE_3D: {
			if (drawerImpl != nullptr && this->artStyle == SapphireArtStyle::ORTHO_3D) {
				static_cast<LevelDrawer3D*>(this->drawerImpl)->setOrthographic(false);
			} else {
				delete drawerImpl;
				this->drawerImpl = new LevelDrawer3D(*this, level);
			}
			break;
		}
		case SapphireArtStyle::ORTHO_3D: {
			if (drawerImpl == nullptr || this->artStyle == SapphireArtStyle::RETRO_2D) {
				delete drawerImpl;
				this->drawerImpl = new LevelDrawer3D(*this, level);
			}
			static_cast<LevelDrawer3D*>(this->drawerImpl)->setOrthographic(true);
			break;
		}
		default: {
			break;
		}
	}
	this->artStyle = style;
	return drawerImpl;
}
}  // namespace userapp

