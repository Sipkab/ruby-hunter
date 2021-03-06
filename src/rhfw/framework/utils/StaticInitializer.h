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
 * StaticInitializer.h
 *
 *  Created on: 2015. nov. 29.
 *      Author: sipka
 */

#ifndef STATICINITIALIZER_H_
#define STATICINITIALIZER_H_

#include <gen/configuration.h>

namespace rhfw {

template<typename ... Types>
class StaticInitializer;

template<typename Head, typename ... Types>
class StaticInitializer<Head, Types...> {
public:
	static void initStatic() {
		Head::initStatic();
		StaticInitializer<Types...>::initStatic();
	}
	static void destroyStatic() {
		//reverse order

		StaticInitializer<Types...>::destroyStatic();
		Head::destroyStatic();
	}
};
template<>
class StaticInitializer<> {
public:
	static void initStatic() {
	}
	static void destroyStatic() {
	}
};

}

#endif /* STATICINITIALIZER_H_ */
