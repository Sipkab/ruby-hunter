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
 * PackageResource.cpp
 *
 *  Created on: 2016. marc. 7.
 *      Author: sipka
 */

#include <framework/resource/PackageResource.h>
#include <framework/resource/font/Font.h>

#include <gen/xmlcompile.h>
#include <gen/serialize.h>

namespace rhfw {

PackageResource::ConditionalObject<ResourcePool<ShareableResource>, (ResIds::RESOURCE_COUNT > 0)> PackageResource::packageResources {
		ResIds::RESOURCE_COUNT };

CREATE_ENDIAN_DESERIALIZE_FUNCTION(ResId, is, outdata){
	return SerializeHandler<uint32>::deserialize<ENDIAN>(is, reinterpret_cast<uint32&>(outdata));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(ResId)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(Resource<Font>, is, outdata){
	ResId id;
	bool success = SerializeHandler<ResId>::deserialize<ENDIAN>(is, id);
	if(!success) {
		LOGW() << "Deserialize error";
		return false;
	}
	outdata = PackageResource::getResourceOrFactory<Font>(id, [=] {return Resource<Font> {new ResourceBlock {new Font(id)}};});
	return true;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(Resource<Font>)

}  // namespace rhfw
