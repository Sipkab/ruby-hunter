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
//  QuadIndexBufferSource.h
//  TestApp
//
//  Created by User on 2016. 03. 23..
//  Copyright © 2016. User. All rights reserved.
//

#ifndef QUADINDEXBUFFERSOURCE_H_
#define QUADINDEXBUFFERSOURCE_H_

#include <framework/render/IndexBuffer.h>
#include <appmain.h>

#include <gen/log.h>

namespace userapp {
class QuadIndexBuffer {
private:
	rhfw::Resource<rhfw::render::IndexBuffer> buffer;
	unsigned int count;

	void fillData(rhfw::WriteOnlyPointer<unsigned short> indices) {
		for (unsigned int i = 0, c = 0; i < count * 6; i += 6, c += 4, indices += 6) {
			indices[0] = c + 0;
			indices[1] = c + 1;
			indices[2] = c + 2;
			indices[3] = c + 2;
			indices[4] = c + 1;
			indices[5] = c + 3;
		}
	}
public:
	QuadIndexBuffer(unsigned int count = 32)
			: count { count } {
	}
	QuadIndexBuffer(QuadIndexBuffer&& o) = default;
	~QuadIndexBuffer() {
	}

	bool load() {
		buffer = renderer->createIndexBuffer();
		bool res = buffer.load();
		buffer->setBufferInitializer<unsigned short>([this](rhfw::WriteOnlyPointer<unsigned short> ptr) {
			fillData(ptr);
		}, this->count * 6);
		buffer->initialize(rhfw::BufferType::IMMUTABLE);
		return res;
	}
	void free() {
		buffer.free();
		buffer = nullptr;
	}

	void ensureLength(unsigned int quadcount) {
		if (count < quadcount) {
			this->count = quadcount * 2;
			buffer->setBufferInitializerLength<unsigned short>(this->count * 6);
		}
	}
	void activate() {
		buffer->activate();
	}
};

} // namespace userapp

#endif /* QUADINDEXBUFFERSOURCE_H_ */
