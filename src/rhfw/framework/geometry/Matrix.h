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
 * Matrix.h
 *
 *  Created on: 2015 febr. 8
 *      Author: sipka
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <framework/geometry/Vector.h>
#include <gen/configuration.h>

namespace rhfw {

namespace render {
class Renderer;
}  // namespace render

template<unsigned int Dimension>
class Matrix;

typedef Matrix<3> Matrix2D;
typedef Matrix<4> Matrix3D;

template<unsigned int Dimension>
class Matrix {
private:
	float data[Dimension * Dimension];
public:

	static const int DIMENSION_VAL = Dimension;

	/// dual functions
	Matrix() = default;
	Matrix(const Matrix<Dimension>&) = default;
	Matrix(Matrix<Dimension> &&) = default;
	Matrix<Dimension>& operator=(const Matrix<Dimension>&) = default;
	Matrix<Dimension>& operator=(Matrix<Dimension> &&) = default;

	Matrix<Dimension>& setIdentity();
	Matrix<Dimension>& operator *=(const Matrix<Dimension>& m);
	Matrix<Dimension> operator *(const Matrix<Dimension>& m) const {
		return Matrix<Dimension>(*this) *= m;
	}
	Matrix<Dimension>& transpose();
	Matrix<Dimension> transposed() {
		return Matrix<Dimension>(*this).transpose();
	}

	template<unsigned int InputDim>
	Matrix<Dimension>& setMatrix(const Matrix<InputDim>& m);

	Matrix<Dimension>& setFlipY();
	Matrix<Dimension>& multFlipY();

	Matrix<Dimension>& setRenderToTexture(const render::Renderer& renderer);
	Matrix<Dimension>& multRenderToTexture(const render::Renderer& renderer);

	Matrix<Dimension>& multXYNdcToTextureCoordinates();

	/**
	 * Shears along the axis X, with amount of Y
	 */
	Matrix<Dimension>& setShearX(float amount);
	/**
	 * Shears along the axis Y, with amount of X
	 */
	Matrix<Dimension>& setShearY(float amount);

	Matrix<Dimension>& setShearZ(float amountx, float amounty);
	Matrix<Dimension>& multShearZ(float amountx, float amounty) {
		return *this *= Matrix<Dimension>().setShearZ(amountx, amounty);
	}
	Matrix<Dimension>& multShearX(float amount) {
		//TODO
		return *this *= Matrix<Dimension>().setShearX(amount);
	}
	Matrix<Dimension>& multShearY(float amount) {
		//TODO
		return *this *= Matrix<Dimension>().setShearY(amount);
	}

	operator float*() {
		return data;
	}

	operator const float*() const {
		return data;
	}

	/// 2D functions

	Matrix<Dimension>& setRotate(float rad);
	Matrix<Dimension>& setScreenDimension(float left, float top, float right, float bottom);
	Matrix<Dimension>& setScreenDimension(float width, float height);
	Matrix<Dimension>& setTranslate(float x, float y);
	Matrix<Dimension>& setScale(float x, float y);

	Matrix<Dimension>& setScreenDimension(const Vector2F& lefttop, const Vector2F& rightbottom) {
		return setScreenDimension(lefttop.x(), lefttop.y(), rightbottom.x(), rightbottom.y());
	}
	Matrix<Dimension>& setScreenDimension(const Vector2F& size) {
		return setScreenDimension(size.width(), size.height());
	}
	Matrix<Dimension>& setTranslate(const Vector2F& translate) {
		return setTranslate(translate.x(), translate.y());
	}
	Matrix<Dimension>& setTranslate(const Vector3F& translate) {
		return setTranslate(translate.x(), translate.y(), translate.z());
	}
	Matrix<Dimension>& setScale(const Vector2F& scale) {
		return setScale(scale.x(), scale.y());
	}

	Matrix<Dimension>& multScreenDimension(float left, float top, float right, float bottom);
	Matrix<Dimension>& multScreenDimension(float width, float height);
	Matrix<Dimension>& multRotate(float rad);
	Matrix<Dimension>& multTranslate(float x, float y);
	Matrix<Dimension>& multScale(float x, float y);

	Matrix<Dimension>& multScreenDimension(const Vector2F& lefttop, const Vector2F& rightbottom) {
		return multScreenDimension(lefttop.x(), lefttop.y(), rightbottom.x(), rightbottom.y());
	}
	Matrix<Dimension>& multScreenDimension(const Size2F& size) {
		return multScreenDimension(size.width(), size.height());
	}
	Matrix<Dimension>& multTranslate(const Vector2F& translate) {
		return multTranslate(translate.x(), translate.y());
	}
	Matrix<Dimension>& multTranslate(const Vector3F& translate) {
		return multTranslate(translate.x(), translate.y(), translate.z());
	}
	Matrix<Dimension>& multScale(const Vector2F& scale) {
		return multScale(scale.x(), scale.y());
	}

	Matrix<Dimension>& setRotateInverse(float rad) {
		return setRotate(-rad);
	}
	Matrix<Dimension>& setScreenDimensionInverse(float left, float top, float right, float bottom);
	Matrix<Dimension>& setTranslateInverse(float x, float y) {
		return setTranslate(-x, -y);
	}
	Matrix<Dimension>& setTranslateInverse(const Vector2F& translate) {
		return setTranslate(-translate.x(), -translate.y());
	}
	Matrix<Dimension>& setScaleInverse(float x, float y) {
		return setScale(1.0f / x, 1.0f / y);
	}
	Matrix<Dimension>& setScaleInverse(const Vector2F& scale) {
		return setScale(1.0f / scale.x(), 1.0f / scale.y());
	}

	Matrix<Dimension>& multRotateInverse(float rad) {
		return multRotate(-rad);
	}
	Matrix<Dimension>& multScreenDimensionInverse(float left, float top, float right, float bottom);
	Matrix<Dimension>& multTranslateInverse(float x, float y) {
		return multTranslate(-x, -y);
	}
	Matrix<Dimension>& multTranslateInverse(const Vector2F& translate) {
		return multTranslate(-translate.x(), -translate.y());
	}
	Matrix<Dimension>& multScaleInverse(float x, float y) {
		return multScale(1.0f / x, 1.0f / y);
	}
	Matrix<Dimension>& multScaleInverse(const Vector2F& scale) {
		return multScale(1.0f / scale.x(), 1.0f / scale.y());
	}

	/// 3D functions

	Matrix<Dimension>& multNormalizeDepthCoordinates(const render::Renderer& renderer);

	Matrix<Dimension>& setRotate(float rad, float x, float y, float z);
	Matrix<Dimension>& setTranslate(float x, float y, float z);
	Matrix<Dimension>& setScale(float x, float y, float z);
	Matrix<Dimension>& setProjection(const render::Renderer& renderer, float fovy, float aspect, float znear, float zfar);
	Matrix<Dimension>& setOrthographic(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
			float zfar);
	Matrix<Dimension>& setOrthographicInverse(const render::Renderer& renderer, float left, float right, float bottom, float top,
			float znear, float zfar);

	Matrix<Dimension>& multRotate(float rad, float x, float y, float z);
	Matrix<Dimension>& multTranslate(float x, float y, float z);
	Matrix<Dimension>& multScale(float x, float y, float z);
	Matrix<Dimension>& multProjection(const render::Renderer& renderer, float fovy, float aspect, float znear, float zfar);
	Matrix<Dimension>& multOrthographic(const render::Renderer& renderer, float left, float right, float bottom, float top, float znear,
			float zfar);
	Matrix<Dimension>& multOrthographicInverse(const render::Renderer& renderer, float left, float right, float bottom, float top,
			float znear, float zfar);

	Matrix<Dimension>& setFrustum(const render::Renderer& renderer, float left, float top, float right, float bottom, float znear,
			float zfar);
	Matrix<Dimension>& multFrustum(const render::Renderer& renderer, float left, float top, float right, float bottom, float znear,
			float zfar) {
		return *this *= Matrix<Dimension>().setFrustum(renderer, left, top, right, bottom, znear, zfar);
	}

};

Vector2F& operator *=(Vector2F& pos, const Matrix2D& matrix);
Vector2F operator *(const Vector2F& pos, const Matrix2D& matrix);

Vector3F& operator *=(Vector3F& pos, const Matrix3D& matrix);
Vector3F operator *(const Vector3F& pos, const Matrix3D& matrix);

Vector4F& operator *=(Vector4F& v, const Matrix3D& matrix);
Vector4F&& operator *=(Vector4F&& v, const Matrix3D& matrix);

Vector2F operator *(const Matrix2D& matrix, const Vector2F& pos);
Vector3F operator *(const Matrix3D& matrix, const Vector3F& pos);
}

#endif /* MATRIX_H_ */
