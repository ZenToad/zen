#include <SDL.h>
#include <signal.h>
#include "zen_lib.h"


SDL_AudioSpec mixer;

typedef struct zen_channel {

	int audio_format;
	void *data;
	int is_playing;
	int position;

} zen_channel;


int max_channels = 10;
zen_channel *channel_list[10];
int sample_size = 4;
int channels = 2;


float zen_get_sample(zen_channel* channel, unsigned int position) {
  switch (channel->audio_format) {
    case AUDIO_S8:
      return (float)((int8*)channel->data)[position];
    case AUDIO_U8:
      return (float)((uint8*)channel->data)[position];
    case AUDIO_S16:
      return (float)((int16*)channel->data)[position];
    case AUDIO_U16:
      return (float)((uint16*)channel->data)[position];
    case AUDIO_S32:
      return (float)((int32*)channel->data)[position];
    case AUDIO_F32:
      return ((float*)channel->data)[position];
    default:
      return 0.0f;
  }
}

/*
 *
 * Things done in SDL_mixer.mix
 * loop through samples 
 * if channel not paused...
 *
 * A channel can have an expiration in ticks.
 *
 * if the expiration time is not 0 (no expire) and expire < ticks
 * 	stop channel
 *
 * else if the channel is fading (in|out)
 * 	if fade length reached
 * 		reset volume
 * 		if was fading out
 * 			stop channel
 * 		set channel to not fading
 * 	else if fading out
 * 		fade volume down
 * 	else
  			fade volume up
 * 	
 * if channel is playing (it may have stopped by now)
 * 	if channel has no more data
 * 		if channel is streaming
 * 			reset data pointer to start
 * 		if looping
 *				reset data pointer to start
 *				if not infinite loop
 *					decrement loop counter 					
 * 		else
 * 			stop channel
 *
 * 	if channel is streaming
 * 		if need more data
 * 			get more data
 *
 * 	we will restart loop if out of data, not has data
 * 	if channel has more data
 * 		if channel is looping
 * 			reset data to start
 * 			if not infinite loop
 * 				decrement loop count
 *
 *       //TODO - volume, pitch and stuff?
 *			// processing?
 * 		add next data
 * 		increment data pointer
 *
 * 	else // no more data
 *
 * 		stop channel	
 * 				
 */

void zen_audio_callback(void *userdata, uint8 *output, int len) {

	int8* out_s8 = (int8*)output;
	uint8* out_u8 = (uint8*)output;
	int16* out_s16 = (int16*)output;
	uint16* out_u16 = (uint16*)output;
	int32* out_s32 = (int32*)output;
	float* out_float = (float*)output;


	SDL_memset(output, mixer.silence, len);

	int samples = len / (sample_size * channels);
	float left, right, sample;

	for (int i = 0; i < samples; ++i) {

		left = right = 0.0f;

		for (int chan = 0; chan - max_channels; ++chan) {
			zen_channel *channel = channel_list[chan];
			if (channel->is_playing) {
				uint32 position = channel->position;
				sample = zen_get_sample(channel, position);
				right += sample; // adjust gain and stuff here?
				left += sample; // adjust gain and stuff here?
				channel->position += 1; // not sure how this works...
			}
		}

		switch (mixer.format) {
			case AUDIO_S8:
				*out_s8++ = (int8)(zen_clamp(left, -128, 127));
				*out_s8++ = (int8)(zen_clamp(right, -128, 127));
				break;
			case AUDIO_U8:
				*out_u8++ = (uint8)(zen_clamp(left, 0, 255));
				*out_u8++ = (uint8)(zen_clamp(right, 0, 255));
				break;
			case AUDIO_S16:
				*out_s16++ = (int16)(zen_clamp(left, -32768, 32767));
				*out_s16++ = (int16)(zen_clamp(right, -32768, 32767));
				break;
			case AUDIO_U16: 
				*out_u16++ = (uint16)(zen_clamp(left, 0, 65535));
				*out_u16++ = (uint16)(zen_clamp(right, 0, 65535));
				break;
			case AUDIO_S32:
				*out_s32++ = (int32)(zen_clamp(left, -2147483648.0f, 2147483647.0f));
				*out_s32++ = (int32)(zen_clamp(right, -2147483648.0f, 2147483647.0f));
				break;
			case AUDIO_F32:
				*out_float++ = left;
				*out_float++ = right;
				break;
		}

	}


}

void ideas() {

	/*
	 * Just use SDL_OpenAudioDevice.  For now, no format changes.
	 * we can worry about all that stuff later.
	 */

	SDL_AudioSpec want;
	SDL_AudioDeviceID dev;

	SDL_memset(&want, 0, sizeof(SDL_AudioSpec));
	want.freq = 48000;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.samples = 4096; // not sure what this is...
	want.callback = zen_audio_callback;
	
	const char *audio_device_name = NULL;
	int is_capture = 0;
	int allowed_changes = 0; // SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_FREQUENCY_CHANGE |
									 // SDL_AUDIO_ALLOW_CHANNELS_CHANGE | SDL_AUDIO_ALLOW_ANY_CHANGE

	dev = SDL_OpenAudioDevice(audio_device_name, is_capture, &want, &mixer, allowed_changes);
	if (dev == 0) {
		GB_PANIC("failed to open audio");
	}

	int set_paused = 0; // 1 = paused, 0 = unpaused
	SDL_PauseAudioDevice(dev, set_paused);

	// audio should now be playing

}
