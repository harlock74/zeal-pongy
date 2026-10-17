#include <zvb_sound.h>
#include "pong.h"
#include "audio.h"
static uint8_t remaining;
void audio_quiet(void)
{
    remaining=0;zvb_sound_set_channels(0,0);zvb_sound_set_volume(VOL_0);
    zvb_sound_set_voices_vol(0x0f,VOL_0);zvb_sound_set_hold(0xff,1);
}
void audio_init(void){zvb_sound_initialize(1);audio_quiet();}
void audio_tick(uint8_t events)
{
    uint16_t divider=0;uint8_t duration=0;
    if(remaining && !--remaining)audio_quiet();
    if(events&EVENT_WALL){divider=520;duration=2;}
    if(events&EVENT_PADDLE){divider=980;duration=2;}
    if(events&EVENT_SERVE){divider=1250;duration=4;}
    if(events&EVENT_POINT){divider=350;duration=9;}
    if(events&EVENT_WIN){divider=1650;duration=16;}
    if(!duration)return;
    remaining=duration;zvb_sound_set_voices(VOICE0,divider,WAV_TRIANGLE);
    zvb_sound_set_voices_vol(VOICE0,VOL_25);zvb_sound_set_hold(VOICE0,0);
    zvb_sound_set_channels(VOICE0,VOICE0);zvb_sound_set_volume(VOL_50);
}
