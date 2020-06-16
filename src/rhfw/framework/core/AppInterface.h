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
 * AppInterface.h
 *
 *  Created on: 2016. febr. 23.
 *      Author: sipka
 */

#ifndef APPINTERFACE_H_
#define APPINTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

void user_app_initialize(int argc, char* argv[]);
void user_app_terminate();

#ifdef __cplusplus
}
#endif

#endif /* APPINTERFACE_H_ */
