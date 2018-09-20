

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include  "AtSha256.h"
#include  "ATCmdProc.h"

static VOS_UINT32 rotlFixed(VOS_UINT32 x, VOS_UINT32 y)
{
    return (x << y) | (x >> (sizeof(y) * 8 - y));
}   

static VOS_UINT32 rotrFixed(VOS_UINT32 x, VOS_UINT32 y)
{
    return (x >> y) | (x << (sizeof(y) * 8 - y));
}

static const VOS_UINT32 K[64] = {
    0x428A2F98L, 0x71374491L, 0xB5C0FBCFL, 0xE9B5DBA5L, 0x3956C25BL,
    0x59F111F1L, 0x923F82A4L, 0xAB1C5ED5L, 0xD807AA98L, 0x12835B01L,
    0x243185BEL, 0x550C7DC3L, 0x72BE5D74L, 0x80DEB1FEL, 0x9BDC06A7L,
    0xC19BF174L, 0xE49B69C1L, 0xEFBE4786L, 0x0FC19DC6L, 0x240CA1CCL,
    0x2DE92C6FL, 0x4A7484AAL, 0x5CB0A9DCL, 0x76F988DAL, 0x983E5152L,
    0xA831C66DL, 0xB00327C8L, 0xBF597FC7L, 0xC6E00BF3L, 0xD5A79147L,
    0x06CA6351L, 0x14292967L, 0x27B70A85L, 0x2E1B2138L, 0x4D2C6DFCL,
    0x53380D13L, 0x650A7354L, 0x766A0ABBL, 0x81C2C92EL, 0x92722C85L,
    0xA2BFE8A1L, 0xA81A664BL, 0xC24B8B70L, 0xC76C51A3L, 0xD192E819L,
    0xD6990624L, 0xF40E3585L, 0x106AA070L, 0x19A4C116L, 0x1E376C08L,
    0x2748774CL, 0x34B0BCB5L, 0x391C0CB3L, 0x4ED8AA4AL, 0x5B9CCA4FL,
    0x682E6FF3L, 0x748F82EEL, 0x78A5636FL, 0x84C87814L, 0x8CC70208L,
    0x90BEFFFAL, 0xA4506CEBL, 0xBEF9A3F7L, 0xC67178F2L
};

#define Ch(x,y,z)       (z ^ (x & (y ^ z)))
#define Maj(x,y,z)      (((x | y) & z) | (x & y))
#define S(x, n)         rotrFixed(x, n)
#define R(x, n)         (((x) & 0xFFFFFFFFU) >> (n))
#define Sigma0(x)       (S(x, 2) ^ S(x, 13) ^ S(x, 22))
#define Sigma1(x)       (S(x, 6) ^ S(x, 11) ^ S(x, 25))
#define Gamma0(x)       (S(x, 7) ^ S(x, 18) ^ R(x, 3))
#define Gamma1(x)       (S(x, 17) ^ S(x, 19) ^ R(x, 10))

#define RND(a,b,c,d,e,f,g,h,i) \
     t0 = h + Sigma1(e) + Ch(e, f, g) + K[i] + W[i]; \
     t1 = Sigma0(a) + Maj(a, b, c); \
     d += t0; \
     h  = t0 + t1;

#define min(a,b) (a > b ? b : a)

void InitSha256(Sha256* sha256)
{
    sha256->digest[0] = 0x6A09E667L;
    sha256->digest[1] = 0xBB67AE85L;
    sha256->digest[2] = 0x3C6EF372L;
    sha256->digest[3] = 0xA54FF53AL;
    sha256->digest[4] = 0x510E527FL;
    sha256->digest[5] = 0x9B05688CL;
    sha256->digest[6] = 0x1F83D9ABL;
    sha256->digest[7] = 0x5BE0CD19L;

    sha256->buffLen = 0;
    sha256->loLen   = 0;
    sha256->hiLen   = 0;
}

static VOS_UINT32 ByteReverseWord32(VOS_UINT32 value)
{
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    return rotlFixed(value, 16U);
}

static void ByteReverseWords(VOS_UINT32* out, const VOS_UINT32* in,
                                    VOS_UINT32 byteCount)
{
    VOS_UINT32 count = byteCount / (VOS_UINT32)sizeof(VOS_UINT32), i;

    for (i = 0; i < count; i++)
    {
        out[i] = ByteReverseWord32(in[i]);
    }

}
static void ByteReverseBytes(VOS_UINT8* out, const VOS_UINT8* in, VOS_UINT32 byteCount)
{
    VOS_UINT32* op       = (VOS_UINT32*)out;
    const VOS_UINT32* ip = (const VOS_UINT32*)in;

    ByteReverseWords(op, ip, byteCount);
}

static void Transform(Sha256* sha256)
{
    VOS_UINT32 S[8], W[64], t0, t1;
    int i;

    /* Copy context->state[] to working vars */
    for (i = 0; i < 8; i++)
    {
        S[i] = sha256->digest[i];
    }

    for (i = 0; i < 16; i++)
    {
        W[i] = sha256->buffer[i];
    }

    for (i = 16; i < 64; i++)
    {
        W[i] = Gamma1(W[i - 2]) + W[i - 7] + Gamma0(W[i - 15]) + W[i - 16];
    }

    for (i = 0; i < 64; i += 8) {
        RND(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i + 0);
        RND(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i + 1);
        RND(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i + 2);
        RND(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i + 3);
        RND(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i + 4);
        RND(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i + 5);
        RND(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i + 6);
        RND(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i + 7);
    }

    /* Add the working vars back into digest state[] */
    for (i = 0; i < 8; i++) 
    {
        sha256->digest[i] += S[i];
    }
}


static void AddLength(Sha256* sha256, VOS_UINT32 len)
{
    VOS_UINT32 tmp = sha256->loLen;
    if ( (sha256->loLen += len) < tmp)
    {
        sha256->hiLen++;                       /* carry low to high */
    }
}


void Sha256Update(Sha256* sha256, const VOS_UINT8* data, VOS_UINT32 len)
{
    /* do block size increments */
    VOS_UINT8* local = (VOS_UINT8*)sha256->buffer;

    while (len) {
        VOS_UINT32 add = min(len, SHA256_BLOCK_SIZE - sha256->buffLen);
        memcpy(&local[sha256->buffLen], data, add);

        sha256->buffLen += add;
        data            += add;
        len             -= add;

        if (sha256->buffLen == SHA256_BLOCK_SIZE) 
        {
            ByteReverseBytes(local, local, SHA256_BLOCK_SIZE);
            Transform(sha256);
            AddLength(sha256, SHA256_BLOCK_SIZE);
            sha256->buffLen = 0;
        }
    }
}


void Sha256Final(Sha256* sha256, VOS_UINT8* hash)
{
    VOS_UINT8* local = (VOS_UINT8*)sha256->buffer;

    AddLength(sha256, sha256->buffLen);  /* before adding pads */

    local[sha256->buffLen++] = 0x80;  /* add 1 */

    /* pad with zeros */
    if (sha256->buffLen > SHA256_PAD_SIZE) 
    {
        memset(&local[sha256->buffLen], 0, SHA256_BLOCK_SIZE - sha256->buffLen);
        sha256->buffLen += SHA256_BLOCK_SIZE - sha256->buffLen;


        ByteReverseBytes(local, local, SHA256_BLOCK_SIZE);
        Transform(sha256);
        sha256->buffLen = 0;
    }
    memset(&local[sha256->buffLen], 0, SHA256_PAD_SIZE - sha256->buffLen);

    /* put lengths in bits */
    sha256->hiLen = (sha256->loLen >> (8*sizeof(sha256->loLen) - 3)) +
                 (sha256->hiLen << 3);
    sha256->loLen = sha256->loLen << 3;

    /* store lengths */
    ByteReverseBytes(local, local, SHA256_BLOCK_SIZE);
    /* ! length ordering dependent on digest endian type ! */
    memcpy((&local[SHA256_PAD_SIZE]), (&sha256->hiLen), sizeof(VOS_UINT32));
    memcpy((&local[SHA256_PAD_SIZE + sizeof(VOS_UINT32)]), (&sha256->loLen),
            sizeof(VOS_UINT32));

    Transform(sha256);
    ByteReverseWords(sha256->digest, sha256->digest, SHA256_DIGEST_SIZE);
    memcpy(hash, sha256->digest, SHA256_DIGEST_SIZE);

    InitSha256(sha256);  /* reset state */
}

void sha256_hash(const VOS_UINT8* data, VOS_UINT32 len, VOS_UINT8 hash[SHA256_DIGEST_SIZE]) 
{
    Sha256      sha;

    InitSha256(&sha);
    Sha256Update(&sha, (VOS_UINT8*)data,(VOS_UINT32)len);
    Sha256Final(&sha, hash);
}