#pragma once
#include <zvb_gfx.h>
#include "pong.h"
enum { VIEW_SPLASH,VIEW_GAME };
extern gfx_context video;
void render_clear_sprites(void);
void render_load(uint8_t scene);
void render_prepare(const pong_game* g,uint8_t paused,uint16_t frame);
void render_commit(void);
