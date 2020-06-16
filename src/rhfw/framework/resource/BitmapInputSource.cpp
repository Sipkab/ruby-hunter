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
 * BitmapInputSource.cpp
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#include <framework/resource/BitmapInputSource.h>
#include <framework/utils/utility.h>

#include <libpng/png.h>
#include <gen/log.h>

namespace rhfw {

BitmapInputSource::BitmapInputSource(InputSource* input)
		: input { input } {
}

BitmapInputSource::~BitmapInputSource() {
	delete input;
}

static png_uint_32 convertFormat(ColorFormat format) {
	switch (format) {
		case ColorFormat::RGBA_8888:
			return PNG_FORMAT_RGBA;
		case ColorFormat::A_8:
			return PNG_FORMAT_GRAY;
		default: {
			THROW()<< "Unsupported color format: " << format;
			return PNG_FORMAT_RGBA;
		}
	}
}

void BitmapInputSource::apply(render::Texture* texture) {
	auto read = input->getData();

	png_image image { 0 };
	image.version = PNG_IMAGE_VERSION;

	int pngsuccess = png_image_begin_read_from_memory(&image, read, read.getLength());
	if (pngsuccess == 0) {
		LOGWTF()<< "png error " << image.message << ", image.error: " << image.warning_or_error;
		return;
	}
	image.format = convertFormat(format);

	unsigned int byteslen = PNG_IMAGE_SIZE(image);

	png_bytep buffer = new png_byte[byteslen];

	pngsuccess = png_image_finish_read(&image, nullptr/*background*/, buffer, 0/*row_stride*/, nullptr/*colormap*/);
	if (pngsuccess == 0) {
		LOGWTF()<< "png error " << image.message << ", image.error: " << image.warning_or_error;
		delete[] buffer;
		return;
	}

	asInitializer(texture)->initWithData(image.width, image.height, format, buffer);

	delete[] buffer;
}

} // namespace rhfw
