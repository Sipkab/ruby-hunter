/*
 * deserializetypes.cpp
 *
 *  Created on: 2016. marc. 22.
 *      Author: sipka
 */

#include <framework/io/byteorder.h>
#include <framework/io/stream/InputStream.h>
#include <framework/io/stream/OutputStream.h>

#include <gen/types.h>
#include <gen/serialize.h>
#include <gen/log.h>

namespace rhfw {

//uint16 functions start

template<>
bool DeserializeType<Endianness::Big, uint16>(InputStream& is, uint16& outdata) {
	uint16 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::betoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Big, uint16>(OutputStream& os, const uint16& data) {
	return os.write(byteorder::htobe(data));
}
template<>
bool DeserializeType<Endianness::Little, uint16>(InputStream& is, uint16& outdata) {
	uint16 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::letoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Little, uint16>(OutputStream& os, const uint16& data) {
	return os.write(byteorder::htole(data));
}
template<>
bool DeserializeType<Endianness::Host, uint16>(InputStream& is, uint16& outdata) {
	uint16 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
template<>
bool SerializeType<Endianness::Host, uint16>(OutputStream& os, const uint16& data) {
	return os.write(data);
}

//uint16 functions end

//uint32 functions start

template<>
bool DeserializeType<Endianness::Big, uint32>(InputStream& is, uint32& outdata) {
	uint32 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::betoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Big, uint32>(OutputStream& os, const uint32& data) {
	return os.write(byteorder::htobe(data));
}
template<>
bool DeserializeType<Endianness::Little, uint32>(InputStream& is, uint32& outdata) {
	uint32 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::letoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Little, uint32>(OutputStream& os, const uint32& data) {
	return os.write(byteorder::htole(data));
}
template<>
bool DeserializeType<Endianness::Host, uint32>(InputStream& is, uint32& outdata) {
	uint32 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
template<>
bool SerializeType<Endianness::Host, uint32>(OutputStream& os, const uint32& data) {
	return os.write(data);
}

//uint32 functions end

//uint64 functions start

template<>
bool DeserializeType<Endianness::Big, uint64>(InputStream& is, uint64& outdata) {
	uint64 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::betoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Big, uint64>(OutputStream& os, const uint64& data) {
	return os.write(byteorder::htobe(data));
}
template<>
bool DeserializeType<Endianness::Little, uint64>(InputStream& is, uint64& outdata) {
	uint64 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = byteorder::letoh(val);
	return true;
}
template<>
bool SerializeType<Endianness::Little, uint64>(OutputStream& os, const uint64& data) {
	return os.write(byteorder::htole(data));
}
template<>
bool DeserializeType<Endianness::Host, uint64>(InputStream& is, uint64& outdata) {
	uint64 val;
	int read = is.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
template<>
bool SerializeType<Endianness::Host, uint64>(OutputStream& os, const uint64& data) {
	return os.write(data);
}

//uint64 functions end

CREATE_ENDIAN_DESERIALIZE_FUNCTION(int16, is, outdata){
	return DeserializeType<ENDIAN, uint16>(is, reinterpret_cast<uint16&>(outdata));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(int16)
CREATE_ENDIAN_SERIALIZE_FUNCTION(int16, stream, data){
	return SerializeType<ENDIAN, uint16>(stream, reinterpret_cast<const uint16&>(data));
}
INSTANTIATE_SERIALIZE_FUNCTIONS(int16)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(int32, is, outdata){
	return DeserializeType<ENDIAN, uint32>(is, reinterpret_cast<uint32&>(outdata));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(int32)
CREATE_ENDIAN_SERIALIZE_FUNCTION(int32, stream, data){
	return SerializeType<ENDIAN, uint32>(stream, reinterpret_cast<const uint32&>(data));
}
INSTANTIATE_SERIALIZE_FUNCTIONS(int32)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(int64, is, outdata){
	return DeserializeType<ENDIAN, uint64>(is, reinterpret_cast<uint64&>(outdata));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(int64)
CREATE_ENDIAN_SERIALIZE_FUNCTION(int64, stream, data){
	return SerializeType<ENDIAN, uint64>(stream, reinterpret_cast<const uint64&>(data));
}
INSTANTIATE_SERIALIZE_FUNCTIONS(int64)



CREATE_ENDIAN_DESERIALIZE_FUNCTION(float, stream, data){
	return DeserializeType<ENDIAN, uint32>(stream, reinterpret_cast<uint32&>(data));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(float)
CREATE_ENDIAN_SERIALIZE_FUNCTION(float, stream, data){
	return SerializeType<ENDIAN, uint32>(stream, reinterpret_cast<const uint32&>(data));
}
INSTANTIATE_SERIALIZE_FUNCTIONS(float)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(double, stream, data){
	return DeserializeType<ENDIAN, uint64>(stream, reinterpret_cast<uint64&>(data));
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(double)
CREATE_ENDIAN_SERIALIZE_FUNCTION(double, stream, data){
	return SerializeType<ENDIAN, uint64>(stream, reinterpret_cast<const uint64&>(data));
}
INSTANTIATE_SERIALIZE_FUNCTIONS(double)

//char functions start

CREATE_ENDIAN_DESERIALIZE_FUNCTION(char, stream, outdata){
	char val;
	int read = stream.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(char)
CREATE_ENDIAN_SERIALIZE_FUNCTION(char, stream, data){
	return stream.write<char>(data);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(char)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(unsigned char, stream, outdata){
	unsigned char val;
	int read = stream.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(unsigned char)
CREATE_ENDIAN_SERIALIZE_FUNCTION(unsigned char, stream, data){
	return stream.write<unsigned char>(data);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(unsigned char)

CREATE_ENDIAN_DESERIALIZE_FUNCTION(signed char, stream, outdata){
	signed char val;
	int read = stream.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val;
	return true;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(signed char)
CREATE_ENDIAN_SERIALIZE_FUNCTION(signed char, stream, data){
	return stream.write<signed char>(data);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(signed char)

//char functions end

CREATE_ENDIAN_DESERIALIZE_FUNCTION(bool, stream, outdata){
	unsigned char val;
	int read = stream.read(&val, sizeof(val));
	if(read < (int) sizeof(val)){
		//LOGW() << "Deserialization failed read result: " << read;
		return false;
	}
	outdata = val != 0;
	return true;
}
INSTANTIATE_DESERIALIZE_FUNCTIONS(bool)
CREATE_ENDIAN_SERIALIZE_FUNCTION(bool, stream, data){
	return SerializeType<ENDIAN, char>(stream, data ? 1 : 0);
}
INSTANTIATE_SERIALIZE_FUNCTIONS(bool)


}  // namespace rhfw
