#include"wb_aes_decrypt.h"
#include"wb_const.h"
#include"table2.h"
#include "DrvInterface.h"
#define TB2 table2_simlock
void wb_aes_decrypt_simlock(const u8* input, u8* output)
{
	int i;
    u8 s = WB_EXP_SIZE2;
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

	//put transformed ciphertext into Xs
#if WB_EXP_SIZE2==8
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

	//decryption
    WB_AES_BROUND(TB2,Y,X,0);
	WB_AES_BROUND(TB2,X,Y,1);
	WB_AES_BROUND(TB2,Y,X,2);
	WB_AES_BROUND(TB2,X,Y,3);
	WB_AES_BROUND(TB2,Y,X,4);
	WB_AES_BROUND(TB2,X,Y,5);
	WB_AES_BROUND(TB2,Y,X,6);
	WB_AES_BROUND(TB2,X,Y,7);
	WB_AES_BROUND(TB2,Y,X,8);
	WB_AES_BROUND(TB2,X,Y,9);
#if WB_NR2>10
	WB_AES_BROUND(TB2,Y,X,10);
	WB_AES_BROUND(TB2,X,Y,11);
#endif
#if WB_NR2>12
	WB_AES_BROUND(TB2,Y,X,12);
	WB_AES_BROUND(TB2,X,Y,13);
#endif

	//output transformation
#if WB_EXP_SIZE2==8
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

int wb_aes_decrypt_cbc_simlock(const u8* iv, const u8* input, u32 in_len, u8* output, u32* out_len)
{
	//not accept input (ciphertext) length without length of multiples of 16 bytes
	if(in_len==0 || (in_len % 16)!=0)return -1;
	u32 count = in_len/16;

	u32 i,j;
	const u8 *ptr_in = input;
	u8 *ptr_out = output;
	u8 buf[16],temp[16];
	u8 pad;
	//cbc-decryption
	memcpy(buf,iv,16);
	for(i=0;i<count;++i)
	{
		if(i==count-1)
		{
			wb_aes_decrypt_simlock(ptr_in,temp);
			xor_block_simlock(temp,buf,16);
			//check pkcs7 padding
			pad = temp[15];
			if(pad>16)return -1;
			for(j=1;j<pad;++j)
			{
				if(temp[15-j]!=pad)return -1;
			}
			*out_len = in_len - pad;
			memcpy(ptr_out,temp,16-pad);
		}
		else
		{
			wb_aes_decrypt_simlock(ptr_in,ptr_out);
			xor_block_simlock(ptr_out,buf,16);
		}
		memcpy(buf,ptr_in,16);

		ptr_in+=16;
		ptr_out+=16;
	}


	return 0;
}
