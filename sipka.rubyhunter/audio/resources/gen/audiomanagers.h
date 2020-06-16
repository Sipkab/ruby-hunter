#ifndef GEN_AUDIOMANAGERS_H_
#define GEN_AUDIOMANAGERS_H_

#include <gen/fwd/types.h>

namespace rhfw {
namespace audio {
class AudioManager;
template<AudioConfig Config>
AudioManager* instantiateAudioManager();
AudioManager* instantiateAudioManager(AudioConfig config);
}  // namespace audio
}  // namespace rhfw

#endif /* GEN_AUDIOMANAGERS_H_ */
