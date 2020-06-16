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
 * Thread.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <gen/configuration.h>
#include <gen/platform.h>

namespace rhfw {

class ThreadBase {
private:
	friend class platform_bridge;

	static void initApplicationMainThread();
public:
	static bool isApplicationMainThread();
};

} // namespace rhfw

#include THREAD_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class THREAD_EXACT_CLASS_TYPE Thread;
} // namespace rhfw

#endif /* THREAD_H_ */
