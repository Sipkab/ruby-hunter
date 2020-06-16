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
 * VertexDataHolder.h
 *
 *  Created on: 2014.08.25.
 *      Author: sipka
 */

#ifndef VERTEXHOLDER_H_
#define VERTEXHOLDER_H_
#include <gen/configuration.h>
namespace rhfw {

template<unsigned int VertexDimension>
class VertexDataHolder {
protected:
	VertexDataHolder() = default;
	VertexDataHolder(VertexDataHolder&&) = default;
	VertexDataHolder(const VertexDataHolder&) = default;
	VertexDataHolder& operator=(const VertexDataHolder&) = default;
	VertexDataHolder& operator=(VertexDataHolder&&) = default;
	~VertexDataHolder() = default;
public:
	static const unsigned int FLOAT_PER_VERTEX = VertexDimension;

	virtual void fillData(float* target, unsigned int stride) const = 0;
	virtual unsigned int getVertexCount() const = 0;
};

template<typename T>
class PodToVertexData: public VertexDataHolder<T::CONV_TO_VERTEX_DATA_DIMENSION> {
public:

	const T& data;

	PodToVertexData(const T& data)
			: data(data) {
	}
	PodToVertexData(T& data)
			: data(data) {
	}

	void fillData(float* data, unsigned int stride) const {
		this->data.fillData(data, stride);
	}
	unsigned int getVertexCount() const {
		return data.getVertexCount();
	}
};

typedef VertexDataHolder<1> VertexData1D;
typedef VertexDataHolder<2> VertexData2D;
typedef VertexDataHolder<3> VertexData3D;
}

#endif /* VERTEXHOLDER_H_ */
