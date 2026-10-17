#include <stdint.h>
#include <string.h>
#include <zvb_sprite.h>
#include "render.h"
#include "fixed_math.h"
#include "screen_settings.h"
gfx_context video;
static gfx_sprite actors[7];
static uint8_t view,last_status=255,last_left=255,last_right=255,last_difficulty=255;
static uint8_t hud[2][40],hud_dirty,prompt_dirty,prompt_visible=255;
static uint8_t prompt_row[PROMPT_W];
static const char* const difficulty_names[]={"EASY","NORMAL","HARD"};
static uint8_t glyph(char c)
{
    if(c>='A' && c<='Z')return FONT_ALPHA+c-'A';
    if(c>='0' && c<='9')return FONT_DIGIT+c-'0';
    return TILE_BLANK;
}
static void text(uint8_t* row,const char* s,uint8_t column)
{
    while(*s && column<40)row[column++]=glyph(*s++);
}
static void centered(uint8_t* row,const char* s)
{
    uint8_t length=(uint8_t)strlen(s);text(row,s,(40-length)/2);
}
static void number(uint8_t* row,uint8_t n,uint8_t column)
{
    row[column]=FONT_DIGIT+n/10;row[column+1]=FONT_DIGIT+n%10;
}
void render_clear_sprites(void)
{
    gfx_sprite blank={0};uint8_t i;
    for(i=0;i<128;i++)gfx_sprite_render(&video,i,&blank);
}
void render_load(uint8_t scene)
{
    const uint8_t* source=scene==VIEW_SPLASH?splash_map:court_map;
    uint8_t row[40],x,y,count=0,tile=0,repeat=0;
    view=scene;
    for(y=0;y<30;y++) {
        for(x=0;x<40;x++) {
            if(!count) {uint8_t control=*source++;count=(control&127)+1;repeat=control&128;if(repeat)tile=*source++;}
            row[x]=repeat?tile:*source++;count--;
            if(row[x]==255)row[x]=TILE_BLANK; /* SDK empty-cell encoding. */
        }
        gfx_tilemap_load(&video,row,40,0,0,y);
    }
    memset(row,0,sizeof(row));
    for(y=0;y<30;y++)gfx_tilemap_load(&video,row,40,1,0,y);
    memset(actors,0,sizeof(actors));
    last_status=last_left=last_right=last_difficulty=prompt_visible=255;
    hud_dirty=prompt_dirty=0;
}
static void actor(uint8_t i,int16_t x,int16_t y,uint8_t tile)
{
    actors[i].x=(uint16_t)(x+16);actors[i].y=(uint16_t)(y+16);
    actors[i].tile=tile;actors[i].flags=SPRITE_NONE;actors[i].options=SPRITE_OPTION_NONE;
}
void render_prepare(const pong_game* g,uint8_t paused,uint16_t frame)
{
    uint8_t i,status=g->phase+paused*4;
    int16_t x=g->x/FP,y=g->y/FP,ly=g->left_y/FP,ry=g->right_y/FP;
    if(view==VIEW_SPLASH) {
        uint8_t visible=(frame/30)&1;
        actor(0,DEMO_X+wide_product(x,DEMO_W)/SCREEN_W-8,DEMO_Y+wide_product(y-COURT_TOP,DEMO_H)/(COURT_BOTTOM-COURT_TOP)-8,TILE_BALL);
        actor(1,DEMO_X+8-4,DEMO_Y+wide_product(ly-COURT_TOP,DEMO_H)/(COURT_BOTTOM-COURT_TOP)-8,TILE_PADDLE);
        actor(2,DEMO_X+DEMO_W-16-4,DEMO_Y+wide_product(ry-COURT_TOP,DEMO_H)/(COURT_BOTTOM-COURT_TOP)-8,TILE_PADDLE);
        if(visible!=prompt_visible){prompt_visible=visible;prompt_dirty=1;for(i=0;i<PROMPT_W;i++)prompt_row[i]=visible?0:prompt_tiles[i];}
    } else {
        for(i=0;i<3;i++) {
            actor(i,LEFT_X-4,ly-PADDLE_H/2+i*16,TILE_PADDLE);
            actor(i+3,RIGHT_X-4,ry-PADDLE_H/2+i*16,TILE_PADDLE);
        }
        actor(6,x-8,y-8,g->phase==PHASE_OVER?TILE_BLANK:TILE_BALL);
    }
    if(status==last_status && g->score[0]==last_left && g->score[1]==last_right && g->difficulty==last_difficulty)return;
    memset(hud,0,sizeof(hud));
    if(view==VIEW_SPLASH) {
        text(hud[0],"DIFFICULTY",11);text(hud[0],difficulty_names[g->difficulty],23);
    } else {
        text(hud[0],"PLAYER",3);number(hud[0],g->score[0],10);
        text(hud[0],difficulty_names[g->difficulty],17);
        text(hud[0],"CPU",28);number(hud[0],g->score[1],33);
        if(paused)centered(hud[1],"PAUSED   SPACE TO RESUME");
        else if(g->phase==PHASE_SERVE)centered(hud[1],"GET READY");
        else if(g->phase==PHASE_OVER)centered(hud[1],g->score[0]>g->score[1]?"YOU WIN   ENTER TO PLAY AGAIN":"CPU WINS   ENTER TO TRY AGAIN");
    }
    last_status=status;last_left=g->score[0];last_right=g->score[1];last_difficulty=g->difficulty;hud_dirty=1;
}
void render_commit(void)
{
    /* Prepared outside VBlank: one 56-byte sprite upload and at most 80 HUD bytes. */
    gfx_sprite_render_array(&video,0,actors,7);
    if(hud_dirty){gfx_tilemap_load(&video,hud[0],40,1,0,view==VIEW_SPLASH?16:0);if(view==VIEW_GAME)gfx_tilemap_load(&video,hud[1],40,1,0,1);hud_dirty=0;}
    if(prompt_dirty){gfx_tilemap_load(&video,prompt_row,PROMPT_W,0,PROMPT_X,PROMPT_Y);prompt_dirty=0;}
}
