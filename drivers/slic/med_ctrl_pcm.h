#ifndef __MED_CTRL_PCM_H__
#define __MED_CTRL_PCM_H__


#include <linux/kernel.h>
#include "DrvInterface.h"


#ifdef __cplusplus
extern "C" {
#endif


#define CODEC_FRAME_LENGTH_NB   (160)
#define PCM_MAX_FRAME_LENGTH    (CODEC_FRAME_LENGTH_NB)

typedef struct
{
    BSP_U16 ausMicInBuffA[PCM_MAX_FRAME_LENGTH];
    BSP_U16 ausMicInBuffB[PCM_MAX_FRAME_LENGTH];
    BSP_U16 ausSpkOutBuffA[PCM_MAX_FRAME_LENGTH];
    BSP_U16 ausSpkOutBuffB[PCM_MAX_FRAME_LENGTH];
}PCM_BUFFER_STRU;


#ifdef __cplusplus
}
#endif

#endif

