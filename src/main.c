#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <zos_keyboard.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zgdk/input.h>
#include <zvb_hardware.h>
#include "input.h"
#include "pong.h"
#include "render.h"
#include "assets.h"
#include "audio.h"
static pong_game match;
int main(void)
{
    pong_input input;uint16_t frame=0,seed=0x831d;
    uint8_t result=0,audio_ready=0;
    uint8_t scene=VIEW_SPLASH,ready=0,paused=0,difficulty=1,guard=20;
    if(pong_input_init()){printf("Keyboard initialization failed.\n");return 1;}
    gfx_enable_screen(0);
    if(gfx_initialize(ZVB_CTRL_VID_MODE_GFX_640_8BIT,&video) || assets_load(&video)){result=1;goto cleanup;}
    audio_init();audio_ready=1;render_clear_sprites();pong_start(&match,difficulty,seed);match.wait=1;
    render_load(scene);render_prepare(&match,0,0);render_commit();gfx_enable_screen(1);
    while(1) {
        int8_t direction;uint16_t motion;
        pong_input_poll(&input);frame++;seed+=frame|1;
        if(input.pressed&BUTTON_SELECT)break;
        motion=input.held;
        if(!(motion&(BUTTON_UP|BUTTON_DOWN)))motion|=input.pressed&(BUTTON_UP|BUTTON_DOWN);
        direction=(motion&BUTTON_UP)?-1:0;
        if(motion&BUTTON_DOWN)direction++;
        if(scene==VIEW_SPLASH) {
            if(guard)guard--;else if(!(input.held&BUTTON_START))ready=1;
            if(input.pressed&BUTTON_LEFT)difficulty=difficulty?difficulty-1:2;
            if(input.pressed&BUTTON_RIGHT)difficulty=difficulty==2?0:difficulty+1;
            if(ready && (input.pressed&BUTTON_START)) {
                scene=VIEW_GAME;paused=0;pong_start(&match,difficulty,seed);
                gfx_enable_screen(0);render_load(scene);audio_tick(EVENT_SERVE);
                render_prepare(&match,0,frame);render_commit();gfx_enable_screen(1);
                ready=0;
            } else {
                int16_t target=pong_predict(&match,0)-match.left_y;
                pong_tick(&match,target>FP*3?1:target<-FP*3?-1:0);
                if(match.phase==PHASE_OVER)pong_start(&match,difficulty,seed);
                match.difficulty=difficulty; /* Menu selection; demo remains silent. */
            }
        } else {
            if(match.phase==PHASE_OVER) {
                if(!(input.held&BUTTON_START))ready=1;
                if(ready && (input.pressed&BUTTON_START)){pong_start(&match,difficulty,seed);ready=0;}
                audio_tick(0);
            } else {
                if(input.pressed&(BUTTON_B|BUTTON_START)) {paused^=1;audio_quiet();}
                if(!paused){pong_tick(&match,direction);audio_tick(match.events);}
            }
        }
        render_prepare(&match,paused,frame);
        gfx_wait_vblank(&video);render_commit();gfx_wait_end_vblank(&video);
    }
cleanup:
    if(audio_ready){audio_quiet();gfx_enable_screen(0);render_clear_sprites();}
    ioctl(DEV_STDOUT,CMD_RESET_SCREEN,0);
    ioctl(DEV_STDIN,KB_CMD_SET_MODE,(void*)(KB_MODE_COOKED|KB_READ_BLOCK));
    if(result)printf("Graphics initialization failed.\n");
    return result;
}
