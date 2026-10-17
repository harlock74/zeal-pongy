#pragma once
#include <stdint.h>
void audio_init(void);
void audio_tick(uint8_t events);
void audio_quiet(void);
