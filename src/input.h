#pragma once
#include <stdint.h>
#include <zos_errors.h>
typedef struct {uint16_t held,pressed;} pong_input;
zos_err_t pong_input_init(void);
void pong_input_poll(pong_input* input);
void pong_input_flush(void);
