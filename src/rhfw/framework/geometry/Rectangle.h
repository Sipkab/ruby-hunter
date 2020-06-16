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
 * Rectangle.h
 *
 *  Created on: 2015 febr. 11
 *      Author: sipka
 */

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include <framework/geometry/VertexDataHolder.h>
#include <framework/geometry/Boundary.h>
#include <framework/geometry/Vector.h>

#include <gen/configuration.h>

namespace rhfw {

class Rectangle {
public:
	static const unsigned int CONV_TO_VERTEX_DATA_DIMENSION = 2;

	float left;
	float top;
	float right;
	float bottom;

	Rectangle(float left, float top, float right, float bottom)
			: left { left }, top { top }, right { right }, bottom { bottom } {
	}
	Rectangle(const Vector2F& lefttop, const Vector2F& rightbottom)
			: Rectangle { lefttop.x(), lefttop.y(), rightbottom.x(), rightbottom.y() } {
	}
	Rectangle()
			: Rectangle { 0.0f, 0.0f, 0.0f, 0.0f } {
	}

	Vector2F middle() const {
		return Vector2F { left + (right - left) / 2.0f, top + (bottom - top) / 2.0f };
	}

	float area() const {
		return width() * height();
	}

	float height() const {
		return bottom - top;
	}
	float width() const {
		return right - left;
	}

	float aspectRatio() const {
		return width() / height();
	}

	bool isInside(const Vector2F& vec) const {
		return vec.x() >= left && vec.x() <= right && vec.y() >= top && vec.y() <= bottom;
	}

	Vector2F leftTop() const {
		return Vector2F { left, top };
	}
	Vector2F rightBottom() const {
		return Vector2F { right, bottom };
	}
	Vector2F leftBottom() const {
		return Vector2F { left, bottom };
	}
	Vector2F rightTop() const {
		return Vector2F { right, top };
	}

	Vector2F leftRight() const {
		return Vector2F { left, right };
	}
	Vector2F topBottom() const {
		return Vector2F { top, bottom };
	}

	Size2F widthHeight() const {
		return Size2F { width(), height() };
	}

	Rectangle inset(float diff) const {
		return Rectangle { left + diff, top + diff, right - diff, bottom - diff };
	}
	Rectangle inset(const Vector2F& diff) const {
		return Rectangle { left + diff.x(), top + diff.y(), right - diff.x(), bottom - diff.y() };
	}

	Rectangle translate(const Vector2F& v) const {
		return Rectangle { left + v.x(), top + v.y(), right + v.x(), bottom + v.y() };
	}

	Rectangle fitInto(const Rectangle& other) const {
		if (this->aspectRatio() < other.aspectRatio()) {
			//this is taller than the other
			float scale = this->width() / other.width();
			float pad = this->height() - other.height() * scale;
			return Rectangle { left, top + pad / 2.0f, right, bottom - pad / 2.0f };
		} else {
			//this is wider than the other
			float scale = this->height() / other.height();
			float pad = this->width() - other.width() * scale;
			return Rectangle { left + pad / 2.0f, top, right - pad / 2.0f, bottom };
		}
	}

	Rectangle scaleAtMiddle(float scale) const {
		return scaleAtMiddle(Vector2F { scale, scale });
	}

	Rectangle scaleAtMiddle(const Vector2F& scale) const {
		Vector2F mid = middle();
		return Rectangle { mid - (mid - leftTop()) * scale, mid + (rightBottom() - mid) * scale };
	}

	Rectangle intersection(const Rectangle& other) const {
		return Rectangle(left > other.left ? left : other.left, top > other.top ? top : other.top,
				right > other.right ? other.right : right, bottom > other.bottom ? other.bottom : bottom);
	}

	Rectangle centerInto(const Rectangle& other) const {
		auto center = other.middle();
		auto w = width() / 2.0f;
		auto h = height() / 2.0f;
		return Rectangle { center.x() - w, center.y() - h, center.x() + w, center.y() + h };
	}

	bool isValid() const {
		return left < right && top < bottom;
	}

	Rectangle operator+(const Rectangle& other) const {
		return Rectangle { left + other.left, top + other.top, right + other.right, bottom + other.bottom };
	}

	bool operator==(const Rectangle& other) const {
		return left == other.left && right == other.right && top == other.top && bottom == other.bottom;
	}
	bool operator!=(const Rectangle& other) const {
		return left != other.left || right != other.right || top != other.top || bottom != other.bottom;
	}

};

class VDRectangle: public Rectangle, public VertexData2D {
public:
	using Rectangle::Rectangle;
	VDRectangle() = default;
	VDRectangle(const Rectangle& o)
			: Rectangle(o) {
	}

	void fillData(float* data, unsigned int stride) const override {
		data[0 * stride + 0] = left;
		data[0 * stride + 1] = bottom;

		data[1 * stride + 0] = right;
		data[1 * stride + 1] = bottom;

		data[2 * stride + 0] = left;
		data[2 * stride + 1] = top;

		data[3 * stride + 0] = right;
		data[3 * stride + 1] = top;
	}
	unsigned int getVertexCount() const override {
		return 4;
	}
};

}

#endif /* RECTANGLE_H_ */
