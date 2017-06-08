/* zen_sdl_audio.h - v0.42 - SDL Audio wrapper -https://github.com/ZenToad/zen

   Do this:
      #define ZEN_SDL_AUDIO_IMPLEMENTATION
   before you include this file in *one* C or C++ file to create the implementation.
   // i.e. it should look like this:
   #include ...
   #include ...
   #include ...
   #define ZEN_SDL_AUDIO_IMPLEMENTATION
   #include "zen_sdl_audio.h"

	 zlib license:
	 Full license at bottom of file.

*/


#if !defined(__ZEN_SDL_AUDIO_H__)
#define __ZEN_SDL_AUDIO_H__


#if defined(ZEN_LIB_DEV)
#include "zen/zen_lib.h"
#endif


#define ZEN_AUDIO_MAX_CHANNELS 		16
#define ZEN_AUDIO_MAX_SOUNDS 			16
#define ZEN_AUDIO_SYSTEM_CHANNELS	2
#define ZEN_AUDIO_SYSTEM_FREQ			44100
#define ZEN_AUDIO_SYSTEM_SAMPLES		1024


#if defined(__cplusplus)
extern "C" {
#endif


#if defined(ZEN_AUDIO_STATIC)
#define ZAUDIODEF static
#else
#define ZAUDIODEF extern
#endif


struct ZenAudioSystem;


ZAUDIODEF ZenAudioSystem *zen_audio_create();
ZAUDIODEF void zen_audio_init(ZenAudioSystem *audio);
ZAUDIODEF void zen_audio_turn_on(ZenAudioSystem *audio);
ZAUDIODEF void zen_audio_turn_off(ZenAudioSystem *audio);
ZAUDIODEF void zen_audio_shutdown(ZenAudioSystem *audio);
ZAUDIODEF void zen_audio_destroy(ZenAudioSystem *audio);


ZAUDIODEF float dB_to_volume(float dB);
ZAUDIODEF float volume_to_dB(float volume);


ZAUDIODEF void zen_audio_load_sound(ZenAudioSystem *audio, const char *sound_name, bool looping = false);
ZAUDIODEF void zen_audio_unload_sound(ZenAudioSystem *audio, const char *sound_name);
ZAUDIODEF uint32 zen_audio_play_sound(ZenAudioSystem *audio, const char *sound_name, float volumedB = 0.0f);
ZAUDIODEF void zen_audio_stop_channel(ZenAudioSystem *audio, uint32 channel_id);
ZAUDIODEF void zen_audio_stop_all_channels(ZenAudioSystem *audio);
ZAUDIODEF void zen_audio_set_channel_volume(ZenAudioSystem *engine, uint32 channel_id, float volumedB);
ZAUDIODEF bool zen_audio_is_playing(ZenAudioSystem *engine, uint32 channel_id);


#if defined(__cplusplus)
}
#endif


#endif // __ZEN_SDL_AUDIO_H__


//------------------------------------------
//
//
// Implementation
//
//------------------------------------------
#if defined(ZEN_SDL_AUDIO_IMPLEMENTATION) || defined(ZEN_LIB_DEV)


#if defined(ZEN_LIB_DEV)
#include <string.h>
#include "SDL.h"

#include <zen/zen_lib.h>
#endif

typedef struct zen_sound {

	int is_loaded;
	const char *name;
	float *audio_data;
	int sample_count;
	bool looping;

} zen_sound;


enum FADE_TYPE {
	NOT_FADING = 0,
	FADE_IN,
	FADE_OUT
};


typedef struct zen_channel {

	uint32 id;
	zen_sound *sound;

	float position;
	float advance;

	int is_playing;
	uint32 expire;
	int loop_count;


	FADE_TYPE fade_type;
	uint32 ticks_fade;
	uint32 fade_length;
	float fade_volume_reset;
	float fade_volume;


	float left_pan;
	float right_pan;

	float gain;

} zen_channel;


typedef struct ZenAudioSystem {

	SDL_AudioSpec mixer;
	SDL_AudioDeviceID id;

	uint32 next_channel_id;
	float gain;

	zen_sound sounds[ZEN_AUDIO_MAX_SOUNDS];
	zen_channel channels[ZEN_AUDIO_MAX_CHANNELS];


} ZenAudioSystem;


static void zen_audio_reset_channel(zen_channel *channel) {

	channel->fade_type = NOT_FADING;
	channel->right_pan = 1.0f;
	channel->left_pan = 1.0f;
	channel->gain = 1.0f;
	channel->expire = 0;
	channel->loop_count = 0;
	channel->is_playing = 0;
	channel->advance = 1.0f;

}


static void mix_audio_stream(ZenAudioSystem *audio, zen_channel *c, float **out_buffer, int output_sample_count) {


	float *out = *out_buffer;
	float *in = c->sound->audio_data;
	int output_sample = 0; // buffer pos
	int channels = 2;


	// calc gain here just once
	float left_gain = audio->gain * c->gain * c->left_pan;
	float right_gain = audio->gain * c->gain * c->right_pan;

	while (output_sample < output_sample_count) {

		int index = c->position * channels;
		float left = in[index] * left_gain;
		float right = in[index + 1] * right_gain;

		*out++ += left;
		*out++ += right;

		output_sample += 1;
		c->position += c->advance; // allows for pitch change


		if (c->position >= c->sound->sample_count) {
			if (c->loop_count < 0) {
				//infinite loop
				c->position = 0;
			} else if (c->loop_count > 0) {
				c->loop_count--;
				c->position = 0;
			} else {
				c->is_playing = 0;
				return;
			}
		}
	}

}


static void mix_channel(ZenAudioSystem *audio, zen_channel *c, uint32 sdl_ticks, float **out_buffer, int out_sample_count) {


	if (c->expire > 0 && c->expire < sdl_ticks) {
		c->is_playing = 0;
		return;
	}


	if (c->fade_type != NOT_FADING) {
		uint32 ticks = sdl_ticks - c->ticks_fade;
		if (ticks < c->fade_length) {
			c->gain = c->fade_volume_reset;
			if (c->fade_type == FADE_OUT) {
				c->is_playing = 0;
				return;
			}
			c->fade_type = NOT_FADING;
		} else if (c->fade_type == FADE_OUT) {
			c->gain = c->fade_volume * (c->fade_length - ticks);
		} else { // FADE_IN
			c->gain = (c->fade_volume * ticks) / c->fade_length;
		}
	}


	mix_audio_stream(audio, c, out_buffer, out_sample_count);


}


static void zen_audio_callback(void *userdata, uint8 *output, int len) {


	GB_ASSERT_NOT_NULL(userdata);
	ZenAudioSystem *audio = (ZenAudioSystem *)userdata;
	SDL_memset(output, audio->mixer.silence, len);


	float *output_buffer = (float *)output;
	uint32 ticks = SDL_GetTicks();
	int sample_count = len / (zen_sizeof(float) * audio->mixer.channels);


	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		if (audio->channels[i].is_playing) {
			mix_channel(audio, &audio->channels[i], ticks, &output_buffer, sample_count);
		}
	}


}


ZAUDIODEF ZenAudioSystem *zen_audio_create() {

	ZenAudioSystem *audio = (ZenAudioSystem *)SDL_malloc(zen_sizeof(ZenAudioSystem));
	GB_ASSERT_NOT_NULL(audio);
	return audio;

}


ZAUDIODEF void zen_audio_init(ZenAudioSystem *audio) {

	audio->gain = 1.0f;
	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		SDL_zero(audio->channels[i]);
		zen_audio_reset_channel(&audio->channels[i]);
	}


	SDL_AudioSpec want;
	want.channels = ZEN_AUDIO_SYSTEM_CHANNELS;
	want.freq = ZEN_AUDIO_SYSTEM_FREQ;
	want.format = AUDIO_F32;
	want.callback = zen_audio_callback;
	want.userdata = audio;
	want.samples = ZEN_AUDIO_SYSTEM_SAMPLES;

	audio->id = SDL_OpenAudioDevice(NULL, 0, &want, &audio->mixer, 0);

	GB_ASSERT_MSG(audio->id, "Couldn't open audio device");
	GB_ASSERT_MSG(want.format == audio->mixer.format, "Couldn't get correct format");

}


ZAUDIODEF void zen_audio_turn_on(ZenAudioSystem *audio) {
	SDL_PauseAudioDevice(audio->id, 0);
}


ZAUDIODEF void zen_audio_turn_off(ZenAudioSystem *audio) {
	SDL_PauseAudioDevice(audio->id, 1);
}


static zen_sound *zen_audio_get_sound(ZenAudioSystem *audio, const char *name) {

	for (int i = 0; i < ZEN_AUDIO_MAX_SOUNDS; ++i) {
		zen_sound *sound = &audio->sounds[i];
		if (sound->is_loaded && (strcmp(sound->name, name) == 0)) {
			return sound;
		}
	}

	return NULL;

}


ZAUDIODEF void zen_audio_unload_sound(ZenAudioSystem *audio, const char *sound_name) {

	zen_sound *sound = zen_audio_get_sound(audio, sound_name);
	if (sound == NULL)
		return; // not loaded


	// stop any channels playing the sound
	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		zen_channel *channel = &audio->channels[i];
		if (channel->is_playing && channel->sound == sound) {
			SDL_LockAudioDevice(audio->id);
			channel->is_playing = 0;
			SDL_UnlockAudioDevice(audio->id);
		}
	}

	sound->is_loaded = 0;
	SDL_free(sound->audio_data);
	sound->audio_data = NULL;

}


ZAUDIODEF void zen_audio_shutdown(ZenAudioSystem *audio) {

	if (audio == NULL)
		return;

	SDL_PauseAudioDevice(audio->id, 1);

	for (int i = 0; i < ZEN_AUDIO_MAX_SOUNDS; ++i) {
		audio->sounds[i].is_loaded = 0;
		SDL_free(audio->sounds[0].audio_data);
		audio->sounds[i].audio_data = NULL;
	}

	SDL_CloseAudioDevice(audio->id);

}


ZAUDIODEF void zen_audio_destroy(ZenAudioSystem *audio) {
	SDL_free(audio);
}


ZAUDIODEF float dB_to_volume(float dB) {
	return powf(10.0f, 0.05f * dB);
}


ZAUDIODEF float volume_to_dB(float volume) {
	return 20.0f * log10f(volume);
}



static void zen_audio_add_sound(ZenAudioSystem *audio, const char *name, float *audio_buf, int sample_count, bool looping) {

	for (int i = 0; i < ZEN_AUDIO_MAX_SOUNDS; ++i) {
		if (!audio->sounds[i].is_loaded) {
			audio->sounds[i].is_loaded = 1;
			audio->sounds[i].name = name;
			audio->sounds[i].audio_data = audio_buf;
			audio->sounds[i].sample_count = sample_count;
			audio->sounds[i].looping = looping;
			return;
		}
	}

}


static void zen_audio_add_channel(ZenAudioSystem *audio, int channel_id, zen_sound *sound, float volumedB) {

	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		zen_channel *channel = &audio->channels[i];
		if (!channel->is_playing) {
			SDL_LockAudioDevice(audio->id);

			zen_audio_reset_channel(channel);
			channel->sound = sound;
			channel->id = channel_id;
			channel->loop_count = sound->looping ? -1 : 0;
			channel->is_playing = 1;
			channel->gain = dB_to_volume(volumedB);
			zout("Channel Added");
			zfout(channel->gain);
			SDL_UnlockAudioDevice(audio->id);
			return;
		}
	}

}


ZAUDIODEF uint32 zen_audio_play_sound(ZenAudioSystem *audio, const char *sound_name, float volumedB) {

	uint32 channel_id = audio->next_channel_id++;
	zen_sound *sound = zen_audio_get_sound(audio, sound_name);
	if (sound == NULL) {
		zen_audio_load_sound(audio, sound_name);
		sound = zen_audio_get_sound(audio, sound_name);
		if (sound == NULL) {
			return channel_id;
		}
	}

	zen_audio_add_channel(audio, channel_id, sound, volumedB);

	return channel_id;

}


static zen_channel *zen_audio_find_channel(ZenAudioSystem *audio, uint32 channel_id) {

	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		zen_channel *channel = &audio->channels[i];
		if (channel->id == channel_id) {
			return channel;
		}
	}

	return NULL;

}


ZAUDIODEF void zen_audio_stop_channel(ZenAudioSystem *audio, uint32 channel_id) {

	zen_channel *channel = zen_audio_find_channel(audio, channel_id);
	if (channel) {
		SDL_LockAudioDevice(audio->id);
		channel->is_playing = 0;
		SDL_UnlockAudioDevice(audio->id);
	}

}


ZAUDIODEF void zen_audio_stop_all_channels(ZenAudioSystem *audio) {

	SDL_LockAudioDevice(audio->id);
	for (int i = 0; i < ZEN_AUDIO_MAX_CHANNELS; ++i) {
		audio->channels[i].is_playing = 0;
	}
	SDL_UnlockAudioDevice(audio->id);

}


ZAUDIODEF void zen_audio_set_channel_volume(ZenAudioSystem *audio, uint32 channel_id, float volumedB) {

	zen_channel *channel = zen_audio_find_channel(audio, channel_id);
	if (channel) {
		SDL_LockAudioDevice(audio->id);
		channel->gain = dB_to_volume(volumedB);
		SDL_UnlockAudioDevice(audio->id);
	}

}


ZAUDIODEF bool zen_audio_is_playing(ZenAudioSystem *audio, uint32 channel_id) {

	bool is_playing = false;
	zen_channel *channel = zen_audio_find_channel(audio, channel_id);
	if (channel) {
		SDL_LockAudioDevice(audio->id);
		is_playing = channel->is_playing;
		SDL_UnlockAudioDevice(audio->id);
	}
	return is_playing;

}


ZAUDIODEF void zen_audio_load_sound(ZenAudioSystem *audio, const char *sound_name, bool looping) {

	zen_sound *sound = zen_audio_get_sound(audio, sound_name);
	if (sound) {
		return; // already loaded
	}


	SDL_AudioSpec wav_spec;
	uint8 *audio_buf = 0;
	uint32 audio_len = 0;
	GB_ASSERT_MSG(SDL_LoadWAV(sound_name, &wav_spec, &audio_buf, &audio_len),
			"Error loading %s: %s", sound_name, SDL_GetError());


	SDL_AudioCVT cvt;
	SDL_zero(cvt);

	SDL_AudioSpec *mix = &audio->mixer;
	int result = SDL_BuildAudioCVT(&cvt,
			wav_spec.format, wav_spec.channels, wav_spec.freq,
			mix->format, mix->channels, mix->freq);

	GB_ASSERT_MSG(result >= 0, "Problem building wav file conversion");

	cvt.len = audio_len;
	cvt.buf = (uint8 *)SDL_malloc(cvt.len * cvt.len_mult);
	SDL_memcpy(cvt.buf, audio_buf, audio_len);

	if (SDL_ConvertAudio(&cvt) < 0) {
		GB_PANIC("Audio conversion error: %s", SDL_GetError());
	}

	float *audio_data = (float *)cvt.buf;
	int sample_count = cvt.len_cvt / (zen_sizeof(float) * mix->channels);
	zen_audio_add_sound(audio, sound_name, audio_data, sample_count, looping);


	SDL_FreeWAV(audio_buf);


}



#endif // ZEN_SDL_AUDIO_IMPLEMENTATION

/*
  zlib license:

  Copyright (c) 2017 Timothy Wright https://github.com/ZenToad

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
