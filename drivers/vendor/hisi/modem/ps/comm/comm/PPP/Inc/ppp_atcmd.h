

#include "vos.h"
#include "PppInterface.h"

#pragma pack(4)

extern VOS_UINT32 PPP_ProcTeConfigInfo (VOS_UINT16 usPppId, PPP_REQ_CONFIG_INFO_STRU *pPppReqConfigInfo);
extern VOS_UINT32 PPP_ProcPppRelEvent (VOS_UINT16 usPppId);
extern VOS_UINT32 PPP_ProcPppDisconnEvent (VOS_UINT16 usPppId);
extern VOS_UINT32 Ppp_SndPPPDisconnIndtoAT(VOS_UINT16 usPppId);
#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

