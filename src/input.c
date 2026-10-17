#include <stdint.h>
#include <zos_keyboard.h>
#include <zos_vfs.h>
#include <zgdk/input.h>
#include "input.h"
static uint8_t storage[127],*buffer;
static uint16_t physical,pad,previous;
static uint8_t release_pending,keyboard_owner;
static uint8_t leases[4];
static uint16_t state(void)
{
    uint16_t keys=0;
    if(physical&(1u<<4))keys|=BUTTON_START;
    if(physical&((1u<<5)|(1u<<6)))keys|=BUTTON_B;
    if(physical&((1u<<7)|(1u<<8)))keys|=BUTTON_SELECT;
    if(physical&(1u<<9))keys|=BUTTON_LEFT;
    if(physical&(1u<<10))keys|=BUTTON_RIGHT;
    if(physical&5)keys|=BUTTON_UP;
    if(physical&10)keys|=BUTTON_DOWN;
    return keys|(keyboard_owner?(pad&~(BUTTON_UP|BUTTON_DOWN)):pad);
}
static void key(uint8_t code,uint8_t released)
{
    uint16_t bit=0;uint8_t i=4;
    switch(code) {
    case KB_UP_ARROW:bit=1;i=0;break;
    case KB_DOWN_ARROW:bit=2;i=1;break;
    case KB_KEY_W:bit=4;i=2;break;
    case KB_KEY_S:bit=8;i=3;break;
    case KB_LEFT_ARROW:bit=1u<<9;break;
    case KB_RIGHT_ARROW:bit=1u<<10;break;
    case KB_KEY_ENTER:bit=1u<<4;break;
    case KB_KEY_SPACE:bit=1u<<5;break;
    case KB_KEY_P:bit=1u<<6;break;
    case KB_KEY_Q:bit=1u<<7;break;
    case KB_RIGHT_SHIFT:bit=1u<<8;break;
    default:return;
    }
    if(i<4){keyboard_owner=1;leases[i]=released?0:(physical&bit)?8:48;}
    if(released)physical&=~bit;else physical|=bit;
}
static void poll_keys(pong_input* out)
{
    uint8_t batch;
    for(batch=0;batch<4;batch++) {
        uint16_t size=64;uint8_t i;
        if(read(DEV_STDIN,buffer,&size)!=ERR_SUCCESS){physical=0;release_pending=0;return;}
        if(!size)return;
        for(i=0;i<size;i++) {
            uint16_t before=state();
            if(buffer[i]==KB_RELEASED)release_pending=1;
            else {key(buffer[i],release_pending);release_pending=0;}
            out->pressed|=state()&~before;
        }
    }
}
zos_err_t pong_input_init(void)
{
    zos_err_t err=keyboard_init();
    buffer=(uint8_t*)((uintptr_t)(storage+63)&~(uintptr_t)63);
    if(err)return err;
    controller_init(); /* Keyboard remains usable with no controller attached. */
    pong_input_flush();return ERR_SUCCESS;
}
void pong_input_poll(pong_input* out)
{
    uint16_t old_pad=pad,raw=controller_read();uint8_t i;
    out->pressed=0;
    pad=(raw&0xf000) || (raw&0x0fff)==0x0fff ? 0:raw;
    if((pad&BUTTON_START) && !(old_pad&BUTTON_START))keyboard_owner=0;
    for(i=0;i<4;i++)if(leases[i] && !--leases[i])physical&=~(1u<<i);
    out->pressed|=state()&~previous;
    poll_keys(out);out->held=state();previous=out->held;
}
void pong_input_flush(void)
{
    pong_input unused={0,0};uint8_t i;
    poll_keys(&unused);physical=pad=previous=release_pending=0;
    for(i=0;i<4;i++)leases[i]=0;
}
