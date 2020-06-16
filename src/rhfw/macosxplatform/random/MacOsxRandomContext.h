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
 * MacOsxRandomContext.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef MACOSXPLATFORM_RANDOM_MACOSXRANDOMCONTEXT_H_
#define MACOSXPLATFORM_RANDOM_MACOSXRANDOMCONTEXT_H_

#include <framework/random/RandomContext.h>
#include <framework/random/Randomer.h>
#include <framework/io/files/StorageFileDescriptor.h>

namespace rhfw {

class MacOsxRandomContext final: public RandomContextBase {
	friend class MacOsxRandomer;
protected:
	StorageFileDescriptor randomFile { "/dev/random" };
	FileInput* input = nullptr;

	virtual bool load() override;
	virtual void free() override;
public:

	MacOsxRandomContext();
	~MacOsxRandomContext();

	virtual Randomer* createRandomer() override;

};

} // namespace rhfw

#endif /* MACOSXPLATFORM_RANDOM_MACOSXRANDOMCONTEXT_H_ */
