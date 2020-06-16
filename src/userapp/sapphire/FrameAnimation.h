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
 * FrameAnimation.h
 *
 *  Created on: 2016. apr. 29.
 *      Author: sipka
 */

#ifndef TEST_SAPPHIRE_FRAMEANIMATION_H_
#define TEST_SAPPHIRE_FRAMEANIMATION_H_

#include <framework/geometry/Rectangle.h>
#include <framework/render/Texture.h>
#include <framework/resource/Resource.h>
#include <framework/xml/XmlAttributes.h>
#include <gen/log.h>
#include <gen/resources.h>
#include <gen/xmldecl.h>

namespace userapp {

using namespace rhfw;

class FrameAnimation: public rhfw::ShareableResource {
	friend void* rhfw::getChild<RXml::Elem::FrameAnimation>(const xml::XmlNode& parent, const xml::XmlNode& child,
			const xml::XmlAttributes& attributes);
public:
	class Element: public rhfw::ShareableResource {
		friend bool rhfw::addChild<RXml::Elem::FrameAnimation>(const xml::XmlNode& parent, const xml::XmlNode& child);
		friend class FrameAnimation;
	private:
		Resource<render::Texture> texture;
		Rectangle pos;
	protected:
		virtual bool load() override {
			return texture.load();
		}
		virtual void free() override {
			texture.free();
		}
		Element(Resource<render::Texture> texture, const Rectangle& pos)
				: texture { texture }, pos { pos } {
		}
	public:
		Element();
		Element(const xml::XmlAttributes& attrs);
		Element(const Element&) = default;
		Element(Element&&) = default;
		Element& operator=(Element&&) = default;
		Element& operator=(const Element&) = default;
		void applyAttributes(const xml::XmlAttributes& attrs);

		operator render::Texture&() const {
			return Resource<render::Texture> { texture };
		}

		Resource<render::Texture> getTexture() const {
			return texture;
		}
		const Rectangle& getPosition() const {
			return pos;
		}

		Element flippedY() const {
			return Element { texture, Rectangle { pos.right, pos.top, pos.left, pos.bottom } };
		}
		Element flippedX() const {
			return Element { texture, Rectangle { pos.left, pos.bottom, pos.right, pos.top } };
		}
	};
private:
	ResId resid;
	unsigned int childCount = 0;
	Resource<Element>* elements = nullptr;
protected:

	virtual bool load() override;
	virtual void free() override;
public:

	FrameAnimation(ResId resid);
	FrameAnimation(const FrameAnimation&) = delete;
	~FrameAnimation();
	void applyAttributes(unsigned int childcount, const xml::XmlAttributes& attrs);

	unsigned int getChildCount() const {
		return childCount;
	}

	const Element& getAtPercent(float percent) const {
		ASSERT(percent >= 0.0f && percent <= 1.0f) << "invalid percent value: " << percent;
		unsigned int index = (unsigned int) (percent * childCount);
		return elements[index >= childCount ? childCount - 1 : index];
	}

	const Element& getAtIndex(unsigned int index) const {
		ASSERT(index < childCount) << "Invalid index: " << index << " >= " << childCount;
		return elements[index];
	}

	const Element& getLast() const {
		return getAtIndex(childCount - 1);
	}
	const Element& getFirst() const {
		return getAtIndex(0);
	}
};

}  // namespace userapp

#endif /* TEST_SAPPHIRE_FRAMEANIMATION_H_ */
