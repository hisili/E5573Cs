

#ifndef __DRV_MD5_H__
#define __DRV_MD5_H__

#include "drv_comm.h"

/*************************MD5相关 START*******************************/

/*****************************************************************************
 函 数 名  : VerifySIMLock
 功能描述  : 判断当前解锁码是否正确 .
 输入参数  : imei       - 单板IMEI号
             unlockcode - 解锁码
 输出参数  : 无。
 返 回 值  : 1：   解锁成功
             0:    解锁失败
*****************************************************************************/
extern int VerifySIMLock(char* UnlockCode, char* Key);
#define DRV_CARDLOCK_MD5_VERIFY(unlockcode, imei)  VerifySL(unlockcode, imei)
#ifdef CONFIG_SIMLOCK_2_1
extern int VerifySL(char* UnlockCode, char* Key);
#endif


extern int GetAuthVer(void);
#define DRV_GET_AUTH_VER()  GetAuthVer()

/*************************MD5相关 END*********************************/

#endif

