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
 * Randomer.h
 *
 *  Created on: 2016. okt. 7.
 *      Author: sipka
 */

#ifndef FRAMEWORK_RANDOM_RANDOMER_H_
#define FRAMEWORK_RANDOM_RANDOMER_H_

namespace rhfw {

class Randomer {
private:

public:
	virtual ~Randomer() {
	}

	virtual int read(void* buffer, unsigned int count) = 0;

	int generate(void* buffer, unsigned int count) {
		return read(buffer, count);
	}
};

}  // namespace rhfw

#endif /* FRAMEWORK_RANDOM_RANDOMER_H_ */
