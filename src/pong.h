/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include "config.h"
enum { PHASE_SERVE, PHASE_RALLY, PHASE_OVER };
enum { EVENT_WALL=1, EVENT_PADDLE=2, EVENT_POINT=4, EVENT_SERVE=8, EVENT_WIN=16 };
typedef struct {
    int16_t x,y,vx,vy;
    int16_t left_y,right_y,left_v,right_v;
    int16_t ai_target,ai_error,ai_offset,defence;
    int16_t speed;
    uint16_t rng,ticks,hits;
    uint8_t score[2],difficulty,phase,wait,ai_wait,events;
    int8_t serve_dir;
} pong_game;
void pong_start(pong_game* g,uint8_t difficulty,uint16_t seed);
void pong_tick(pong_game* g,int8_t direction);
int16_t pong_predict(const pong_game* g,uint8_t right);
void pong_ai_tick(pong_game* g);
