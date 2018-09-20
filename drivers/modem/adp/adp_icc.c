

#include <icc_balong.h>


extern struct icc_init_info g_icc_init_info[];

#define ICC_DEFAULT_SUB_CHANNEL   (0)

s32 icc_read_cb_wraper(u32 channel_id , u32 len, void* context);
s32 icc_write_cb_wraper(u32 real_channel_id , void* context);

s32 icc_read_cb_wraper(u32 channel_id , u32 len, void* context)
{
	icc_read_cb cb = (icc_read_cb)context;

	if(cb)
	{
		return (s32)cb(GET_CHN_ID(channel_id),(s32)len);
	}

	return ICC_OK;
}

s32 icc_write_cb_wraper(u32 real_channel_id , void* context)
{
	icc_write_cb cb = (icc_write_cb)context;
	if(cb)
	{
		return (s32)cb(real_channel_id);
	}

	return ICC_OK;
}

BSP_S32 BSP_ICC_Open(BSP_U32 u32ChanId, ICC_CHAN_ATTR_S *pChanAttr)
{
	u32 func_id = ICC_DEFAULT_SUB_CHANNEL; /* 除ifc通道外，其他通道只有一个回调 */
	u32 channel_id = u32ChanId << 16 | func_id;
	u32 i = 0;
	u32 real_channel_size = bsp_icc_channel_size_get();

	/* coverity[REVERSE_INULL] */
	if(!pChanAttr)
	{
		icc_print_error("pChanAttr is null!\n");
		goto out;
	}
	else if(pChanAttr->u32FIFOOutSize != pChanAttr->u32FIFOInSize || u32ChanId >= ICC_CHN_ID_MAX)
	{
		icc_print_error("invalid param u32ChanId[%d],pChanAttr[0x%x],fifo_in[0x%x],fifo_out[0x%x]\n",
			u32ChanId, (s32)pChanAttr, pChanAttr->u32FIFOInSize, pChanAttr->u32FIFOOutSize);
		goto out;
	}

	for(i = 0; i < real_channel_size; i++)
	{
		if(u32ChanId == g_icc_init_info[i].real_channel_id)
		{
			break;
		}
	}

	if(i >= real_channel_size)
	{
		icc_print_error("channel_id[%d] cannot find in g_icc_init_info array\n", u32ChanId);
		goto out;
	}
	else
	{
		if(pChanAttr->u32FIFOInSize > g_icc_init_info[i].fifo_size)/*lint !e574 */
		{
			icc_print_error("user fifo_size(%d) > fifo_size(%d) defined in bsp(g_icc_init_info)\n",
					 pChanAttr->u32FIFOInSize, g_icc_init_info[i].fifo_size);
			goto out;
		}
	}

	return (BSP_S32)bsp_icc_event_register(channel_id, icc_read_cb_wraper, (void*)pChanAttr->read_cb, icc_write_cb_wraper, (void*)pChanAttr->write_cb);

out:
	return BSP_ERR_ICC_INVALID_PARAM;
}

BSP_S32 BSP_ICC_Close(BSP_U32 u32ChanId)
{
	bsp_icc_channel_uninit((u32)u32ChanId);
	return ICC_OK;
}
BSP_S32 BSP_ICC_Write(BSP_U32 u32ChanId, BSP_U8* pData, BSP_S32 s32Size)
{
	u32 channel_id = (u32ChanId << 16 | (u16)ICC_DEFAULT_SUB_CHANNEL);
	
	if(!pData || u32ChanId >= ICC_CHN_ID_MAX)
	{
		icc_print_error("invalid param[%d], pData[0x%x]\n", u32ChanId, (u32)pData);
		return BSP_ERR_ICC_INVALID_PARAM;
	}

	return (BSP_S32)bsp_icc_send(ICC_SEND_CPU, channel_id, (u8*)pData, (u32)s32Size);
}

BSP_S32 BSP_ICC_Read(BSP_U32 u32ChanId, BSP_U8* pData, BSP_S32 s32Size)
{
	u32 channel_id = (u32ChanId << 16 | (u16)ICC_DEFAULT_SUB_CHANNEL);
	
	if(!pData || u32ChanId >= ICC_CHN_ID_MAX)
	{
		icc_print_error("invalid param[%d], pData[0x%x]\n", u32ChanId, (u32)pData);
		return BSP_ERR_ICC_INVALID_PARAM;
	}

	return (BSP_S32)bsp_icc_read(channel_id, (u8*)pData, (u32)s32Size);
}

BSP_S32 BSP_ICC_Ioctl(BSP_U32 u32ChanId, BSP_U32 cmd, BSP_VOID *param)
{
	u32 channel_id = (u32ChanId << 16 | (u16)ICC_DEFAULT_SUB_CHANNEL);/* [false alarm]:屏蔽Fortify错误 */
	s32 ret = ICC_OK;
	
	if(u32ChanId >= ICC_CHN_ID_MAX)
	{
		icc_print_error("invalid param[%d].\n", u32ChanId);
		return BSP_ERR_ICC_INVALID_PARAM;
	}
	switch(cmd)
	{
		case ICC_IOCTL_SET_READ_CB:
		{
			ret = (s32)bsp_icc_event_register(channel_id, icc_read_cb_wraper ,(void*)param, NULL, NULL);
			break;
		}
		case ICC_IOCTL_SET_WRITE_CB:
		{
			ret = (s32)bsp_icc_event_register(channel_id, NULL, NULL, icc_write_cb_wraper ,(void*)param);
			break;
		}
		case ICC_IOCTL_GET_STATE:
		{
			ret = (s32)bsp_icc_channel_status_get((u32)u32ChanId, (u32*)param);
			break;
		}
		case ICC_IOCTL_SET_EVENT_CB:
		{
			break;
		}
		default:
		{
			icc_print_error("channel[%d] invalid cmd[%d].\n", u32ChanId, cmd);
			ret = BSP_ERR_ICC_INVALID_PARAM;
			break;
		}
	}
	
	return ret;
}
BSP_S32 BSP_ICC_Debug_Register(BSP_U32 u32ChanId, FUNCPTR_1 debug_routine, int param)
{
	return bsp_icc_debug_register(u32ChanId,debug_routine,param);
}


