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
 * RandomContext.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef FRAMEWORK_RANDOM_RANDOMCONTEXT_H_
#define FRAMEWORK_RANDOM_RANDOMCONTEXT_H_

#include <framework/resource/ShareableResource.h>
#include <gen/platform.h>

namespace rhfw {

class Randomer;

class RandomContextBase: public ShareableResource {
protected:
public:
	virtual ~RandomContextBase() {
	}

	virtual Randomer* createRandomer() = 0;
};

}  // namespace rhfw

#include RANDOMCONTEXT_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class RANDOMCONTEXT_EXACT_CLASS_TYPE RandomContext;
} // namespace rhfw

#endif /* FRAMEWORK_RANDOM_RANDOMCONTEXT_H_ */
