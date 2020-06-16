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
 * RegistrationToken.cpp
 *
 *  Created on: 2016. dec. 30.
 *      Author: sipka
 */

#include <sapphire/common/RegistrationToken.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

#include <gen/types.h>
#include <gen/serialize.h>
#include <gen/log.h>

namespace rhfw {
using namespace userapp;
CREATE_ENDIAN_SERIALIZE_FUNCTION(RegistrationToken, stream, data){
	return stream.write(data.data, 256);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(RegistrationToken)
CREATE_ENDIAN_DESERIALIZE_FUNCTION(RegistrationToken, is, outdata){
	return is.read(outdata.data, 256) == 256;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(RegistrationToken)
}  // namespace rhfw
