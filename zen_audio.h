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
#include "zen_lib/zen_math.h"
#endif

struct ZenAudioEngine;


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


typedef struct FMODSound {

	void release() {  }

} FMODSound;


stb_declare_hash(static, ZenAudioSounds, zen_audio_sounds_, const char*, FMODSound*);
stb_define_hash_vnull(ZenAudioSounds, zen_audio_sounds_, const char*, NULL, NULL, return stb_hash((char *)k);, FMODSound*, NULL); 


typedef struct FMODChannel {

	void isPlaying(bool *playing) {
		*playing = false;
	}

} FMODChannel;

stb_declare_hash(static, ZenAudioChannels, zen_audio_channels_, int, FMODChannel *);
stb_define_hash(ZenAudioChannels, zen_audio_channels_, int, 0, 0, return stb_hash_number((unsigned int)k);, FMODChannel *);


typedef struct FakeSystem {

	void update() {  }
	void createSound(const char *name, int mode, void *, FMODSound **sound) {

	}

} FakeSystem;


typedef struct ZenAudioEngine {

	FakeSystem *system = 0;
	ZenAudioSounds *sounds;
	ZenAudioChannels *channels;
	int next_channel_id = 0;
	int *stopped_channels = NULL;

} ZenAudioEngine;


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

	int k;
	FMODChannel *v;	

	stb_arr_empty(engine->stopped_channels);
	for (int i = 0; i < engine->channels->limit; ++i) {
		k = engine->channels->table[i].k;
		v = engine->channels->table[i].v;
		if (v) {
			bool is_playing = false;
			v->isPlaying(&is_playing);
			if (!is_playing) {
				stb_arr_push(engine->stopped_channels, k);
			}
		}
	}

	for (int i = 0; i < stb_arr_len(engine->stopped_channels); ++i) {
		zen_audio_channels_remove(engine->channels, engine->stopped_channels[i], NULL);
	}
	stb_arr_empty(engine->stopped_channels);


	engine->system->update();

}


ZAUDIODEF void zen_audio_destroy(ZenAudioEngine *engine) {
	stb_arr_free(engine->stopped_channels);
	free(engine);
}


ZAUDIODEF void zen_audio_load_sound(ZenAudioEngine *engine, const char *sound_name, bool is3d, bool looping, bool stream) {

	FMODSound *sound = zen_audio_sounds_get(engine->sounds, sound_name);
	if (sound) {
		// already loaded
		return;
	}

	// pretend make sound
	int mode = 0;
	mode |= is3d ? 0x1 : 0x2;
	mode |= looping ? 0x4 : 0x8;
	mode |= stream ? 0xF : 0x10;
	engine->system->createSound(sound_name, mode, NULL, &sound);


	if (sound) {
		zen_audio_sounds_add(engine->sounds, sound_name, sound);
	}

}


ZAUDIODEF void zen_audio_unload_sound(ZenAudioEngine *engine, const char *sound_name) {

	FMODSound *sound = zen_audio_sounds_get(engine->sounds, sound_name);
	if (!sound)
		return; // not loaded

	zen_audio_sounds_remove(engine->sounds, sound_name, NULL);
	sound->release();

}


ZAUDIODEF int zen_audio_play_sound(ZenAudioEngine *engine, const char *sound_name, Vector3_t position, float volumedB) {


	int channel_id = engine->next_channel_id++;
	FMODSound *sound = zen_audio_sounds_get(engine->sounds, sound_name);
	if (!sound) {
		zen_audio_load_sound(engine, sound_name);
		sound = zen_audio_sounds_get(engine->sounds, sound_name);
		if (!sound) {
			return channel_id; // important
		}
	}

	FMODChannel *channel = NULL;

	//FMOD::Channel *channel = NULL;
	//impl->system->playSound(FMOD_CHAMMEL_FREE, foundIt->second, true, &channel); // true here to not play sound - could pop
	//if (channel) {
		//FMOD_VECTOR pos = VectorToFmod(position);
		//channel->set3DAttributes(&pos, NULL);
		//channel->setVolume(dBToVolume(volumedB));
		//channel->setPaused(false); // now we unpause
		//impl->channels[channelId] = channel;
	//}
	
	return channel_id;

}


//ZAUDIODEF void zen_audio_set_3d_listener_and_orientation(ZenAudioEngine *engine, Vector3_t position, Vector3_t lookat, Vector3_t up);
//ZAUDIODEF void zen_audio_stop_channel(ZenAudioEngine *engine, int channel_id);
//ZAUDIODEF void zen_audio_stop_all_channels(ZenAudioEngine *engine);
//ZAUDIODEF void zen_audio_set_channel_3d_position(ZenAudioEngine *engine, int channel_id, Vector3_t position);
//ZAUDIODEF void zen_audio_set_channel_volume(ZenAudioEngine *engine, int channel_id, float volumedB);
//ZAUDIODEF bool zen_audio_is_playing(ZenAudioEngine *engine, int channel_id);

#endif //ZEN_AUDIO_IMPLEMENTATION
