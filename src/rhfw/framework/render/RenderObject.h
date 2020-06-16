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
 * RenderObject.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RENDER_RENDEROBJECT_H_
#define RENDER_RENDEROBJECT_H_

#include <framework/resource/ShareableResource.h>

namespace rhfw {
namespace render {

class Renderer;

class RenderObject: public ShareableResource {
	friend class Renderer;
private:
public:
};

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_RENDEROBJECT_H_ */
