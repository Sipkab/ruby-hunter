#ifndef OPENALGLUE_H_
#define OPENALGLUE_H_

#include <AL/al.h>
#include <AL/alc.h>
#include <framework/utils/utility.h>
#include <dlfcn.h>
#include <gen/log.h>

#define DECLARE_OPENAL_FUNCTION(name)\
	private:\
	decltype(&name) proto_##name = nullptr;\
	public:\
	template<typename... Args>\
	auto name(Args&&... args) -> decltype(::name(rhfw::util::forward<Args>(args)...)) { \
		return proto_##name(rhfw::util::forward<Args>(args)...);\
	}

#define LOAD_OPENAL_FUNCTION(name) proto_##name = (decltype(proto_##name)) dlsym(libhandle, STRINGIZE(name)); ASSERT(proto_##name != nullptr);

class OpenAlGlue {
	template<class >
	class result_of; // not defined

	template<class F, class ... ArgTypes>
	class result_of<F(ArgTypes...)> {
	public:
		typedef F type;
	};
	void* libhandle = nullptr;
public:
DECLARE_OPENAL_FUNCTION(alcOpenDevice)
DECLARE_OPENAL_FUNCTION(alcCreateContext)
DECLARE_OPENAL_FUNCTION(alcMakeContextCurrent)
DECLARE_OPENAL_FUNCTION(alcDestroyContext)
DECLARE_OPENAL_FUNCTION(alcCloseDevice)
DECLARE_OPENAL_FUNCTION(alcGetError)

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
		libhandle = dlopen("libopenal.so.1", RTLD_LAZY);
		if (libhandle == nullptr) {
			LOGE() << "libopenal.so.1 not found";
			libhandle = dlopen("libopenal.so.0", RTLD_LAZY);
			if (libhandle == nullptr) {
				LOGE() << "libopenal.so.0 not found";
				libhandle = dlopen("libopenal.so", RTLD_LAZY);
				if (libhandle == nullptr) {
					LOGE() << "libopenal not found";
					return false;
				}
			}
		}
		LOAD_OPENAL_FUNCTION(alcOpenDevice)
		LOAD_OPENAL_FUNCTION(alcCreateContext)
		LOAD_OPENAL_FUNCTION(alcMakeContextCurrent)
		LOAD_OPENAL_FUNCTION(alcDestroyContext)
		LOAD_OPENAL_FUNCTION(alcCloseDevice)
		LOAD_OPENAL_FUNCTION(alcGetError)

		LOAD_OPENAL_FUNCTION(alGenSources)
		LOAD_OPENAL_FUNCTION(alDeleteSources)
		LOAD_OPENAL_FUNCTION(alGetSourcei)
		LOAD_OPENAL_FUNCTION(alSourceUnqueueBuffers)
		LOAD_OPENAL_FUNCTION(alSourceQueueBuffers)
		LOAD_OPENAL_FUNCTION(alGenBuffers)
		LOAD_OPENAL_FUNCTION(alDeleteBuffers)
		LOAD_OPENAL_FUNCTION(alBufferData)
		LOAD_OPENAL_FUNCTION(alSourcePlay)
		LOAD_OPENAL_FUNCTION(alSourceStop)
		LOAD_OPENAL_FUNCTION(alGetError)
		LOAD_OPENAL_FUNCTION(alSourcef)
		return true;
	}
	void free() {
		dlclose(libhandle);
	}
};

#endif /* OPENALGLUE_H_ */
