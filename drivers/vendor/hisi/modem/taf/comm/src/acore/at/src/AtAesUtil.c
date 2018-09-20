
#include "AtAesUtil.h"

u16 ROTL(u16 x, int n, int m)
{
    u16 vmask = (1 << m) - 1;
    int rot = n % m;
    u16 ret = x << rot | x >> (m - rot);
    return ret & vmask;
}
void xor_block_128(u8* tar, const u8* src)
{
    u32* T = (u32*)tar;
    u32* S = (u32*)src;

    *T++ ^= *S++;
    *T++ ^= *S++;
    *T++ ^= *S++;
    *T++ ^= *S++;
}
void xor_block(u8* tar, const u8* src, u32 len)
{
    u32 i;
    for(i = 0;i < len;++i)
    {
        tar[i] ^= src[i];
    }
}
