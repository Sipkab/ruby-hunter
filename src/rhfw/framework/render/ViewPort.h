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
//
//  ViewPort.h
//  TestApp
//
//  Created by User on 2016. 03. 16..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef RENDER_VIEWPORT_H_
#define RENDER_VIEWPORT_H_

#include <framework/geometry/Vector.h>

namespace rhfw {
namespace render {

class ViewPort {
public:
	Vector2UI pos;
	Size2UI size;

	ViewPort(unsigned int x, unsigned int y, unsigned int width, unsigned int height)
			: pos { x, y }, size { width, height } {
	}
	ViewPort(const Vector2UI& pos, const Size2UI& size)
			: ViewPort { pos.x(), pos.y(), size.width(), size.height() } {
	}
	ViewPort(unsigned int x, unsigned int y, const Size2UI& size)
			: ViewPort { x, y, size.width(), size.height() } {
	}
	ViewPort(const Vector2UI& pos, unsigned int width, unsigned int height)
			: ViewPort { pos.x(), pos.y(), width, height } {
	}
	ViewPort(const Size2UI& size)
			: ViewPort { 0, 0, size.width(), size.height() } {
	}
	ViewPort()
			: ViewPort { 0, 0, 0, 0 } {
	}

	bool operator==(const ViewPort& o) {
		return pos == o.pos && size == o.size;
	}
	bool operator!=(const ViewPort& o) {
		return pos != o.pos || size != o.size;
	}
};

} // namespace render
} // namespace rhfw

#endif /* RENDER_VIEWPORT_H_ */
