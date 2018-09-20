

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "drv_mailbox.h"
#include "drv_mailbox_cfg.h"
#include "bsp_nvim.h"




unsigned long DRV_MAILBOX_SENDMAIL(
                unsigned long           MailCode,
                void                   *pData,
                unsigned long           Length)
{
    return MAILBOX_OK;
}


unsigned long DRV_MAILBOX_REGISTERRECVFUNC(
                unsigned long           MailCode,
                mb_msg_cb               pFun,
                void                   *UserHandle)
{
    return MAILBOX_OK;
}


unsigned long DRV_MAILBOX_READMAILDATA(
                void                   *MailHandle,
                unsigned char          *pData,
                unsigned long          *pSize)
{
    *pSize = 0;
    return MAILBOX_OK;
}

void drv_hifi_fill_mb_info(unsigned int addr)
{
    return ;
}


