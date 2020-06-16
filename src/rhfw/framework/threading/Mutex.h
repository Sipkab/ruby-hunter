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
 * Mutex.h
 *
 *  Created on: 2015 aug. 10
 *      Author: sipka
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <gen/configuration.h>

#include MUTEX_EXACT_CLASS_INCLUDE
namespace rhfw {
typedef class MUTEX_EXACT_CLASS_TYPE Mutex;
} // namespace rhfw

namespace rhfw {

class MutexLocker {
private:
	Mutex& mutex;
public:
	MutexLocker(Mutex& mutex)
			: mutex(mutex) {
		mutex.lock();
	}
	~MutexLocker() {
		mutex.unlock();
	}
};

} // namespace rhfw

#endif /* MUTEX_H_ */
