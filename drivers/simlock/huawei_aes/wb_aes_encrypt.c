#include "wb_aes_encrypt.h"
#include"wb_const.h"
#include"table1.h"
#include "DrvInterface.h"
#define TB1 table1
void wb_aes_encrypt(const u8* input, u8* output)
{
	int i;
    u8 s = WB_EXP_SIZE1;
    u16 vmask = (1<<s)-1;
    u16 t16;

    //input transformation
    u16 temp[16];
    for (i = 0; i < 16; i++)
	{
    	t16 = input[i];
    	temp[i] =
				ROTL_SIMLOCK(t16,IN_ROT1*i,s)^
				ROTL_SIMLOCK(t16,IN_ROT2*i,s)^
				ROTL_SIMLOCK(t16,IN_ROT3*i,s)^
				(i*IN_CONST);
	}

    //put transformed plaintext into Xs
#if WB_EXP_SIZE1==8
    u32 X0, X1, X2, X3, Y0, Y1, Y2, Y3;
    GET_UINT32_LE( X0, temp,  0 );
	GET_UINT32_LE( X1, temp,  4 );
	GET_UINT32_LE( X2, temp,  8 );
	GET_UINT32_LE( X3, temp, 12 );
#else
    u64 X0, X1, X2, X3, Y0, Y1, Y2, Y3;
    GET_UINT64_LE( X0, temp,  0 );
	GET_UINT64_LE( X1, temp,  4 );
    GET_UINT64_LE( X2, temp,  8 );
    GET_UINT64_LE( X3, temp, 12 );
#endif

   	//encryption
    WB_AES_FROUND(TB1,Y,X,0);
	WB_AES_FROUND(TB1,X,Y,1);
	WB_AES_FROUND(TB1,Y,X,2);
	WB_AES_FROUND(TB1,X,Y,3);
	WB_AES_FROUND(TB1,Y,X,4);
	WB_AES_FROUND(TB1,X,Y,5);
	WB_AES_FROUND(TB1,Y,X,6);
	WB_AES_FROUND(TB1,X,Y,7);
	WB_AES_FROUND(TB1,Y,X,8);
	WB_AES_FROUND(TB1,X,Y,9);
#if WB_NR1>10
	WB_AES_FROUND(TB1,Y,X,10);
	WB_AES_FROUND(TB1,X,Y,11);
#endif
#if WB_NR1>12
	WB_AES_FROUND(TB1,Y,X,12);
	WB_AES_FROUND(TB1,X,Y,13);
#endif

	//output transformation
#if WB_EXP_SIZE1==8
	PUT_UINT32_LE( X0, temp,  0 );
	PUT_UINT32_LE( X1, temp,  4 );
	PUT_UINT32_LE( X2, temp,  8 );
	PUT_UINT32_LE( X3, temp, 12 );
#else
    PUT_UINT64_LE( X0, temp,  0 );
    PUT_UINT64_LE( X1, temp,  4 );
    PUT_UINT64_LE( X2, temp,  8 );
    PUT_UINT64_LE( X3, temp, 12 );
#endif

    for (i = 0; i < 16; i++)
	{
    	t16 = temp[i] & vmask;
    	output[i] =
				ROTL_SIMLOCK(t16,OUT_ROT1*i,s)^
				ROTL_SIMLOCK(t16,OUT_ROT2*i,s)^
				ROTL_SIMLOCK(t16,OUT_ROT3*i,s)^
				(i*OUT_CONST);
	}
}

int wb_aes_encrypt_cbc(const u8* iv, const u8* input, u32 in_len, u8* output, u32* out_len)
{
	//calculate padding length and padded output length
	if(in_len==0)return -1;

	u8 pad_len = 16 - (in_len % 16);
	*out_len = in_len + pad_len;
	//output block count
	u32 count = (*out_len)/16;

	const u8* ptr_in = input;
	u8* ptr_out = output;
	u32 i,j;
	u8 buf[16];//buffer
	memcpy(buf,iv,16);
	//cbc-encryption
	for(i=0;i<count;++i)
	{
		if(i==count-1)
		{//padded block (last block)
			//xor remaining bytes to buffer
			xor_block_simlock(buf,ptr_in,in_len%16);
			//xor pkcs7 padding to buffer
			for(j=0;j<pad_len;++j)
			{
				buf[15-j] ^= pad_len;
			}
		}
		else
		{//normal block
			xor_block_simlock(buf,ptr_in,16);
		}

		wb_aes_encrypt(buf,ptr_out);
		memcpy(buf,ptr_out,16);

		ptr_in += 16;
		ptr_out += 16;
	}

	return 0;
}

