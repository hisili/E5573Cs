
#ifndef __HI_ABB_H__
#define __HI_ABB_H__

#include <osl_bio.h>
#include <bsp_memmap.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ABB_V7R2
#define ABB_VERSION_ADDR (0xA0)

static __inline__ void hi_abb_control_enable(int id)
{

}

static __inline__ void hi_abb_control_disable(int id)
{

}

static __inline__ int hi_abb_control_status(int id)
{
    return 0;
}

static __inline__ int hi_abb_get_buffer_status(int id)
{
    return 1;
}

static __inline__ int hi_abb_get_finish_status(int id)
{
    return 0;
}


#ifdef __cplusplus
}
#endif

#endif
