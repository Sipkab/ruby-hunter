#ifndef OPENALGLUE_H_
#define OPENALGLUE_H_

#include <OpenAL/OpenAL.h>

#define DECLARE_OPENAL_FUNCTION(name) \
	template<typename... Args> \
	auto name(Args&&... args) -> decltype(::name(rhfw::util::forward<Args>(args)...)) { \
		return ::name(rhfw::util::forward<Args>(args)...);\
	}
class OpenAlGlue {
	template<typename >
	class result_of; // not defined

	template<typename F, typename ... ArgTypes>
	class result_of<F(ArgTypes...)> {
	public:
		typedef F type;
	};
public:
DECLARE_OPENAL_FUNCTION(alcOpenDevice)
DECLARE_OPENAL_FUNCTION(alcCreateContext)
DECLARE_OPENAL_FUNCTION(alcMakeContextCurrent)
DECLARE_OPENAL_FUNCTION(alcDestroyContext)
DECLARE_OPENAL_FUNCTION(alcCloseDevice)

DECLARE_OPENAL_FUNCTION(alGenSources)
DECLARE_OPENAL_FUNCTION(alDeleteSources)
DECLARE_OPENAL_FUNCTION(alGetSourcei)
DECLARE_OPENAL_FUNCTION(alSourceUnqueueBuffers)
DECLARE_OPENAL_FUNCTION(alSourceQueueBuffers)
DECLARE_OPENAL_FUNCTION(alGenBuffers)
DECLARE_OPENAL_FUNCTION(alDeleteBuffers)
DECLARE_OPENAL_FUNCTION(alBufferData)
DECLARE_OPENAL_FUNCTION(alSourcePlay)
DECLARE_OPENAL_FUNCTION(alSourceStop)
DECLARE_OPENAL_FUNCTION(alGetError)
DECLARE_OPENAL_FUNCTION(alSourcef)

	bool load() {
		return true;
	}
	void free() {
	}
};

#endif /* OPENALGLUE_H_ */
