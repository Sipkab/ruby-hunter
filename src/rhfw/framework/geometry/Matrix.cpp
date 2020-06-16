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
 * Matrix.cpp
 *
 *  Created on: 2015 febr. 8
 *      Author: sipka
 */

#include <framework/geometry/Matrix.h>
#include <framework/render/Renderer.h>

#include <cmath>

#include <gen/log.h>

namespace rhfw {

enum MatrixOrder {
	ROW_MAJOR,
	COLUMN_MAJOR
};

template<unsigned int GetRow, unsigned int GetColumn, unsigned int Dimension, MatrixOrder Orderm>
class Indexer;

template<unsigned int GetRow, unsigned int GetColumn, unsigned int Dimension>
class Indexer<GetRow, GetColumn, Dimension, ROW_MAJOR> {
public:
	static_assert(GetRow < Dimension && GetColumn < Dimension, "Invalid dimensions");
	static const int index = GetColumn + GetRow * Dimension;
};
template<unsigned int GetRow, unsigned int GetColumn, unsigned int Dimension>
class Indexer<GetRow, GetColumn, Dimension, COLUMN_MAJOR> {
public:
	static_assert(GetRow < Dimension && GetColumn < Dimension, "Invalid dimensions");
	static const int index = GetRow + GetColumn * Dimension;
};

#define M_INDEX(row, column) (Indexer<(row), (column), DIMENSION_VAL, COLUMN_MAJOR>::index)

template<> Matrix2D& Matrix2D::setIdentity() {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}

template<> Matrix2D& Matrix2D::transpose() {
	float temp;

	temp = data[M_INDEX(1, 0)];
	data[M_INDEX(1, 0)] = data[M_INDEX(0, 1)];
	data[M_INDEX(0, 1)] = temp;

	temp = data[M_INDEX(2, 0)];
	data[M_INDEX(2, 0)] = data[M_INDEX(0, 2)];
	data[M_INDEX(0, 2)] = temp;

	temp = data[M_INDEX(1, 2)];
	data[M_INDEX(1, 2)] = data[M_INDEX(2, 1)];
	data[M_INDEX(2, 1)] = temp;

	return *this;
}

template<> Matrix2D& Matrix2D::setRotate(float rad) {
	const float cosrad = cosf(rad);
	const float sinrad = sinf(rad);

	data[M_INDEX(0, 0)] = cosrad;
	data[M_INDEX(1, 0)] = sinrad;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = -sinrad;
	data[M_INDEX(1, 1)] = cosrad;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setScreenDimension(float left, float top, float right, float bottom) {
	const float w = right - left;
	const float h = bottom - top;
	data[M_INDEX(0, 0)] = 2.0f / w;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = -1.0f - 2.0f * left / w;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = -2.0f / h;
	data[M_INDEX(2, 1)] = 1.0f + 2.0f * top / h;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setScreenDimension(float width, float height) {
	data[M_INDEX(0, 0)] = 2.0f / width;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = -1.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = -2.0f / height;
	data[M_INDEX(2, 1)] = 1.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setTranslate(float x, float y) {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = x;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = y;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setScale(float x, float y) {
	data[M_INDEX(0, 0)] = x;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = y;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setShearX(float amount) {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = amount;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix3D& Matrix3D::setShearX(float amount) {
	data[M_INDEX(0, 0)] = 1;
	data[M_INDEX(1, 0)] = amount;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<> Matrix2D& Matrix2D::setShearY(float amount) {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = amount;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	return *this;
}
template<> Matrix3D& Matrix3D::setShearY(float amount) {
	data[M_INDEX(0, 0)] = 1;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = amount;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<> Matrix3D& Matrix3D::setShearZ(float amountx, float amounty) {
	data[M_INDEX(0, 0)] = 1;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = amountx;
	data[M_INDEX(1, 2)] = amounty;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}

template<> Matrix2D& Matrix2D::multScreenDimension(float left, float top, float right, float bottom) {
	const float w = right - left;
	const float h = bottom - top;
	data[M_INDEX(0, 0)] = data[M_INDEX(0, 0)] * 2.0f / w + data[M_INDEX(0, 2)] * (-1.0f - 2.0f * left / w);
	data[M_INDEX(1, 0)] = data[M_INDEX(1, 0)] * 2.0f / w + data[M_INDEX(1, 2)] * (-1.0f - 2.0f * left / w);
	data[M_INDEX(2, 0)] = data[M_INDEX(2, 0)] * 2.0f / w + data[M_INDEX(2, 2)] * (-1.0f - 2.0f * left / w);

	data[M_INDEX(0, 1)] = data[M_INDEX(0, 1)] * -2.0f / h + data[M_INDEX(0, 2)] * (1.0f + 2.0f * top / h);
	data[M_INDEX(1, 1)] = data[M_INDEX(1, 1)] * -2.0f / h + data[M_INDEX(1, 2)] * (1.0f + 2.0f * top / h);
	data[M_INDEX(2, 1)] = data[M_INDEX(2, 1)] * -2.0f / h + data[M_INDEX(2, 2)] * (1.0f + 2.0f * top / h);
	return *this;
}
template<> Matrix2D& Matrix2D::multScreenDimension(float width, float height) {
	data[M_INDEX(0, 0)] = data[M_INDEX(0, 0)] * 2.0f / width - data[M_INDEX(0, 2)];
	data[M_INDEX(1, 0)] = data[M_INDEX(1, 0)] * 2.0f / width - data[M_INDEX(1, 2)];
	data[M_INDEX(2, 0)] = data[M_INDEX(2, 0)] * 2.0f / width - data[M_INDEX(2, 2)];

	data[M_INDEX(0, 1)] = data[M_INDEX(0, 1)] * -2.0f / height + data[M_INDEX(0, 2)];
	data[M_INDEX(1, 1)] = data[M_INDEX(1, 1)] * -2.0f / height + data[M_INDEX(1, 2)];
	data[M_INDEX(2, 1)] = data[M_INDEX(2, 1)] * -2.0f / height + data[M_INDEX(2, 2)];
	return *this;
}
template<> Matrix2D& Matrix2D::multRotate(float rad) {
	//TODO
	const float cosrad = cosf(rad);
	const float sinrad = sinf(rad);

	const float temp00 = data[M_INDEX(0, 0)] * cosrad + data[M_INDEX(0, 1)] * sinrad;
	const float temp10 = data[M_INDEX(1, 0)] * cosrad + data[M_INDEX(1, 1)] * sinrad;
	const float temp20 = data[M_INDEX(2, 0)] * cosrad + data[M_INDEX(2, 1)] * sinrad;

	data[M_INDEX(0, 1)] = data[M_INDEX(0, 0)] * -sinrad + data[M_INDEX(0, 1)] * cosrad;
	data[M_INDEX(1, 1)] = data[M_INDEX(1, 0)] * -sinrad + data[M_INDEX(1, 1)] * cosrad;
	data[M_INDEX(2, 1)] = data[M_INDEX(2, 0)] * -sinrad + data[M_INDEX(2, 1)] * cosrad;

	data[M_INDEX(0, 0)] = temp00;
	data[M_INDEX(1, 0)] = temp10;
	data[M_INDEX(2, 0)] = temp20;
	return *this;
}
template<> Matrix2D& Matrix2D::multTranslate(float x, float y) {
	data[M_INDEX(0, 0)] += data[M_INDEX(0, 2)] * x;
	data[M_INDEX(1, 0)] += data[M_INDEX(1, 2)] * x;
	data[M_INDEX(2, 0)] += data[M_INDEX(2, 2)] * x;

	data[M_INDEX(0, 1)] += data[M_INDEX(0, 2)] * y;
	data[M_INDEX(1, 1)] += data[M_INDEX(1, 2)] * y;
	data[M_INDEX(2, 1)] += data[M_INDEX(2, 2)] * y;
	return *this;
}
template<> Matrix2D& Matrix2D::multScale(float x, float y) {
	data[M_INDEX(0, 0)] *= x;
	data[M_INDEX(1, 0)] *= x;
	data[M_INDEX(2, 0)] *= x;

	data[M_INDEX(0, 1)] *= y;
	data[M_INDEX(1, 1)] *= y;
	data[M_INDEX(2, 1)] *= y;
	return *this;
}

#define M_3D_MULT(r, c, array) (data[M_INDEX(r, 0)] * array[M_INDEX(0, c)] +\
		data[M_INDEX(r, 1)] * array[M_INDEX(1, c)] +\
		data[M_INDEX(r, 2)] * array[M_INDEX(2, c)])
template<> Matrix2D& Matrix2D::operator *=(const Matrix2D& m) {
	//TODO eleg lenne csak 3 float-nyi buffer?
	float buf[DIMENSION_VAL * DIMENSION_VAL];
	buf[M_INDEX(0, 0)] = M_3D_MULT(0, 0, m.data);
	buf[M_INDEX(1, 0)] = M_3D_MULT(1, 0, m.data);
	buf[M_INDEX(2, 0)] = M_3D_MULT(2, 0, m.data);

	buf[M_INDEX(0, 1)] = M_3D_MULT(0, 1, m.data);
	buf[M_INDEX(1, 1)] = M_3D_MULT(1, 1, m.data);
	buf[M_INDEX(2, 1)] = M_3D_MULT(2, 1, m.data);

	buf[M_INDEX(0, 2)] = M_3D_MULT(0, 2, m.data);
	buf[M_INDEX(1, 2)] = M_3D_MULT(1, 2, m.data);
	buf[M_INDEX(2, 2)] = M_3D_MULT(2, 2, m.data);

	data[M_INDEX(0, 0)] = buf[M_INDEX(0, 0)];
	data[M_INDEX(1, 0)] = buf[M_INDEX(1, 0)];
	data[M_INDEX(2, 0)] = buf[M_INDEX(2, 0)];

	data[M_INDEX(0, 1)] = buf[M_INDEX(0, 1)];
	data[M_INDEX(1, 1)] = buf[M_INDEX(1, 1)];
	data[M_INDEX(2, 1)] = buf[M_INDEX(2, 1)];

	data[M_INDEX(0, 2)] = buf[M_INDEX(0, 2)];
	data[M_INDEX(1, 2)] = buf[M_INDEX(1, 2)];
	data[M_INDEX(2, 2)] = buf[M_INDEX(2, 2)];

	return *this;
}
#define M_4D_MULT(r, c, array) (data[M_INDEX(r, 0)] * array[M_INDEX(0, c)] +\
		data[M_INDEX(r, 1)] * array[M_INDEX(1, c)] +\
		data[M_INDEX(r, 2)] * array[M_INDEX(2, c)] +\
		data[M_INDEX(r, 3)] * array[M_INDEX(3, c)])
template<> Matrix3D& Matrix3D::operator *=(const Matrix3D& m) {
	//TODO eleg lenne csak 4 float-nyi buffer?
	float buf[DIMENSION_VAL * DIMENSION_VAL];
	buf[M_INDEX(0, 0)] = M_4D_MULT(0, 0, m.data);
	buf[M_INDEX(1, 0)] = M_4D_MULT(1, 0, m.data);
	buf[M_INDEX(2, 0)] = M_4D_MULT(2, 0, m.data);
	buf[M_INDEX(3, 0)] = M_4D_MULT(3, 0, m.data);

	buf[M_INDEX(0, 1)] = M_4D_MULT(0, 1, m.data);
	buf[M_INDEX(1, 1)] = M_4D_MULT(1, 1, m.data);
	buf[M_INDEX(2, 1)] = M_4D_MULT(2, 1, m.data);
	buf[M_INDEX(3, 1)] = M_4D_MULT(3, 1, m.data);

	buf[M_INDEX(0, 2)] = M_4D_MULT(0, 2, m.data);
	buf[M_INDEX(1, 2)] = M_4D_MULT(1, 2, m.data);
	buf[M_INDEX(2, 2)] = M_4D_MULT(2, 2, m.data);
	buf[M_INDEX(3, 2)] = M_4D_MULT(3, 2, m.data);

	buf[M_INDEX(0, 3)] = M_4D_MULT(0, 3, m.data);
	buf[M_INDEX(1, 3)] = M_4D_MULT(1, 3, m.data);
	buf[M_INDEX(2, 3)] = M_4D_MULT(2, 3, m.data);
	buf[M_INDEX(3, 3)] = M_4D_MULT(3, 3, m.data);

	data[M_INDEX(0, 0)] = buf[M_INDEX(0, 0)];
	data[M_INDEX(1, 0)] = buf[M_INDEX(1, 0)];
	data[M_INDEX(2, 0)] = buf[M_INDEX(2, 0)];
	data[M_INDEX(3, 0)] = buf[M_INDEX(3, 0)];

	data[M_INDEX(0, 1)] = buf[M_INDEX(0, 1)];
	data[M_INDEX(1, 1)] = buf[M_INDEX(1, 1)];
	data[M_INDEX(2, 1)] = buf[M_INDEX(2, 1)];
	data[M_INDEX(3, 1)] = buf[M_INDEX(3, 1)];

	data[M_INDEX(0, 2)] = buf[M_INDEX(0, 2)];
	data[M_INDEX(1, 2)] = buf[M_INDEX(1, 2)];
	data[M_INDEX(2, 2)] = buf[M_INDEX(2, 2)];
	data[M_INDEX(3, 2)] = buf[M_INDEX(3, 2)];

	data[M_INDEX(0, 3)] = buf[M_INDEX(0, 3)];
	data[M_INDEX(1, 3)] = buf[M_INDEX(1, 3)];
	data[M_INDEX(2, 3)] = buf[M_INDEX(2, 3)];
	data[M_INDEX(3, 3)] = buf[M_INDEX(3, 3)];

	return *this;
}

#define M_R_INDEX(row, column) (Indexer<(row), (column), MatIndexerType::DIMENSION_VAL, COLUMN_MAJOR>::index)
Vector2F& operator *=(Vector2F& pos, const Matrix2D& matrix) {
	typedef Matrix2D MatIndexerType;

	const float w = pos.x() * matrix[M_R_INDEX(0, 2)] + pos.y() * matrix[M_R_INDEX(1, 2)] + matrix[M_R_INDEX(2, 2)];
	const float x = pos.x() * matrix[M_R_INDEX(0, 0)] + pos.y() * matrix[M_R_INDEX(1, 0)] + matrix[M_R_INDEX(2, 0)];
	pos.y() = (pos.x() * matrix[M_R_INDEX(0, 1)] + pos.y() * matrix[M_R_INDEX(1, 1)] + matrix[M_R_INDEX(2, 1)]) / w;
	pos.x() = x / w;
	return pos;
}
Vector2F operator *(const Vector2F& pos, const Matrix2D& matrix) {
	Vector2F res = pos;
	res *= matrix;
	return res;
}

Vector3F& operator *=(Vector3F& pos, const Matrix3D& matrix) {
	typedef Matrix3D MatIndexerType;
	const float w = pos.x() * matrix[M_R_INDEX(0, 3)] + pos.y() * matrix[M_R_INDEX(1, 3)] + pos.z() * matrix[M_R_INDEX(2, 3)]
			+ matrix[M_R_INDEX(3, 3)];
	const float x = pos.x() * matrix[M_R_INDEX(0, 0)] + pos.y() * matrix[M_R_INDEX(1, 0)] + pos.z() * matrix[M_R_INDEX(2, 0)]
			+ matrix[M_R_INDEX(3, 0)];
	const float y = pos.x() * matrix[M_R_INDEX(0, 1)] + pos.y() * matrix[M_R_INDEX(1, 1)] + pos.z() * matrix[M_R_INDEX(2, 1)]
			+ matrix[M_R_INDEX(3, 1)];
	pos.z() = (pos.x() * matrix[M_R_INDEX(0, 2)] + pos.y() * matrix[M_R_INDEX(1, 2)] + pos.z() * matrix[M_R_INDEX(2, 2)]
			+ matrix[M_R_INDEX(3, 2)]) / w;
	pos.x() = x / w;
	pos.y() = y / w;
	return pos;
}
Vector3F operator *(const Vector3F& pos, const Matrix3D& matrix) {
	Vector3F res = pos;
	res *= matrix;
	return res;
}

Vector4F& operator *=(Vector4F& v, const Matrix3D& matrix) {
	typedef Matrix3D MatIndexerType;
	Vector4F r = v;
	v.x() = r.x() * matrix[M_R_INDEX(0, 0)] + r.y() * matrix[M_R_INDEX(1, 0)] + r.z() * matrix[M_R_INDEX(2, 0)]
			+ r.w() * matrix[M_R_INDEX(3, 0)];
	v.y() = r.x() * matrix[M_R_INDEX(0, 1)] + r.y() * matrix[M_R_INDEX(1, 1)] + r.z() * matrix[M_R_INDEX(2, 1)]
			+ r.w() * matrix[M_R_INDEX(3, 1)];
	v.z() = r.x() * matrix[M_R_INDEX(0, 2)] + r.y() * matrix[M_R_INDEX(1, 2)] + r.z() * matrix[M_R_INDEX(2, 2)]
			+ r.w() * matrix[M_R_INDEX(3, 2)];
	v.w() = r.x() * matrix[M_R_INDEX(0, 3)] + r.y() * matrix[M_R_INDEX(1, 3)] + r.z() * matrix[M_R_INDEX(2, 3)]
			+ r.w() * matrix[M_R_INDEX(3, 3)];
	return v;
}
Vector4F&& operator *=(Vector4F&& v, const Matrix3D& matrix) {
	return static_cast<Vector4F&&>(v *= matrix);
}

template<> Matrix3D& Matrix3D::setIdentity() {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<> Matrix3D& Matrix3D::transpose() {
	float temp;

	temp = data[M_INDEX(1, 0)];
	data[M_INDEX(1, 0)] = data[M_INDEX(0, 1)];
	data[M_INDEX(0, 1)] = temp;

	temp = data[M_INDEX(2, 0)];
	data[M_INDEX(2, 0)] = data[M_INDEX(0, 2)];
	data[M_INDEX(0, 2)] = temp;

	temp = data[M_INDEX(3, 0)];
	data[M_INDEX(3, 0)] = data[M_INDEX(0, 3)];
	data[M_INDEX(0, 3)] = temp;

	temp = data[M_INDEX(1, 2)];
	data[M_INDEX(1, 2)] = data[M_INDEX(2, 1)];
	data[M_INDEX(2, 1)] = temp;

	temp = data[M_INDEX(1, 3)];
	data[M_INDEX(1, 3)] = data[M_INDEX(3, 1)];
	data[M_INDEX(3, 1)] = temp;

	temp = data[M_INDEX(2, 3)];
	data[M_INDEX(2, 3)] = data[M_INDEX(3, 2)];
	data[M_INDEX(3, 2)] = temp;

	return *this;
}

//TODO implement these
template<> Matrix3D& Matrix3D::setRotate(float rad, float x, float y, float z) {
	const float len = sqrtf(x * x + y * y + z * z);
	x /= len;
	y /= len;
	z /= len;
	const float cosrad = cosf(rad);
	const float sinrad = sinf(rad);
	const float mincos = 1.0f - cosrad;

	data[M_INDEX(0, 0)] = x * x * mincos + cosrad;
	data[M_INDEX(1, 0)] = x * y * mincos + z * sinrad;
	data[M_INDEX(2, 0)] = x * z * mincos - y * sinrad;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = y * x * mincos - z * sinrad;
	data[M_INDEX(1, 1)] = y * y * mincos + cosrad;
	data[M_INDEX(2, 1)] = y * z * mincos + x * sinrad;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = z * x * mincos + y * sinrad;
	data[M_INDEX(1, 2)] = z * y * mincos - x * sinrad;
	data[M_INDEX(2, 2)] = z * z * mincos + cosrad;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;

	return *this;
}
template<> Matrix3D& Matrix3D::setTranslate(float x, float y, float z) {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = x;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = y;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = z;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;

	return *this;
}
template<> Matrix3D& Matrix3D::setScale(float x, float y, float z) {
	data[M_INDEX(0, 0)] = x;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = y;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = z;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<> Matrix3D& Matrix3D::setProjection(const render::Renderer& renderer, float fovy, float aspect, float znear, float zfar) {
	const float diff = znear - zfar;
	data[M_INDEX(0, 0)] = 1.0f / (tanf(fovy / 2.0f) * aspect);
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 1.0f / tanf(fovy / 2.0f);
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	static_assert((unsigned int)DepthRange::_count_of_entries == 2, "More than 2 types of depthrange");
	if (renderer.getDepthRange() == DepthRange::RANGE_0_TO_1) {
		//moving to: [0, 1] from [-1, 1]
		data[M_INDEX(2, 2)] = (znear + zfar) / diff * 0.5f - 0.5f;
		data[M_INDEX(3, 2)] = znear * zfar / diff;
	} else {
		data[M_INDEX(2, 2)] = (znear + zfar) / diff;
		data[M_INDEX(3, 2)] = 2.0f * znear * zfar / diff;
	}

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = -1.0f;
	data[M_INDEX(3, 3)] = 0.0f;
	return *this;
}
template<>
Matrix3D& Matrix3D::setOrthographic(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
		float zfar) {
	const float diffx = right - left;
	const float diffy = top - bottom;
	const float diffz = zfar - znear;
	data[M_INDEX(0, 0)] = 2.0f / diffx;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = -(right + left) / diffx;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 2.0f / diffy;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = -(top + bottom) / diffy;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	static_assert((unsigned int)DepthRange::_count_of_entries == 2, "More than 2 types of depthrange");
	if (renderer.getDepthRange() == DepthRange::RANGE_0_TO_1) {
		//moving from [-1, 1] to [0, 1] 
		data[M_INDEX(2, 2)] = -1.0 / diffz;
		data[M_INDEX(3, 2)] = -(zfar + znear) / diffz * 0.5f + 0.5f;
	} else {
		//TODO test this
		data[M_INDEX(2, 2)] = -2.0 / diffz;
		data[M_INDEX(3, 2)] = -(zfar + znear) / diffz;
	}

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<>
Matrix3D& Matrix3D::multOrthographic(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
		float zfar) {
	//TODO
	return *this *= Matrix3D { }.setOrthographic(renderer, left, right, bottom, top, znear, zfar);
}
template<>
Matrix3D& Matrix3D::setOrthographicInverse(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
		float zfar) {
	const float diffx = right - left;
	const float diffy = top - bottom;
	const float diffz = znear - zfar;
	data[M_INDEX(0, 0)] = diffx / 2.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = (right + left) / 2.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = diffy / 2.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = (top + bottom) / 2.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	static_assert((unsigned int)DepthRange::_count_of_entries == 2, "More than 2 types of depthrange");
	if (renderer.getDepthRange() == DepthRange::RANGE_0_TO_1) {
		//moving to: [-znear, -zfar] from [0, 1]
		data[M_INDEX(2, 2)] = -diffz;
		data[M_INDEX(3, 2)] = (znear);
	} else {
		data[M_INDEX(2, 2)] = diffz / -2.0f;
		data[M_INDEX(3, 2)] = (zfar + znear) / 2.0f;
	}

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;
	return *this;
}
template<>
Matrix3D& Matrix3D::multOrthographicInverse(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
		float zfar) {
	//TODO
	return *this *= Matrix3D { }.setOrthographicInverse(renderer, left, right, bottom, top, znear, zfar);
}
template<>
Matrix3D& Matrix3D::setFrustum(const render::Renderer& renderer, float left, float top, float right, float bottom, float znear,
		float zfar) {
	ASSERT(znear < zfar);
	const float diffx = right - left;
	const float diffy = top - bottom;
	const float diffz = zfar - znear;
	data[M_INDEX(0, 0)] = 2.0f * znear / diffx;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = (right + left) / diffx;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = 2.0f * znear / diffy;
	data[M_INDEX(2, 1)] = (top + bottom) / diffy;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	static_assert((unsigned int)DepthRange::_count_of_entries == 2, "More than 2 types of depthrange");
	if (renderer.getDepthRange() == DepthRange::RANGE_0_TO_1) {
		//moving from [-1, 1] to [0, 1]
		data[M_INDEX(2, 2)] = -(zfar + znear) / diffz * 0.5f - 0.5f;
		data[M_INDEX(3, 2)] = -zfar * znear / diffz;
	} else {
		data[M_INDEX(2, 2)] = -(zfar + znear) / diffz;
		data[M_INDEX(3, 2)] = -2 * zfar * znear / diffz;
	}

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = -1.0f;
	data[M_INDEX(3, 3)] = 0.0f;
	return *this;
}

template<> Matrix3D& Matrix3D::multRotate(float rad, float x, float y, float z) {
	//TODO normalisan
	return *this *= Matrix3D().setRotate(rad, x, y, z);
}
template<> Matrix3D& Matrix3D::multTranslate(float x, float y, float z) {
	data[M_INDEX(0, 0)] += data[M_INDEX(0, 3)] * x;
	data[M_INDEX(1, 0)] += data[M_INDEX(1, 3)] * x;
	data[M_INDEX(2, 0)] += data[M_INDEX(2, 3)] * x;
	data[M_INDEX(3, 0)] += data[M_INDEX(3, 3)] * x;

	data[M_INDEX(0, 1)] += data[M_INDEX(0, 3)] * y;
	data[M_INDEX(1, 1)] += data[M_INDEX(1, 3)] * y;
	data[M_INDEX(2, 1)] += data[M_INDEX(2, 3)] * y;
	data[M_INDEX(3, 1)] += data[M_INDEX(3, 3)] * y;

	data[M_INDEX(0, 2)] += data[M_INDEX(0, 3)] * z;
	data[M_INDEX(1, 2)] += data[M_INDEX(1, 3)] * z;
	data[M_INDEX(2, 2)] += data[M_INDEX(2, 3)] * z;
	data[M_INDEX(3, 2)] += data[M_INDEX(3, 3)] * z;
	return *this;
}
template<> Matrix3D& Matrix3D::multScale(float x, float y, float z) {
	data[M_INDEX(0, 0)] *= x;
	data[M_INDEX(1, 0)] *= x;
	data[M_INDEX(2, 0)] *= x;
	data[M_INDEX(3, 0)] *= x;

	data[M_INDEX(0, 1)] *= y;
	data[M_INDEX(1, 1)] *= y;
	data[M_INDEX(2, 1)] *= y;
	data[M_INDEX(3, 1)] *= y;

	data[M_INDEX(0, 2)] *= z;
	data[M_INDEX(1, 2)] *= z;
	data[M_INDEX(2, 2)] *= z;
	data[M_INDEX(3, 2)] *= z;
	return *this;
}
template<> Matrix3D& Matrix3D::multProjection(const render::Renderer& renderer, float fovy, float aspect, float znear, float zfar) {
	//TODO
	return *this *= Matrix().setProjection(renderer, fovy, aspect, znear, zfar);
}

template<>
template<>
Matrix3D& Matrix3D::setMatrix<3>(const Matrix2D& m) {
	typedef Matrix2D MatIndexerType;

	data[M_INDEX(0, 0)] = m[M_R_INDEX(0, 0)];
	data[M_INDEX(1, 0)] = m[M_R_INDEX(1, 0)];
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = m[M_R_INDEX(2, 0)];

	data[M_INDEX(0, 1)] = m[M_R_INDEX(0, 1)];
	data[M_INDEX(1, 1)] = m[M_R_INDEX(1, 1)];
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = m[M_R_INDEX(2, 1)];

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = m[M_R_INDEX(0, 2)];
	data[M_INDEX(1, 3)] = m[M_R_INDEX(1, 2)];
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = m[M_R_INDEX(2, 2)];
	return *this;
}

template<> Matrix2D& Matrix2D::multFlipY() {
	data[M_INDEX(0, 1)] = -data[M_INDEX(0, 1)];
	data[M_INDEX(1, 1)] = -data[M_INDEX(1, 1)];
	data[M_INDEX(2, 1)] = -data[M_INDEX(2, 1)];

	return *this;
}
template<> Matrix3D& Matrix3D::multFlipY() {
	data[M_INDEX(0, 1)] = -data[M_INDEX(0, 1)];
	data[M_INDEX(1, 1)] = -data[M_INDEX(1, 1)];
	data[M_INDEX(2, 1)] = -data[M_INDEX(2, 1)];
	data[M_INDEX(3, 1)] = -data[M_INDEX(3, 1)];

	return *this;
}
template<> Matrix2D& Matrix2D::setFlipY() {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = -1.0f;
	data[M_INDEX(2, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;

	return *this;
}
template<> Matrix3D& Matrix3D::setFlipY() {
	data[M_INDEX(0, 0)] = 1.0f;
	data[M_INDEX(1, 0)] = 0.0f;
	data[M_INDEX(2, 0)] = 0.0f;
	data[M_INDEX(3, 0)] = 0.0f;

	data[M_INDEX(0, 1)] = 0.0f;
	data[M_INDEX(1, 1)] = -1.0f;
	data[M_INDEX(2, 1)] = 0.0f;
	data[M_INDEX(3, 1)] = 0.0f;

	data[M_INDEX(0, 2)] = 0.0f;
	data[M_INDEX(1, 2)] = 0.0f;
	data[M_INDEX(2, 2)] = 1.0f;
	data[M_INDEX(3, 2)] = 0.0f;

	data[M_INDEX(0, 3)] = 0.0f;
	data[M_INDEX(1, 3)] = 0.0f;
	data[M_INDEX(2, 3)] = 0.0f;
	data[M_INDEX(3, 3)] = 1.0f;

	return *this;
}

template<> Matrix2D& Matrix2D::setRenderToTexture(const render::Renderer& renderer) {
	if (renderer.getTextureUvOrientation() == TextureUvOrientation::Y_DIR_UP) {
		return setFlipY();
	} else {
		return setIdentity();
	}
}
template<> Matrix2D& Matrix2D::multRenderToTexture(const render::Renderer& renderer) {
	if (renderer.getTextureUvOrientation() == TextureUvOrientation::Y_DIR_UP) {
		return multFlipY();
	} else {
		return *this;
	}
}
template<> Matrix3D& Matrix3D::setRenderToTexture(const render::Renderer& renderer) {
	if (renderer.getTextureUvOrientation() == TextureUvOrientation::Y_DIR_UP) {
		return setFlipY();
	} else {
		return setIdentity();
	}
}
template<> Matrix3D& Matrix3D::multRenderToTexture(const render::Renderer& renderer) {
	if (renderer.getTextureUvOrientation() == TextureUvOrientation::Y_DIR_UP) {
		return multFlipY();
	} else {
		return *this;
	}
}

template<> Matrix3D& Matrix3D::multNormalizeDepthCoordinates(const render::Renderer& renderer) {
	//TODO
	static_assert((unsigned int)DepthRange::_count_of_entries == 2, "More than 2 types of depthrange");
	if (renderer.getDepthRange() == DepthRange::RANGE_NEG_1_TO_POS_1) {
		multTranslate(0.0f, 0.0f, 1.0f);
		multScale(1.0f, 1.0f, 0.5f);
	}
	return *this;
}
template<> Matrix3D& Matrix3D::multXYNdcToTextureCoordinates() {
	//TODO
	multTranslate(1.0f, 1.0f, 0.0f);
	multScale(0.5f, 0.5f, 1.0f);
	return *this;
}
template<> Matrix2D& Matrix2D::multXYNdcToTextureCoordinates() {
	//TODO
	multTranslate(1.0f, 1.0f);
	multScale(0.5f, 0.5f);
	return *this;
}

}
