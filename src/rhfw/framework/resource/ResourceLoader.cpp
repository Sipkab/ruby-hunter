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
 * ResourceLoader.cpp
 *
 *  Created on: 2015 aug. 12
 *      Author: sipka
 */

#include <framework/resource/ResourceLoader.h>
#include <gen/log.h>

namespace rhfw {

ResourceLoader::ResourceLoader() {
}

ResourceLoader::~ResourceLoader() {
	//ASSERT(toLoadList.isEmpty(), "ResourceLoader still has to load resources, call executeLoading");
}

void ResourceLoader::executeLoading() {
	/*for (auto* n : toLoadList.nodes()) {
	 //n cant be nullptr
	 Resource* res = n->get();
	 res->load();
	 n->removeLinkFromList();
	 }*/
}
}
