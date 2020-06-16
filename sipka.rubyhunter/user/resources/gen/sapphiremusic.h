#ifndef SAPPHIREMUSIC_H_
#define SAPPHIREMUSIC_H_
#include <gen/assets.h>
#include <gen/log.h>
#include <gen/fwd/types.h>
#include <gen/serialize.h>
namespace userapp {
using namespace rhfw;
enum class SapphireMusic
	: uint32 {
@foreach e in musicids.entrySet():
	@e.getKey().replaceAll("[ ]+", "_")@ = @e.getValue()@,
@
	_count_of_entries = @musicids.size()@,
};
inline rhfw::RAssetFile convertSapphireMusic(SapphireMusic music) {
	switch (music) {
	@foreach e in musicfiles.entrySet():
		case SapphireMusic::@e.getKey()@:
			return rhfw::RAssets::@e.getValue()@;
	@
		default:
			THROW()<<"Invalid music " << (unsigned int)music;
			return rhfw::RAssetFile::INVALID_ASSET_IDENTIFIER;
	}
}

extern const char* SapphireMusicStrings[];

} // namespace userapp

namespace rhfw {
using namespace userapp;

template<Endianness ENDIAN>
class SerializeExecutor<SapphireMusic, ENDIAN> {
public:
	template<typename InStream>
	static bool deserialize(InStream& is, SapphireMusic& outdata) {
		bool res = SerializeExecutor<uint32, ENDIAN>::deserialize(is, reinterpret_cast<uint32&>(outdata));
		if (res) {
			if ((unsigned int) outdata > (unsigned int) SapphireMusic::_count_of_entries) {
				return false;
			}
		}
		return res;
	}

	template<typename OutStream>
	static bool serialize(OutStream& os, const SapphireMusic& data) {
		return SerializeExecutor<uint32, ENDIAN>::serialize(os, reinterpret_cast<const uint32&>(data));
	}
};

}  // namespace rhfw

#endif /* SAPPHIREMUSIC_H_ */
