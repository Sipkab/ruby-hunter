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
 * deserialize.cpp
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#include <framework/utils/FixedString.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

#include <gen/types.h>
#include <gen/serialize.h>
#include <gen/log.h>

namespace rhfw {

CREATE_ENDIAN_DESERIALIZE_FUNCTION(FixedString, is, outdata){
	uint32 len;
	bool success = SerializeHandler<uint32>::deserialize<ENDIAN>(is, len);
	if(!success) {
		LOGW() << "Deserialize error";
		return false;
	}
	char* array = new char[len + 1];
	int read = is.read(array, len);
	WARN(read < len) << "Failed to read full string: length: " << len << " read: " << read;
	if (len != read) {
		//failed to fully read the string, consider it as read failure
		delete[] array;
		return false;
	}
	if (read >= 0) {
		array[read] = 0;
		outdata = FixedString::make(array, read);
		return true;
	}
	return false;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(FixedString)

CREATE_ENDIAN_SERIALIZE_FUNCTION(FixedString, os, data){
	return os.serialize<uint32, ENDIAN>(data.length()) && os.write((const char*)data, data.length());
}
INSTANTIATE_SERIALIZE_FUNCTIONS(FixedString)

}  // namespace rhfw
