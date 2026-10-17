#pragma once
#include <stdint.h>
/* The installed SDCC signed 16x16->32 helper tests the low-byte sign bit.
 * Keep multiplication unsigned and restore the sign explicitly. No SDK patch.
 * Magnitudes include INT16_MIN and their product fits signed 32 bits. */
static int32_t wide_product(int16_t a,int16_t b)
{
    uint16_t ua=(uint16_t)a,ub=(uint16_t)b;
    uint32_t product;
    if(a<0)ua=(uint16_t)(0u-ua);
    if(b<0)ub=(uint16_t)(0u-ub);
    product=(uint32_t)ua*ub;
    return (a<0)!=(b<0)?-(int32_t)product:(int32_t)product;
}
