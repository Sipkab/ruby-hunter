#include <gen/audiomanagers.h>
#include <gen/log.h>

#include <gen/types.h>

namespace rhfw {
namespace audio {

static AudioManager* (*audioLUT[(unsigned int) AudioConfig::_count_of_entries])() = {
	@foreach item in this.getValues().entrySet() : instantiateAudioManager<AudioConfig::@item.getKey()@>,
	@
};
AudioManager* instantiateAudioManager(AudioConfig config) {
	ASSERT(config < AudioConfig::_count_of_entries) << "Config is out of bounds";
	return audioLUT[(unsigned int) config]();
}

} // namespace audio
} // namespace rhfw
