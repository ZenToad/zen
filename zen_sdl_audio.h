#include <SDL.h>
#include <signal.h>
#include "zen_lib.h"


#if !defined(MAX_AUDIO_CHANNELS)
#define MAX_AUDIO_CHANNELS 16
#endif


#if !defined(MAX_AUDIO_SOUNDS)
#define MAX_AUDIO_SOUNDS 16
#endif


typedef struct zen_sound {

	int is_loaded;
	const char *name;
	float *audio_data;
	int sample_count;

} zen_sound;


enum FADE_TYPE {
	NOT_FADING = 0,
	FADE_IN,
	FADE_OUT
};


typedef struct zen_channel {

	int in_use;
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


typedef struct AudioSystem {

	SDL_AudioSpec mixer;
	SDL_AudioDeviceID id;

	uint32 next_channel_id;
	float gain;

	zen_sound sounds[MAX_AUDIO_SOUNDS];
	zen_channel channels[MAX_AUDIO_CHANNELS];


} AudioSystem;



void reset_channel(zen_channel *channel) {

	channel->fade_type = NOT_FADING;
	channel->right_pan = 1.0f;
	channel->left_pan = 1.0f;
	channel->gain = 1.0f;
	channel->expire = 0;
	channel->loop_count = 0;
	channel->is_playing = 0;
	channel->position = 0.0f;
	channel->advance = 1.0f;

}


void stop_channel(zen_channel *channel) {

	channel->position = 0;
	channel->is_playing = 0;
	channel->fade_type = NOT_FADING;
	channel->expire = 0;
	channel->loop_count = 0;

}


void mix_audio_stream(AudioSystem *audio, zen_channel *c, float **out_buffer, int output_sample_count) {


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
				stop_channel(c);
				return;
			} 
		}


	}


}


void mix_channel(AudioSystem *audio, zen_channel *c, uint32 sdl_ticks, float **out_buffer, int out_sample_count) {


	if (c->expire > 0 && c->expire < sdl_ticks) {
		stop_channel(c);
		return;
	}


	if (c->fade_type != NOT_FADING) {
		uint32 ticks = sdl_ticks - c->ticks_fade; 
		if (ticks < c->fade_length) {
			c->gain = c->fade_volume_reset;
			if (c->fade_type == FADE_OUT) {
				stop_channel(c);
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


void zen_audio_callback(void *userdata, uint8 *output, int len) {


	GB_ASSERT_NOT_NULL(userdata);
	AudioSystem *audio = (AudioSystem *)userdata;
	SDL_memset(output, audio->mixer.silence, len);


	float *output_buffer = (float *)output;
	uint32 ticks = SDL_GetTicks();
	int sample_count = len / (zen_sizeof(float) * audio->mixer.channels);


	for (int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
		if (audio->channels[i].in_use && audio->channels[i].is_playing) {
			mix_channel(audio, &audio->channels[i], ticks, &output_buffer, sample_count);	
		}
	}


}


zen_sound *audio_get_sound(AudioSystem *audio, const char *name) {

	for (int i = 0; i < MAX_AUDIO_SOUNDS; ++i) {
		zen_sound *sound = &audio->sounds[i];
		if (sound->is_loaded && (strcmp(sound->name, name) == 0)) {
			return sound;	
		}
	}

	return NULL;

}


void audio_add_sound(AudioSystem *audio, const char *name, float *audio_buf, int sample_count) {

	for (int i = 0; i < MAX_AUDIO_SOUNDS; ++i) {
		if (!audio->sounds[i].is_loaded) {
			audio->sounds[i].is_loaded = 1;
			audio->sounds[i].name = name;
			audio->sounds[i].audio_data = audio_buf;
			audio->sounds[i].sample_count = sample_count;
		}
	}

}


void audio_load_sound(AudioSystem *audio, const char *name) {

	zen_sound *sound = audio_get_sound(audio, name);
	if (sound) {
		return; // already loaded
	}


	SDL_AudioSpec wav_spec;
	uint8 *audio_buf = 0;
	uint32 audio_len = 0;
	GB_ASSERT_NOT_NULL(SDL_LoadWAV(name, &wav_spec, &audio_buf, &audio_len));


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
	audio_add_sound(audio, name, audio_data, sample_count);


	SDL_FreeWAV(audio_buf);


}

void audio_add_channel(AudioSystem *audio, int channel_id, zen_sound *sound, int loop_count) {

	for (int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
		zen_channel *channel = &audio->channels[i];
		if (!channel->in_use) {
			SDL_LockAudioDevice(audio->id); {

				reset_channel(channel);
				channel->sound = sound;
				channel->id = channel_id;
				channel->loop_count = loop_count;

			} SDL_UnlockAudioDevice(audio->id);
		}
	}

}


int audio_play_sound(AudioSystem *audio, const char *name, int loop_count) {
 
	int channel_id = audio->next_channel_id++;
	zen_sound *sound = audio_get_sound(audio, name);
	if (!sound) {
		audio_load_sound(audio, name);
		sound = audio_get_sound(audio, name);
		if (!sound) {
			return channel_id;
		}
	}

	audio_add_channel(audio, channel_id, sound, loop_count);

	return channel_id;

}


AudioSystem *create_audio_system() {

	AudioSystem *audio = (AudioSystem *)SDL_malloc(zen_sizeof(AudioSystem));
	GB_ASSERT_NOT_NULL(audio);

	audio->gain = 1.0f;
	for (int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
		reset_channel(&audio->channels[i]);
	}


	SDL_AudioSpec want;
	want.channels = 2;
	want.freq = 44100;
	want.format = AUDIO_F32;
	want.callback = zen_audio_callback;

	audio->id = SDL_OpenAudioDevice(NULL, 0, &want, &audio->mixer, 0);

	GB_ASSERT_MSG(audio->id, "Couldn't open audio device");
	GB_ASSERT_MSG(want.format == audio->mixer.format, "Couldn't get correct format");

	return audio;

}


int audio_is_playing(AudioSystem *audio, uint32 channel_id) {

	for (int i = 0; i < MAX_AUDIO_CHANNELS; ++i) {
		if (audio->channels[i].id == channel_id) {
			return audio->channels[i].is_playing;
		}
	}

	return 0;

}

void destroy_audio_system(AudioSystem *audio) {

	if (audio == NULL)
		return;

	for (int i = 0; i < MAX_AUDIO_SOUNDS; ++i) {
		if (audio->sounds[i].audio_data) {
			SDL_free(audio->sounds[i].audio_data);
		}
	}

	SDL_CloseAudioDevice(audio->id);
	SDL_free(audio);

}


int main(int argc char *argv[]){

	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		GB_PANIC("Unable to init SDL: %s", SDL_GetError());
	}

	AudioSystem *audio = create_audio_system();
	GB_ASSERT_NOT_NULL(audio);

	audio_load_sound(audio, "path/to/file1");
	audio_load_sound(audio, "path/to/file2");
	audio_load_sound(audio, "path/to/file3");


	SDL_PauseAudioDevice(audio->id, 0);

	uint32 sound1 = audio_play_sound(audio, "path/to/file1", 2);
	uint32 sound1 = audio_play_sound(audio, "path/to/file1", 2);
	uint32 sound1 = audio_play_sound(audio, "path/to/file1", 2);

	

	destroy_audio_system(audio);
	SDL_Quit();

	return 0;

}

