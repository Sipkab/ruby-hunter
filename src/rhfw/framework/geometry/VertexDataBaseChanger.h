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
 * VertexDataBaseChanger.h
 *
 *  Created on: 2015 maj. 3
 *      Author: sipka
 */

#ifndef VERTEXDATABASECHANGER_H_
#define VERTEXDATABASECHANGER_H_

#include <framework/geometry/VertexDataHolder.h>
#include <gen/configuration.h>

namespace rhfw {

template<unsigned int BaseFrom, unsigned int BaseTo>
class VertexDataBaseChanger;

template<>
class VertexDataBaseChanger<2, 3> : public VertexDataHolder<3> {
private:
	const VertexDataHolder<2>& wrapped;
public:
	VertexDataBaseChanger(const VertexDataHolder<2>& wrapped)
			: wrapped(wrapped) {
	}
	virtual void fillData(float* target, unsigned int stride) const override {
		const unsigned int count = wrapped.getVertexCount();
		wrapped.fillData(target, stride);
		for (unsigned int i = 0; i < count; ++i) {
			target[i * stride + 2] = 0.0f;
		}
	}
	virtual unsigned int getVertexCount() const override {
		return wrapped.getVertexCount();
	}
};

}
#endif /* VERTEXDATABASECHANGER_H_ */
