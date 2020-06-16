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
 * Boundary.h
 *
 *  Created on: 2015 febr. 15
 *      Author: sipka
 */

#ifndef BOUNDARY_H_
#define BOUNDARY_H_

#include <framework/geometry/Vector.h>
#include <gen/configuration.h>
namespace rhfw {

class Boundary {
protected:
	~Boundary() = default;
public:
	virtual bool isInside(const Vector2F& pos) const = 0;
};
}

#endif /* BOUNDARY_H_ */
