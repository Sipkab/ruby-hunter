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
 * AndroidRandomContext.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef ANDROIDPLATFORM_RANDOM_ANDROIDRANDOMCONTEXT_H_
#define ANDROIDPLATFORM_RANDOM_ANDROIDRANDOMCONTEXT_H_

#include <framework/random/RandomContext.h>
#include <framework/random/Randomer.h>
#include <framework/io/files/StorageFileDescriptor.h>

namespace rhfw {

class AndroidRandomContext final: public RandomContextBase {
	friend class AndroidRandomer;
protected:
	StorageFileDescriptor randomFile { "/dev/urandom" };
	FileInput* input = nullptr;

	virtual bool load() override;
	virtual void free() override;
public:

	AndroidRandomContext();
	~AndroidRandomContext();

	virtual Randomer* createRandomer() override;

};

} // namespace rhfw

#endif /* ANDROIDPLATFORM_RANDOM_ANDROIDRANDOMCONTEXT_H_ */
