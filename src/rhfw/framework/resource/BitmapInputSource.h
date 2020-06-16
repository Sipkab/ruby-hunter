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
 * BitmapInputSource.h
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#ifndef BITMAPINPUTSOURCE_H_
#define BITMAPINPUTSOURCE_H_

#include <framework/render/Texture.h>

namespace rhfw {

class BitmapInputSource: public render::TextureInputSource {
private:
	InputSource* input;

	ColorFormat format = ColorFormat::NONE;

protected:
	virtual void apply(render::Texture* texture) override;
public:
	BitmapInputSource(InputSource* input);
	BitmapInputSource(const BitmapInputSource&) = delete;
	BitmapInputSource& operator=(const BitmapInputSource&) = delete;
	~BitmapInputSource();

	ColorFormat getColorFormat() const {
		return format;
	}
	void setColorFormat(ColorFormat format) {
		this->format = format;
	}
};

} // namespace rhfw

#endif /* BITMAPINPUTSOURCE_H_ */
