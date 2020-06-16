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
 * View.cpp
 *
 *  Created on: 2014.06.08.
 *      Author: sipka
 */

#include <framework/layer/Layer.h>

#include <gen/log.h>
#include <gen/xmldecl.h>

using namespace rhfw;

LINK_XML_SIMPLE(Layer)

namespace rhfw {

Layer::Layer()
		: flags(LayerOptions::NO_FLAG) {
}

} // namespace rhfw
