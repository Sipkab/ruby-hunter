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
 * PseudoRandomer.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef FRAMEWORK_RANDOM_PSEUDORANDOMER_H_
#define FRAMEWORK_RANDOM_PSEUDORANDOMER_H_

#include <framework/core/timing.h>
#include <framework/random/Randomer.h>
#include <gen/fwd/types.h>

namespace rhfw {

class PseudoRandomer final : public Randomer {
	uint32 seed;
public:
	PseudoRandomer(uint32 seed)
			: seed(seed) {
	}
	void setSeed(uint32 seed) {
		this->seed = seed;
	}
	uint32 nextInt() {
		seed = (seed * 1664525 + 1013904223);
		return seed;
	}
	virtual int read(void* buffer, unsigned int count) {
		unsigned int c = count;
		uint8* ptr = reinterpret_cast<uint8*>(buffer);
		if (c % 4 != 0) {
			uint32 randomed = nextInt();
			if (c > 4) {
				//random one uint32 to the last 4 bytes.
				*reinterpret_cast<uint32*>(ptr + c - 4) = randomed;
			} else {
				for (unsigned int i = 0; i < c; ++i) {
					ptr[i] = reinterpret_cast<uint8*>(&randomed)[i];
				}
				return count;
			}
		}
		while (c >= 4) {
			*reinterpret_cast<uint32*>(ptr) = nextInt();
			c -= 4;
			ptr += 4;
		}
		return count;
	}

};

}  // namespace rhfw

#endif /* FRAMEWORK_RANDOM_PSEUDORANDOMER_H_ */
