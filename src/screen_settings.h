/* Fixed splash prompt/demo settings. Edit here when changing their positions in Tiled. */
#pragma once
#include <stdint.h>
extern const uint8_t court_map[];
extern const uint8_t splash_map[];
static const uint8_t prompt_tiles[]={47,49,36,50,50,0,36,45,51,36,49,0,51,46,0,47,43,32,56};
#define PROMPT_X 10
#define PROMPT_Y 27
#define PROMPT_W 19
#define DEMO_X 128
#define DEMO_Y 320
#define DEMO_W 384
#define DEMO_H 80
