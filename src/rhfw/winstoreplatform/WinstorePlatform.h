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
 */
#ifndef WINSTOREPLATFORM_H_
#define WINSTOREPLATFORM_H_

#include <framework/utils/BasicGlobalListener.h>
#include <gen/configuration.h>

namespace rhfw {
namespace windowsstore {

class ApplicationStateListener: public BasicGlobalListener<ApplicationStateListener> {
private:
public:
	virtual void applicationSuspending() {
	}
	virtual void applicationResuming() {
	}
};

} // namespace windowsstore
} // namespace rhfw

#endif /* WINSTOREPLATFORM_H_ */
