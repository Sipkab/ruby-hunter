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
 * FastFontDrawer.cpp
 *
 *  Created on: 2017. jan. 2.
 *      Author: sipka
 */

#include <sapphire/FastFontDrawer.h>

namespace userapp {

void FastFontDrawerPool::prepare(Resource<Font> font, const Matrix2D& mvp) {
	this->font = font;
	textu->update( { &font->getTexture() });
	mvpu->update( { Matrix3D { }.setMatrix(mvp) });
}

void FastFontDrawerPool::commit() {
	renderer->setTopology(Topology::TRIANGLES);
	simpleFontShader->useProgram();
	simpleFontShader->set(mvpu);
	simpleFontShader->set(textu);

	quadIndexBuffer.activate();
	for (auto&& d : drawers.objects()) {
		if (d.index == 0) {
			continue;
		}
		d.initer.commit();
		d.il->activate();

		simpleFontShader->drawIndexedCount(d.index * 6 / 4);
		d.index = 0;
		d.charCount = 0;
	}
}

void FastFontDrawer::commit() {
	if (index == 0) {
		return;
	}
	initer.commit();

	quadIndexBuffer.activate();

	renderer->setTopology(Topology::TRIANGLES);

	simpleFontShader->useProgram();

	il->activate();
	simpleFontShader->set(pool.mvpu);
	simpleFontShader->set(pool.textu);

	simpleFontShader->drawIndexedCount(index * 6 / 4);
	index = 0;
	charCount = 0;
}
}  // namespace userapp

