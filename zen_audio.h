#if !defined(__ZEN_AUDIO_H__)
#define __ZEN_AUDIO_H__


#if defined(__cplusplus)
extern "C" {
#endif


#if defined(ZEN_AUDIO_STATIC)
#define ZAUDIODEF static
#else
#define ZAUDIODEF extern
#endif


#if defined(ZEN_LIB_DEV)
#include "zen_lib/zen.h"
#include "zen_math.h"
#endif


typedef struct ZenAudioEngine {

	int id;

} ZenAudioEngine;



ZAUDIODEF ZenAudioEngine *zen_audio_create();
ZAUDIODEF void zen_audio_init(ZenAudioEngine *engine);
ZAUDIODEF void zen_audio_update(ZenAudioEngine *engine);
ZAUDIODEF void zen_audio_shutdown(ZenAudioEngine *engine);
ZAUDIODEF void zen_audio_destroy(ZenAudioEngine *engine);

ZAUDIODEF float dB_to_volume(float dB);
ZAUDIODEF float volume_to_dB(float volume);

ZAUDIODEF void zen_audio_load_sound(ZenAudioEngine *engine, const char *sound_name, bool is3d = false, bool looping = false, bool stream = false);
ZAUDIODEF void zen_audio_unload_sound(ZenAudioEngine *engine, const char *sound_name);
ZAUDIODEF void zen_audio_set_3d_listener_and_orientation(ZenAudioEngine *engine, Vector3_t position, Vector3_t lookat, Vector3_t up);
ZAUDIODEF int zen_audio_play_sound(ZenAudioEngine *engine, const char *sound_name, Vector3_t position, float volumedB = 0.0f);
ZAUDIODEF void zen_audio_stop_channel(ZenAudioEngine *engine, int channel_id);
ZAUDIODEF void zen_audio_stop_all_channels(ZenAudioEngine *engine);
ZAUDIODEF void zen_audio_set_channel_3d_position(ZenAudioEngine *engine, int channel_id, Vector3_t position);
ZAUDIODEF void zen_audio_set_channel_volume(ZenAudioEngine *engine, int channel_id, float volumedB);
ZAUDIODEF bool zen_audio_is_playing(ZenAudioEngine *engine, int channel_id);


#if defined(__cplusplus)
}
#endif


#endif //__ZEN_AUDIO_H__


//------------------------------------------
// implementation
//------------------------------------------
#if defined(ZEN_AUDIO_IMPLEMENTATION)

ZAUDIODEF float dB_to_volume(float dB) {
	return powf(10.0f, 0.05f * dB);
}

ZAUDIODEF float volume_to_dB(float volume) {
	return 20.0f * log10f(volume);
}

ZAUDIODEF ZenAudioEngine *zen_audio_create() {
	return ZEN_CALLOC(ZenAudioEngine, 1);
}

ZAUDIODEF void zen_audio_init(ZenAudioEngine *engine) {
	// TODO: setup sound library
}


ZAUDIODEF void zen_audio_shutdown(ZenAudioEngine *engine) {
	// TODO: shutdown sound library
}


ZAUDIODEF void zen_audio_update(ZenAudioEngine *engine) {

	// still need a hashmap, and I am trying to
	// not use malloc/free?  Why not?  We already
	// have an awesome stb_arr!
	// just add zen_hash_map to zen.h. Can always override
	// the malloc() and free() if we need to later.  For now,
	// no worries
	// need some fancy replacement for the vector
	vector<ChannelMap::iterator> stoppedChannels;

	for(auto it = channels.begin(), itEnd = channels.end(); it != itEnd; ++it) {
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);
		if(!isPlaying) {
			stoppedChannels.push_back(it);
		}
	}

	for(auto &it : stoppedChannels) {
		channels.erase(it);
	}

	// TODO: here we
	// system->Update();

}


ZAUDIODEF void zen_audio_destroy(ZenAudioEngine *engine) {
	free(engine);
}


ZAUDIODEF void zen_audio_load_sound(ZenAudioEngine *engine, const char *sound_name, bool is3d = false, bool looping = false, bool stream = false);
ZAUDIODEF void zen_audio_unload_sound(ZenAudioEngine *engine, const char *sound_name);
ZAUDIODEF void zen_audio_set_3d_listener_and_orientation(ZenAudioEngine *engine, Vector3_t position, Vector3_t lookat, Vector3_t up);
ZAUDIODEF int zen_audio_play_sound(ZenAudioEngine *engine, const char *sound_name, Vector3_t position, float volumedB = 0.0f);
ZAUDIODEF void zen_audio_stop_channel(ZenAudioEngine *engine, int channel_id);
ZAUDIODEF void zen_audio_stop_all_channels(ZenAudioEngine *engine);
ZAUDIODEF void zen_audio_set_channel_3d_position(ZenAudioEngine *engine, int channel_id, Vector3_t position);
ZAUDIODEF void zen_audio_set_channel_volume(ZenAudioEngine *engine, int channel_id, float volumedB);
ZAUDIODEF bool zen_audio_is_playing(ZenAudioEngine *engine, int channel_id);

#endif //ZEN_AUDIO_IMPLEMENTATION
