/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include "pong.h"
#include "fixed_math.h"
static const uint8_t ai_reaction[]={17,10,6};
static const uint8_t ai_speed[]={88,112,136};
static const uint8_t ai_error[]={17,9,4};
static uint16_t random_next(pong_game* g)
{
    uint16_t n=g->rng;
    n^=n<<7;n^=n>>9;n^=n<<8;g->rng=n;
    return n;
}
static int16_t clamp(int16_t x,int16_t a,int16_t b){return x<a?a:x>b?b:x;}
static int16_t approach(int16_t value,int16_t target,int16_t amount)
{
    if(value<target)return value+amount>target?target:value+amount;
    return value-amount<target?target:value-amount;
}
static void choose_return(pong_game* g)
{
    uint16_t n=random_next(g);
    int16_t aim=(int16_t)(n%37)-18;
    uint8_t tactic=(uint8_t)(n>>8)&3;
    /* Choose once per incoming shot. Read the observed paddle position, never
     * the player's input or a future position. Keep reaction/speed limits. */
    if(tactic==1 || tactic==2) {
        int16_t open_side=(COURT_TOP+COURT_BOTTOM)/2*FP-g->left_y;
        int16_t depth=10+(n%9);
        /* Near centre, either lane is viable. A negative paddle offset sends
         * the right-hand return downwards; a positive offset sends it up. */
        if(open_side>40*FP)aim=-depth;
        else if(open_side< -40*FP)aim=depth;
        else aim=(n&1)?depth:-depth;
    } else if(tactic==3) {
        int16_t shallow=7+(n%4);
        aim=(n&1)?shallow:-shallow;
    }
    g->ai_offset=aim*FP;
    g->ai_error=((int16_t)(random_next(g)%(ai_error[g->difficulty]*2+1))-ai_error[g->difficulty])*FP;
    g->ai_wait=ai_reaction[g->difficulty];
}
int16_t pong_predict(const pong_game* g,uint8_t right)
{
    int16_t face=(right?RIGHT_X-BALL_R:LEFT_X+PADDLE_W+BALL_R)*FP;
    int32_t y=g->y;
    const int16_t low=(COURT_TOP+BALL_R)*FP,range=(COURT_BOTTOM-COURT_TOP-2*BALL_R)*FP;
    if(g->vx && ((right && g->vx>0)||(!right && g->vx<0)))
        y+=wide_product(g->vy,face-g->x)/g->vx;
    /* Triangle-wave folding predicts arbitrary wall bounces without a loop. */
    y=(y-low)%(range*2L);if(y<0)y+=range*2L;
    if(y>range)y=range*2L-y;
    return (int16_t)(low+y);
}
void pong_ai_tick(pong_game* g)
{
    int16_t delta,target_v;
    if(g->ai_wait)g->ai_wait--;
    else {
        if(g->phase==PHASE_RALLY && g->vx>0)
            g->ai_target=pong_predict(g,1)+g->ai_error+g->ai_offset;
        else g->ai_target=(COURT_TOP+COURT_BOTTOM)/2*FP+g->defence;
        g->ai_target=clamp(g->ai_target,(COURT_TOP+PADDLE_H/2)*FP,(COURT_BOTTOM-PADDLE_H/2)*FP);
        g->ai_wait=ai_reaction[g->difficulty]+random_next(g)%5;
    }
    delta=g->ai_target-g->right_y;
    /* Brake near the target. Acceleration and speed limits apply in all modes. */
    target_v=clamp(delta/3,-ai_speed[g->difficulty],ai_speed[g->difficulty]);
    g->right_v=approach(g->right_v,target_v,20);
    g->right_y=clamp(g->right_y+g->right_v,(COURT_TOP+PADDLE_H/2)*FP,(COURT_BOTTOM-PADDLE_H/2)*FP);
}
static void serve(pong_game* g,int8_t direction)
{
    g->phase=PHASE_SERVE;g->wait=60;g->serve_dir=direction;
    g->x=SCREEN_W/2*FP;g->y=(COURT_TOP+COURT_BOTTOM)/2*FP;
    g->vx=g->vy=0;g->speed=128;g->hits=0;
    g->defence=((int16_t)(random_next(g)%129)-64)*FP;
    choose_return(g);
}
void pong_start(pong_game* g,uint8_t difficulty,uint16_t seed)
{
    memset(g,0,sizeof(*g));g->rng=seed?seed:0x6d2b;
    g->difficulty=difficulty>2?1:difficulty;
    g->left_y=g->right_y=(COURT_TOP+COURT_BOTTOM)/2*FP;
    g->ai_target=g->right_y;
    serve(g,(random_next(g)&1)?1:-1);
}
static void bounce(pong_game* g,int16_t paddle,int16_t movement,uint8_t left)
{
    /* Thirteen symmetric return angles, about 0/8/17/24/30/36/41 degrees.
     * Finer contact zones improve aiming without changing the speed ceiling. */
    static const uint8_t horizontal[]={193,208,222,235,245,253,255,253,245,235,222,208,193};
    static const int16_t vertical[]={-168,-149,-128,-102,-74,-38,0,38,74,102,128,149,168};
    int16_t offset=(g->y-paddle)/FP+movement/8;
    uint8_t distance=(uint8_t)clamp(((offset<0?-offset:offset)+2)/4,0,6);
    uint8_t zone=offset<0?6-distance:6+distance;
    if(g->speed<288)g->speed+=6;
    if(g->speed>288)g->speed=288;
    g->vx=(int16_t)((int32_t)g->speed*horizontal[zone]/256);
    if(!left)g->vx=-g->vx;
    g->vy=(int16_t)(wide_product(g->speed,vertical[zone])/256);
    g->hits++;g->events|=EVENT_PADDLE;
    if(left)choose_return(g);
    else {g->defence=((int16_t)(random_next(g)%161)-80)*FP;g->ai_wait=0;}
}
static void point(pong_game* g,uint8_t scorer)
{
    uint8_t other=(uint8_t)(scorer^1);
    g->score[scorer]++;g->events|=EVENT_POINT;
    if((g->score[scorer]>=11 && g->score[scorer]>=g->score[other]+2)||g->score[scorer]>=21) {
        g->phase=PHASE_OVER;g->vx=g->vy=0;g->x=clamp(g->x,-BALL_R*FP,(SCREEN_W+BALL_R)*FP);g->events|=EVENT_WIN;
    } else serve(g,scorer? -1:1); /* Serve towards the player who conceded. */
}
/* Exact signed quarter-step, preserving C truncation toward zero.
 * The remainder is distributed in the same substeps as
 * (value*(step+1))/4 - (value*step)/4, without repeated products. */
static int16_t quarter_step(int16_t value,uint8_t step)
{
    int16_t part=value/4;
    uint8_t remainder=(uint8_t)(value<0?-value:value)&3;
    if(remainder && (step==3 || (step==1 && remainder>=2) || (step==2 && remainder==3)))
        part+=value<0?-1:1;
    return part;
}
void pong_tick(pong_game* g,int8_t direction)
{
    uint8_t i;
    int16_t old_left=g->left_y,old_right=g->right_y;
    g->events=0;g->ticks++;
    if(g->phase==PHASE_OVER)return;
    direction=direction>0?1:direction<0?-1:0;
    g->left_v=approach(g->left_v,direction*160,direction?48:64);
    g->left_y=clamp(g->left_y+g->left_v,(COURT_TOP+PADDLE_H/2)*FP,(COURT_BOTTOM-PADDLE_H/2)*FP);
    if(g->left_y==(COURT_TOP+PADDLE_H/2)*FP || g->left_y==(COURT_BOTTOM-PADDLE_H/2)*FP)g->left_v=0;
    pong_ai_tick(g);
    if(g->phase==PHASE_SERVE) {
        if(--g->wait==0) {
            static const int16_t slopes[]={-64,-37,37,64};
            uint8_t choice=random_next(g)%4;
            g->phase=PHASE_RALLY;
            g->vx=g->serve_dir*((choice==1 || choice==2)?122:111);
            g->vy=slopes[choice];g->events|=EVENT_SERVE;
        }
        return;
    }
    /* Four substeps: at maximum speed a step is <=2.25px, below ball diameter.
     * Remainder differences conserve fractional velocity exactly. */
    {
        int16_t ly=old_left,ry=old_right;
        int16_t left_move=g->left_y-old_left,right_move=g->right_y-old_right;
        for(i=0;i<4;i++) {
            int16_t old_x=g->x;
            ly+=quarter_step(left_move,i);
            ry+=quarter_step(right_move,i);
            /* Read velocity at each substep: a contact can change it mid-tick. */
            g->x+=quarter_step(g->vx,i);
            g->y+=quarter_step(g->vy,i);
            if(g->y<(COURT_TOP+BALL_R)*FP) {g->y=2*(COURT_TOP+BALL_R)*FP-g->y;if(g->vy<0)g->vy=-g->vy;g->events|=EVENT_WALL;}
            else if(g->y>(COURT_BOTTOM-BALL_R)*FP) {g->y=2*(COURT_BOTTOM-BALL_R)*FP-g->y;if(g->vy>0)g->vy=-g->vy;g->events|=EVENT_WALL;}
            if(g->vx<0 && old_x>=(LEFT_X+PADDLE_W+BALL_R)*FP && g->x<=(LEFT_X+PADDLE_W+BALL_R)*FP &&
               g->y>=ly-(PADDLE_H/2+BALL_R)*FP && g->y<=ly+(PADDLE_H/2+BALL_R)*FP) {
                g->x=2*(LEFT_X+PADDLE_W+BALL_R)*FP-g->x;
                bounce(g,ly,g->left_y-old_left,1);
            } else if(g->vx>0 && old_x<=(RIGHT_X-BALL_R)*FP && g->x>=(RIGHT_X-BALL_R)*FP &&
               g->y>=ry-(PADDLE_H/2+BALL_R)*FP && g->y<=ry+(PADDLE_H/2+BALL_R)*FP) {
                g->x=(RIGHT_X-BALL_R)*FP+((RIGHT_X-BALL_R)*FP-g->x);
                bounce(g,ry,g->right_y-old_right,0);
            }
            if(g->x<-BALL_R*FP){point(g,1);break;}
            if(g->x>(SCREEN_W+BALL_R)*FP){point(g,0);break;}
        }
    }
}
