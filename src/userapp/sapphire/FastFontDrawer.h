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
 * FastFontDrawer.h
 *
 *  Created on: 2016. maj. 21.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_FASTFONTDRAWER_H_
#define TEST_SAPPHIRE_FASTFONTDRAWER_H_

#include <framework/geometry/Vector.h>
#include <framework/render/Texture.h>
#include <framework/resource/font/Font.h>
#include <framework/resource/Resource.h>
#include <framework/utils/LinkedList.h>
#include <gen/types.h>
#include <gen/shader/SimpleFontShader.h>
#include <appmain.h>
#include <QuadIndexBuffer.h>

namespace userapp {
using namespace rhfw;

class FastFontDrawer;
class FastFontDrawerPool {
	friend class FastFontDrawer;
public:
	AutoResource<Font> font;

	AutoResource<SimpleFontShader::MVP> mvpu = simpleFontShader->createUniform_MVP();
	AutoResource<SimpleFontShader::UTexture> textu = simpleFontShader->createUniform_UTexture();

	LinkedList<FastFontDrawer, false> drawers;

	FastFontDrawerPool() {
	}
	void prepare(Resource<Font> font, const Matrix2D& mvp);
	void commit();

};
class FastFontDrawer: private LinkedNode<FastFontDrawer> {
	friend class FastFontDrawerPool;

	unsigned int charCount = 0;
	unsigned int index = 0;
	AutoResource<render::VertexBuffer> buffer = renderer->createVertexBuffer();
	render::Buffer::StructuredInitializer<SimpleFontShader::VertexInput> initer;

	AutoResource<SimpleFontShader::InputLayout> il { simpleFontShader->createInputLayout(), [&](SimpleFontShader::InputLayout* il) {
		il->setLayout(buffer);
	} };

	FastFontDrawerPool& pool;

	virtual FastFontDrawer* get() override {
		return this;
	}
public:
	FastFontDrawer(FastFontDrawerPool& pool)
			: pool(pool) {
		pool.drawers.addToEnd(*this);
	}
	FastFontDrawer(const FastFontDrawer&) = delete;
	FastFontDrawer(FastFontDrawer&& o) = delete;
	~FastFontDrawer() {
	}

	void prepare(unsigned int charcount) {
		ASSERT(index == 0) << "Drawer wasnt committed";
		if (charCount == charcount) {
			//already initialized for this, and initer is still valid
			return;
		}

		quadIndexBuffer.ensureLength(charcount);

		initer = buffer->initializer<SimpleFontShader::VertexInput>(charcount * 4, BufferType::DYNAMIC);
		this->charCount = charcount;
		this->index = 0;
	}

	void commit();

	float add(const char* begin, const char* end, const Color& color, const Vector2F& pos, float size, Gravity gravity, float measuredlen) {
		unsigned int length = end - begin;
		ASSERT(length * 4 < charCount);
		if (length * 4 + index > charCount * 4) {
			unsigned int oldcc = charCount;
			commit();
			prepare(oldcc);
		}
		pool.font->fillBufferDataWithCharacters(begin, end, (SimpleFontShader::VertexInput*) &initer[index], size, pos, gravity,
				measuredlen, [&](SimpleFontShader::VertexInput& i, const Vector2F& pos, const Vector2F& tex) {
					i.a_position = Vector4F {pos.xy(), 0.0f, 1.0f};
					i.a_texcoord = tex;
					i.a_color = color;
				});
		index += (end - begin) * 4;
		return measuredlen;
	}

	float add(const char* begin, const char* end, const Color& color, const Vector2F& pos, float size, Gravity gravity) {
		const float pxwidth = pool.font->measureText(begin, end, size);
		return add(begin, end, color, pos, size, gravity, pxwidth);
	}

	float add(const char* text, const Color& color, const Vector2F& pos, float size, Gravity gravity) {
		unsigned int len = 0;
		const float pxwidth = pool.font->measureText(text, size, &len);
		return add(text, text + len, color, pos, size, gravity, pxwidth);
	}

	Resource<Font> getFont() {
		return pool.font;
	}

};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_FASTFONTDRAWER_H_ */
