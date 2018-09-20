

#ifndef AT_AES_KEYB_ENCRYPT_H_
#define AT_AES_KEYB_ENCRYPT_H_

#include"AtAesUtil.h"

void keyb_aes_encrypt(const u8* input, u8* output);
int keyb_aes_encrypt_cbc(
        const u8* iv,
        const u8* input, u32 in_len,
        u8* output, u32* out_len);

#endif /* AT_AES_KEYB_ENCRYPT_H_ */
