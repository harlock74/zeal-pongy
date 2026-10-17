#include <stdint.h>
#include "assets.h"
extern uint8_t _palette_start,_palette_end,_tiles_start,_tiles_end;
gfx_error assets_load(gfx_context* ctx)
{
    gfx_error err=gfx_palette_load(ctx,&_palette_start,(uint16_t)(&_palette_end-&_palette_start),0);
    gfx_tileset_options options={.compression=TILESET_COMP_1BIT};
    if(err)return err;
    return gfx_tileset_load(ctx,&_tiles_start,(uint16_t)(&_tiles_end-&_tiles_start),&options);
}
void __assets__(void) __naked
{
    __asm__("__palette_start:\n.incbin \"assets/pong.ztp\"\n__palette_end:\n"
            "__tiles_start:\n.incbin \"assets/pong.zts\"\n__tiles_end:\n");
}
