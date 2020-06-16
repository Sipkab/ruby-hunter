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
 * Refreshable.h
 *
 *  Created on: 2017. aug. 14.
 *      Author: sipka
 */

#ifndef JNI_TEST_SAPPHIRE_DIALOGS_REFRESHABLE_H_
#define JNI_TEST_SAPPHIRE_DIALOGS_REFRESHABLE_H_

namespace userapp {

class Refreshable {
public:
	virtual ~Refreshable() {
	}

	virtual void refresh() = 0;
};

}  // namespace userapp

#endif /* JNI_TEST_SAPPHIRE_DIALOGS_REFRESHABLE_H_ */
