

#ifndef __AT_SHA256_H__
#define __AT_SHA256_H__

#include  "vos.h"

#define SHA256_BLOCK_SIZE        (64)
#define SHA256_DIGEST_SIZE       (32)
#define SHA256_PAD_SIZE          (56)

typedef struct Sha256 { 
    VOS_UINT32  buffLen;   /* in bytes          */
    VOS_UINT32  loLen;     /* length in bytes   */
    VOS_UINT32  hiLen;     /* length in bytes   */
    VOS_UINT32  digest[SHA256_DIGEST_SIZE / sizeof(VOS_UINT32)];
    VOS_UINT32  buffer[SHA256_BLOCK_SIZE  / sizeof(VOS_UINT32)];
} Sha256;

void sha256_hash(const VOS_UINT8* data, VOS_UINT32 len, VOS_UINT8 hash[SHA256_DIGEST_SIZE]);

#endif /*__AT_SHA256_H__*/