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
 * SizedObject.h
 *
 *  Created on: 2016. marc. 27.
 *      Author: sipka
 */

#ifndef RENDER_SIZEDOBJECT_H_
#define RENDER_SIZEDOBJECT_H_

#include <framework/geometry/Vector.h>

namespace rhfw {
namespace render {

class SizedObject {
private:
public:
	virtual ~SizedObject() = default;

	virtual unsigned int getWidth() const = 0;
	virtual unsigned int getHeight() const = 0;

	Size2UI getSize() const {
		return Size2UI { getWidth(), getHeight() };
	}
};

}  // namespace render

}  // namespace rhfw

#endif /* RENDER_SIZEDOBJECT_H_ */
