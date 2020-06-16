#ifndef RHFW_GEN_SERIALIZE_H_
#define RHFW_GEN_SERIALIZE_H_

#include <gen/fwd/types.h>
#include <framework/utils/utility.h>

//deserializing and serializing functions
namespace rhfw {
class InputStream;
class OutputStream;

//these methods return false on error.

template<Endianness ENDIAN, typename T>
bool DeserializeType(InputStream& is, T& outdata);
template<Endianness ENDIAN, typename T>
bool SerializeType(OutputStream& os, const T& data);

template<typename T>
class EndianSerializerHelper {
public:
	template<Endianness ENDIAN>
	static bool DeserializeEndian(InputStream& is, T& outdata);
	template<Endianness ENDIAN>
	static bool SerializeEndian(OutputStream& os, const T& data);
};
#define CREATE_ENDIAN_DESERIALIZE_FUNCTION(type, isname, outdataname) \
	template<> \
	template<Endianness ENDIAN> \
	bool EndianSerializerHelper<type>::DeserializeEndian(InputStream& isname, type& outdataname)

#define CREATE_ENDIAN_SERIALIZE_FUNCTION(type, isname, outdataname) \
	template<> \
	template<Endianness ENDIAN> \
	bool EndianSerializerHelper<type>::SerializeEndian(OutputStream& isname, const type& outdataname)

#define INSTANTIATE_DESERIALIZE_FUNCTIONS(type) \
	template<> bool DeserializeType<Endianness::Big>(InputStream& is, type& outdata) { return EndianSerializerHelper<type>::DeserializeEndian<Endianness::Big>(is, outdata); } \
	template<> bool DeserializeType<Endianness::Little>(InputStream& is, type& outdata) { return EndianSerializerHelper<type>::DeserializeEndian<Endianness::Little>(is, outdata); } \
	template<> bool DeserializeType<Endianness::Host>(InputStream& is, type& outdata) { return EndianSerializerHelper<type>::DeserializeEndian<Endianness::Host>(is, outdata); }

#define INSTANTIATE_SERIALIZE_FUNCTIONS(type) \
	template<> bool SerializeType<Endianness::Big>(OutputStream& is, const type& outdata) { return EndianSerializerHelper<type>::SerializeEndian<Endianness::Big>(is, outdata); } \
	template<> bool SerializeType<Endianness::Little>(OutputStream& is, const type& outdata) { return EndianSerializerHelper<type>::SerializeEndian<Endianness::Little>(is, outdata); } \
	template<> bool SerializeType<Endianness::Host>(OutputStream& is, const type& outdata) { return EndianSerializerHelper<type>::SerializeEndian<Endianness::Host>(is, outdata); }

template<typename T, Endianness ENDIAN>
class SerializeExecutor {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, T& outdata) {
		return DeserializeType<ENDIAN>(is, outdata);
	}
	template<typename OutStream>
	static bool serialize(OutStream& os, const T& data) {
		return SerializeType<ENDIAN>(os, data);
	}
};

template<typename T>
class SerializeIsEndianIndependent {
public:
	static const bool value = false;
};

template<typename T>
class SerializeHandler {
public:
	template<Endianness ENDIAN = (Endianness) 0, typename ... Args, typename U = T, typename util::enable_if<
			SerializeIsEndianIndependent<U>::value>::type* = nullptr>
	static bool deserialize(Args&&... args) {
		return SerializeExecutor<T, ENDIAN>::deserialize(util::forward<Args>(args)...);
	}
	template<Endianness ENDIAN = (Endianness) 0, typename ... Args, typename U = T, typename util::enable_if<
			SerializeIsEndianIndependent<U>::value>::type* = nullptr>
	static bool serialize(Args&&... args) {
		return SerializeExecutor<T, ENDIAN>::serialize(util::forward<Args>(args)...);
	}

	template<Endianness ENDIAN, typename ... Args, typename U = T,
			typename util::enable_if<!SerializeIsEndianIndependent<U>::value>::type* = nullptr>
	static bool deserialize(Args&&... args) {
		return SerializeExecutor<T, ENDIAN>::deserialize(util::forward<Args>(args)...);
	}
	template<Endianness ENDIAN, typename ... Args, typename U = T,
			typename util::enable_if<!SerializeIsEndianIndependent<U>::value>::type* = nullptr>
	static bool serialize(Args&&... args) {
		return SerializeExecutor<T, ENDIAN>::serialize(util::forward<Args>(args)...);
	}
};

template<>
class SerializeIsEndianIndependent<char> {
public:
	static const bool value = true;
};
template<>
class SerializeIsEndianIndependent<signed char> {
public:
	static const bool value = true;
};
template<>
class SerializeIsEndianIndependent<unsigned char> {
public:
	static const bool value = true;
};
template<>
class SerializeIsEndianIndependent<bool> : public SerializeIsEndianIndependent<char> {
};
}  // namespace rhfw

#endif
