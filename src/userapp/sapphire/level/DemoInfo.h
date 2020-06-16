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
 * LevelInfo.h
 *
 *  Created on: 2016. jan. 17.
 *      Author: sipka
 */

#ifndef DEMOINFO_H_
#define DEMOINFO_H_

#include <framework/utils/FixedString.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace userapp {
using namespace rhfw;
class DemoInfo {
public:
	FixedString title;
};

}

#endif /* DEMOINFO_H_ */
