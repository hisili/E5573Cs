

#ifndef AT_AES_DECRYPT_H_
#define AT_AES_DECRYPT_H_

#include"AtAesUtil.h"

void wb_aes_decrypt(const u8* input, u8* output);
int wb_aes_decrypt_cbc(
        const u8* iv,
        const u8* input, u32 in_len,
        u8* output, u32* out_len
        );


#endif /* AT_AES_DECRYPT_H_ */
