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
 * IndexBuffer.h
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#ifndef RENDER_INDEXBUFFER_H_
#define RENDER_INDEXBUFFER_H_

#include <framework/render/RenderObject.h>
#include <framework/render/Buffer.h>

namespace rhfw {
namespace render {

class IndexBuffer: public Buffer {
private:
protected:
public:
	virtual void activate() = 0;
};

}  // namespace render
}  // namespace rhfw

#endif /* RENDER_INDEXBUFFER_H_ */
