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
 * SapphireUUID.cpp
 *
 *  Created on: 2016. okt. 3.
 *      Author: sipka
 */

#include <sapphire/level/SapphireUUID.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

#include <gen/types.h>
#include <gen/serialize.h>
#include <gen/log.h>

namespace rhfw {
using namespace userapp;
CREATE_ENDIAN_SERIALIZE_FUNCTION(SapphireUUID, stream, data){
	return stream.write(data.getData(), 16);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(SapphireUUID)
CREATE_ENDIAN_DESERIALIZE_FUNCTION(SapphireUUID, is, outdata){
	return is.read(outdata.getData(), 16) == 16;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(SapphireUUID)
}  // namespace rhfw
