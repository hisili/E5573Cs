

#ifndef __DRV_EFUSE_H__
#define __DRV_EFUSE_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include "drv_comm.h"


#define     BUF_ERROR          (-55)
#define     LEN_ERROR          (-56)
#define     READ_EFUSE_ERROR   (-57)




extern int efuseWriteHUK(char *pBuf,unsigned int len);

#define EFUSE_WRITE_HUK(pBuf,len) efuseWriteHUK(pBuf,len)



extern int CheckHukIsValid(void);
#define DRV_CHECK_HUK_IS_VALID() CheckHukIsValid()


extern int DRV_GET_DIEID(unsigned char* buf,int length);


extern int DRV_GET_CHIPID(unsigned char* buf,int length);

#ifdef __cplusplus
}
#endif

#endif

