/*lint --e{537}*/
#include "product_config.h"

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ctype.h>

#include <linux/delay.h>
#include <linux/sched.h>


#include "hi_base.h"
#include "hi_syssc.h"
#include "hi_syscrg.h"

#include "osl_types.h"
#include "osl_bio.h"

#include "bsp_version.h"
#include "drv_version.h"
#include "bsp_memmap.h"
#include "drv_dload.h"
#include "bsp_wdt.h"
#include "drv_onoff.h"
#include "hi_syssc_interface.h"

#include "bsp_sram.h"
#include "drv_onoff.h"




#ifndef OK
#define OK 0
#endif
#ifndef ERROR
#define ERROR  (-1)
#endif

/* set 2.5s delay before restart */
static struct delayed_work restart_process;
static bool restart_workqueue_create_flag = false;
static struct workqueue_struct* restart_workqueue = NULL;
#define HUAWEI_UPDATE_START_DELAY_TIME  2500


/* 通过HKADC标示的产品形态，(0:E5,1:stick,2:Phone,3:pad,4:cpe,5:mod,6~9:resv)*/
enum hkac_product_type
{
	HKADC_PRODUCT_TYPE_E5 = 0,
	HKADC_PRODUCT_TYPE_STICK,
	HKADC_PRODUCT_TYPE_PHONE,
	HKADC_PRODUCT_TYPE_PAD,
	HKADC_PRODUCT_TYPE_CPE,
	HKADC_PRODUCT_TYPE_MOD,
    HKADC_PRODUCT_TYPE_CPE_EXT = 8,
};


/*****************************************************************************
*
*兼容以的接口
*
*****************************************************************************/

int BSP_GetProductName (char * pProductName, unsigned int ulLength)
{
	static bool b_geted=false;
	static char* pout_name=NULL;

	if((BSP_NULL == pProductName) || (0 == ulLength))
	{
		return ERROR;
	}

	if(!b_geted){
		b_geted=true;
		pout_name = bsp_version_get_product_out_name();
	}

	if(!pout_name)
	{
		ver_print_error("get product out name failed !\n");
		return ERROR;
	}

    /*lint -save -e732  -e713*/
	strncpy(pProductName, pout_name, StrParamType(ulLength));
    /*lint -restore*/
	return OK;
}

/*****************************************************************************
* 函数	: bsp_get_board_mode_type
* 功能	: get board type(GUL or LTE)
* 输入	: void
* 输出	: void
* 返回	: BOARD_TYPE_E
*****************************************************************************/
BOARD_TYPE_E bsp_get_board_mode_type(void)
{
	return BOARD_TYPE_GUL;
}

bool bsp_board_type_is_gutl(void)
{
	return (BOARD_TYPE_GUL == bsp_get_board_mode_type() ? (bool)1 : (bool)0);
}

BSP_S32 BSP_DLOAD_GetWebuiVer(char *pVersionInfo,BSP_U32 ulLength)
{
    static char webui_ver[VERSION_WEBUI_MAX_LEN] = {0};

    if(VERSION_WEBUI_MAX_LEN > ulLength || NULL == pVersionInfo)
    {
        ver_print_error("pVersionInfo = %d, ulLength = %d error.\n",(int)pVersionInfo,(int)ulLength);
        return ERROR;
	}

    (void)bsp_get_web_version(webui_ver, VERSION_WEBUI_MAX_LEN);

    /*lint -save -e732 -e713 */
    strncpy(pVersionInfo, webui_ver, strlen(webui_ver));
    /*lint -restore*/
    return OK; 
}

BSP_S32 BSP_DLOAD_GetISOVer(char *pVersionInfo,BSP_U32 ulLength)
{
	static bool b_geted = false;
    static char iso_ver[VERSION_ISO_MAX_LEN];

if(VERSION_ISO_MAX_LEN>ulLength||NULL==pVersionInfo)
    {
		ver_print_error("pVersionInfo = %d, ulLength = %d error.\n",(int)pVersionInfo,(int)ulLength);
		return ERROR;
	}

	if(!b_geted){
		b_geted=true;
        bsp_get_iso_version(iso_ver, VERSION_ISO_MAX_LEN);
	}

	if(!iso_ver)
	{
		ver_print_error("get iso version fail.\n");
		return ERROR;
	}

    /*lint -save -e732 -e713 */
	strncpy(pVersionInfo,iso_ver,StrParamType(ulLength));
    /*lint -restore*/
	return OK;
}

/*****************************************************************************
 函 数 名  : BSP_MNTN_GetProductIdInter
 功能描述  : 产品名称完整版本读接口。
 输入参数  : pProductIdInter：为调用者待保存返回的产品名称完整版本的内存首地址；
             ulLength       ：为调用者待保存返回的产品名称完整版本的内存大小；
 输出参数  : 无。
 返 回 值  : 0:  操作成功；
             -1：操作失败。
*****************************************************************************/
int	BSP_MNTN_GetProductIdInter(char * pProductIdInter, unsigned int ulLength)
{
	static bool b_geted=false;
	static char* inner_name=NULL;

	if(!b_geted){
		b_geted=true;
		inner_name = bsp_version_get_product_inner_name();
	}

	if(!inner_name)
	{
		ver_print_error("get product inner name failed !\n");
		return ERROR;
	}

    /*lint -save -e732 -e713*/
	strncpy(pProductIdInter,inner_name,StrParamType(ulLength));
    /*lint -restore*/
	return OK;
}

/*****************************************************************************
* 函 数 名  : BSP_GetProductInnerName
* 功能描述  : 获取内部产品名称
* 输入参数  : char* pProductIdInter,    字符串指针
*             BSP_U32 ulLength,         缓冲区长度
* 输出参数  : 无
* 返 回 值  : 0：正确，非0: 失败
* 修改记录  :
*****************************************************************************/
BSP_S32 BSP_GetProductInnerName (char * pProductIdInter, unsigned int ulLength)
{

	static bool b_geted=false;
	static char* inner_name=NULL;

	if(!b_geted){
		b_geted=true;
		inner_name = bsp_version_get_product_inner_name();
	}

	if(!inner_name)
	{
		ver_print_error("get product inner name failed !\n");
		return ERROR;
	}

    /*lint -save -e732 -e713*/
	strncpy(pProductIdInter,inner_name,StrParamType(ulLength));
    /*lint -restore*/
	return OK;
}
/*****************************************************************************
* 函 数 名  : BSP_HwGetHwVersion
* 功能描述  : 获取硬件版本名称
* 输入参数  : char* pHwVersion,字符串指针，保证不小于32字节
* 输出参数  : 无
* 返 回 值  : 无
* 修改记录  :
*****************************************************************************/
BSP_S32 BSP_HwGetHwVersion (char* pFullHwVersion, BSP_U32 ulLength)
{
	static bool b_geted=false;
	static char* hw_version=NULL;

	if(!b_geted){
		b_geted=true;
		hw_version = bsp_version_get_hardware();
	}

	if(!hw_version)
	{
		ver_print_error("get product hardware version failed !\n");
		return ERROR;
	}

    /*lint -save -e732 -e713*/
	strncpy(pFullHwVersion,hw_version,StrParamType(ulLength));
    /*lint -restore*/
	return OK;
}

/*****************************************************************************
 函 数 名  : bsp_version_get_platform_info
 功能描述  : 获取芯片的版本号
 输入参数  : 无
 输出参数  : Platform_info:芯片的版本号
 返 回 值  : void
*****************************************************************************/
static void bsp_version_get_platform_info(unsigned long *platform_info)
{
	/*board FPGA*/
	u32 product_type = 0xff;
	product_type = bsp_version_get_board_type();
	if(HW_VER_INVALID==product_type){
		ver_print_error("get board chip type fail!\n");
		return ;
	}

	switch(product_type){
	case HW_VER_PRODUCT_P531_FPGA:
	case HW_VER_K3V3_FPGA:
		*platform_info=VER_SOC;
		break;
	default:
		*platform_info=VER_ASIC;
	}

}
/*****************************************************************************
 函 数 名  : DRV_GET_PLATFORM_INFO
 功能描述  : 获取芯片的版本号
 输入参数  : 无
 输出参数  : u32PlatformInfo:芯片的版本号
 返 回 值  : void
*****************************************************************************/
void DRV_GET_PLATFORM_INFO(unsigned long *u32PlatformInfo)
{
	static bool b_geted=false;
	static unsigned long platform_info = 0xffffffff;

	if(!b_geted){
		b_geted=true;
		bsp_version_get_platform_info(&platform_info);
	}

	*u32PlatformInfo = platform_info;
	return ;
}

/*****************************************************************************
* 函 数 名  : BSP_OM_GetChipType
*
* 功能描述  : 获取芯片类型
*
* 输入参数  : void
*
* 输出参数  : 无
*
* 返 回 值  : 芯片类型
*             PV500_CHIP:PV500芯片
*             V7R1_CHIP: V7R1芯片
*
* 其它说明  : 无
*
*****************************************************************************/
BSP_CHIP_TYPE_E BSP_OM_GetChipType(void)
{
	return V7R2_CHIP;
}

/*************************************************
 函 数 名   : BSP_MNTN_ProductTypeGet
 功能描述: 返回当前产品类型
 输入参数: 无
 输出参数: 无
 返 回 值   :
 		0:STICK
         1:MOD
         2:E5
         3:CPE
         4:HILINK
         5:WINGLE
*************************************************/
BSP_U32 BSP_MNTN_ProductTypeGet(void)
{
    unsigned int id = 0xffffffff;
	eProductType product_type = PRODUCT_TYPE_STICK;	/* default to stick */

	id = (unsigned int)bsp_version_get_boardid();

	if(HW_VER_INVALID==id){
		ver_print_error("[bsp_get_board_chip_type]:get hw version failed!hw_ver:0x%x\n",id);
		return product_type;
	}

	/* UDP默认返回stick形态 */
	if(HW_VER_UDP_MASK == (id & HW_VER_UDP_MASK))
	{
		return product_type;
	}
	/* 其他产品版，根据hakdc获取的形态返回 */
	switch((enum hkac_product_type)((id & HW_VER_PRODUCT_TYPE_MASK) >> HW_VER_PRODUCT_TYPE_OS))
	{
    case HKADC_PRODUCT_TYPE_E5:
		product_type = PRODUCT_TYPE_E5;
		break;

    case HKADC_PRODUCT_TYPE_STICK:
	case HKADC_PRODUCT_TYPE_MOD:
		product_type = PRODUCT_TYPE_MOD;
		break;

	case HKADC_PRODUCT_TYPE_CPE: /* fall through */
    case HKADC_PRODUCT_TYPE_CPE_EXT:
		product_type = PRODUCT_TYPE_CPE;
		break;

	default:/* default to stick */
		product_type = PRODUCT_TYPE_STICK;
		break;
	}

	return (BSP_U32)product_type;
}

/*****************************************************************************
 函 数 名  : BSP_MNTN_GetHwGpioInfo
 功能描述  : Get flash infomation
 输入参数  : 无。
 输出参数  : 无。
 返 回 值  : 无。
*****************************************************************************/
int BSP_MNTN_GetHwGpioInfo(unsigned char *pGpioInfo, unsigned long usLength )
{
	///*lint -save -e438 -e830*/
	//pGpioInfo = pGpioInfo;
	//usLength = usLength;
	///*lint -restore*/
    return 0;
}

BSP_S32 bsp_dload_set_datalock_state(void)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        ver_print_error("Dload smem_data malloc fail!\n");
        return VER_ERROR;
    }

    smem_data->smem_datalock_status = SMEM_DATALOCK_STATUS_FLAG_NUM;
    ver_print_error("Dload smem_data set datalock state 0x%x\n", smem_data->smem_datalock_status);
    return VER_OK;
}

/***********************************************************************/


/*****************************************************************************
* 函 数 名  : BSP_GetBuildVersion
*
* 功能描述  :
*
* 输入参数  :
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 修改记录  : 2011-3-29 wuzechun creat
*
*****************************************************************************/
UE_SW_BUILD_VER_INFO_STRU* BSP_GetBuildVersion(void)
{
#define MAX_VER_SECTION 8
#define VER_PART_LEN 3
#define VERC_PART_LEN 2

    static UE_SW_BUILD_VER_INFO_STRU st = {0,0,0,0,0,0,0,{0},{0}};
	static bool binited = false;
	int ret;

    char * pCur, * pFind;
    char au8Args[MAX_VER_SECTION] = {0};
	u32 verno = 0;

	if(!binited)
	{
		pCur = PRODUCT_VERSION_STR;
	    pFind = pCur;

        memset(&st, 0, sizeof(UE_SW_BUILD_VER_INFO_STRU));

	    while (NULL != (pCur = strchr(pCur, 'V')) )
	    {
	        pCur++;

	        if (isdigit(*(pCur)))
	        {
	            memset(au8Args, 0, MAX_VER_SECTION);
	            strncpy(au8Args, pCur, VER_PART_LEN);
				ret = strict_strtoul(au8Args, 10, (unsigned long *)&verno);
				if(ret)
				{
					ver_print_error("kstrtouint error:ret %#x, au8Args:%s\n", ret, au8Args);
					return (&st);
				}
	            st.ulVVerNo = (u16)verno;
	            break;
	        }
	    }
	    pCur = pFind;
	    while (NULL != (pCur = strchr(pFind, 'R')) )
	    {
	        pCur++;

	        if (isdigit(*(pCur-2)) && isdigit(*(pCur)))
	        {
	            memset(au8Args, 0, MAX_VER_SECTION);
	            strncpy(au8Args, pCur, VER_PART_LEN);
				ret = strict_strtoul(au8Args, 10, (unsigned long *)&verno);
				if(ret)
				{
					ver_print_error("kstrtouint error:ret %#x, au8Args:%s\n", ret, au8Args);
					return (&st);
				}
	            st.ulRVerNo = (u16)verno;
	            break;
	        }
	    }
	    pCur = pFind;
	    while (NULL != (pCur = strchr(pFind, 'C')) )
	    {
	        pCur++;

	        if (isdigit(*(pCur-2)) && isdigit(*(pCur)))
	        {
	            memset(au8Args, 0, MAX_VER_SECTION);
	            strncpy(au8Args, pCur, VERC_PART_LEN);
	            pCur += VERC_PART_LEN;
				ret = strict_strtoul(au8Args, 10, (unsigned long *)&verno);
				if(ret)
				{
					ver_print_error("kstrtouint error:ret %#x, au8Args:%s\n", ret, au8Args);
					return (&st);
				}
	            st.ulCVerNo = (u16)verno;
	            break;
	        }
	    }
	    pCur = pFind;
	    while (NULL != (pCur = strchr(pFind, 'B')) )
	    {
	        pCur++;

	        if (isdigit(*(pCur-2)) && isdigit(*(pCur)))
	        {
	            memset(au8Args, 0, MAX_VER_SECTION);
	            strncpy(au8Args, pCur, VER_PART_LEN);
				ret = strict_strtoul(au8Args, 10, (unsigned long *)&verno);
				if(ret)
				{
					ver_print_error("kstrtouint error:ret %#x, au8Args:%s\n", ret, au8Args);
					return (&st);
				}
	            st.ulBVerNo = (u16)verno;
	            break;
	        }
	    }
	    pCur = pFind;
	    while (NULL != (pCur = strstr(pFind, "SPC")) )
	    {
	        pCur+=3;

	        if (isdigit(*(pCur-4)) && isdigit(*(pCur)))
	        {
	            memset(au8Args, 0, MAX_VER_SECTION);
	            strncpy(au8Args, pCur, VER_PART_LEN);
				ret = strict_strtoul(au8Args, 10, (unsigned long *)&verno);
				if(ret)
				{
					ver_print_error("kstrtouint error:ret %#x, au8Args:%s\n", ret, au8Args);
					return (&st);
				}
	            st.ulSpcNo = (u16)verno;
	            break;
	        }
	    }

		st.ulProductNo = BSP_MNTN_ProductTypeGet();

        /* coverity[buffer_size_warning] */
		strncpy(st.acBuildDate, __DATE__, BUILD_DATE_LEN);

        /* coverity[buffer_size_warning] */
		strncpy(st.acBuildTime, __TIME__, BUILD_TIME_LEN);
	}

	return (&st);
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetCurMode
*
* 功能描述  : 设置当前工作模式
*
* 输入参数  : void
* 输出参数  : DLOAD_MODE_NORMAL     :正常模式
*             DLOAD_MODE_DATA       :数据模式
*             DLOAD_MODE_DOWNLOAD   :下载模式
*
* 返 回 值  : 无
*
* 其它说明  : AT模块调用
*             正常模式支持的有:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DOWNLOAD)
*             下载模式支持的有:
*                 BSP_DLOAD_GetCurMode(DLOAD_MODE_DATA)
*
*****************************************************************************/
void BSP_DLOAD_SetCurMode(DLOAD_MODE_E eDloadMode)
{
	/*先打桩，后面产品线实现*/
	return ;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetCurMode
*
* 功能描述  : 获取当前工作模式
*
* 输入参数  : void
* 输出参数  : 无
*
* 返 回 值  : DLOAD_MODE_NORMAL     :正常模式
*             DLOAD_MODE_DATA       :数据模式
*             DLOAD_MODE_DOWNLOAD   :下载模式
*
* 其它说明  : 无
*
*****************************************************************************/
DLOAD_MODE_E BSP_DLOAD_GetCurMode(void)
{
	/*暂时打桩，由产品线实现*/
    return DLOAD_MODE_NORMAL;
}
/*****************************************************************************
* 函 数 名  : BSP_DLOAD_SetSoftLoad
*
* 功能描述  : 设置启动标志
*
* 输入参数  : BSP_BOOL bSoftLoad  :
*             BSP_FALSE :从vxWorks启动
*             BSP_TRUE  :从bootrom启动
* 输出参数  : 无
*
* 返 回 值  : 无
*
* 其它说明  : 无
*
*****************************************************************************/
void BSP_DLOAD_SetSoftLoad (BSP_BOOL bSoftLoad)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        ver_print_error("Dload smem_data malloc fail!\n");
        return;
    }
    
    if(BSP_TRUE == bSoftLoad)
    {
        smem_data->smem_dload_flag = SMEM_DLOAD_FLAG_NUM;
    }
    else
    {
        smem_data->smem_dload_flag = 0;
    }
    
    if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        smem_data->smem_switch_pcui_flag = 0;
    }
    return;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetSoftLoad
*
* 功能描述  : 获取启动标志
*
* 输入参数  : void
* 输出参数  : 无
*
* 返 回 值  : BSP_TRUE  :从bootrom启动
*             BSP_FALSE :从vxWorks启动
*
* 其它说明  : 无
*
*****************************************************************************/
BSP_BOOL BSP_DLOAD_GetSoftLoad (void)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    
    if (NULL == smem_data)
    {
        ver_print_error("Dload smem_data malloc fail!\n");
        return BSP_FALSE;
    }

    if(SMEM_DLOAD_FLAG_NUM == smem_data->smem_dload_flag )
    {
        return BSP_TRUE;
    }
    else
    {
        return BSP_FALSE;
    }
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_SendData
*
* 功能描述  : 通过虚拟串口向PC端发送数据
*
* 输入参数  : pBuf      :数据缓冲区
*             u32Len    :数据缓冲区长度
* 输出参数  : pBuf      :数据缓冲区
*
* 返 回 值  : 无
*
* 其它说明  : pBuf必须保证cache line(32字节)对齐
*
*****************************************************************************/
BSP_S32 BSP_DLOAD_SendData(char *pBuf, BSP_U32 u32Len)
{
	/*TODO:*/
	return OK;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_AtProcReg
*
* 功能描述  : 注册AT处理函数
*
* 输入参数  : pFun
* 输出参数  : 无
*
* 返 回 值  : DLOAD_OK:成功
*
* 其它说明  : 无
*
*****************************************************************************/

BSP_S32 BSP_DLOAD_AtProcReg (BSP_DLOAD_AtCallBack pFun)
{
    return DLOAD_OK;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetDloadVer
*
* 功能描述  : 查询下载协议版本号。该信息BSP固定写为2.0。
*
* 输入参数  :
				char *str  :字符串缓冲区
*             		BSP_S32 len      :字符串长度
* 输出参数  : BSP_S8 *str   :字符串缓冲区
*
* 返 回 值  : DLOAD_ERROR   :输入参数非法
*             其它          :返回字符串长度
*
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
BSP_S32 BSP_DLOAD_GetDloadVer(char *str, BSP_U32 len)
{

    ver_print_info("getDloadVersion\r\n");

    if((NULL == str) || ((UINT32)len < sizeof(DLOAD_VERSION)))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }

    memset(str, 0x0, (UINT32)len);
    memcpy(str, DLOAD_VERSION, sizeof(DLOAD_VERSION));

    return VER_OK;
}

int BSP_DLOAD_GetDloadInfo(unsigned char atCmdBuf[], unsigned int dloadType)
{
	int ret=VER_OK;
	static bool b_geted=false;
    static struct dload_info_type dload_info={"\0","\0","\0"};

	if(!b_geted){
		b_geted=true;
		ret=bsp_dload_get_dload_info(&dload_info);
		if(VER_OK!=ret){
			ver_print_error("get dload info failed !\n");
			return VER_ERROR;
		}
	}
	/* coverity[secure_coding] */
	sprintf((char *)atCmdBuf,"\r\n\r\nswver:%s\r\n\
	    \r\niso:%s\r\n\
	    \r\nproduct name:%s\r\n\
	    \r\ndload type:%d\r\n\
	    \r\nOK\r\n",dload_info.software_ver,dload_info.iso_ver,dload_info.product_name,dloadType);

	return VER_OK;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetDloadType
*
* 功能描述  : 获取下载类型
*
* 输入参数  : void
* 输出参数  : 无
*
* 返 回 值  : 0 :正常模式
*             1 :强制模式
*
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*
*****************************************************************************/
BSP_S32 BSP_DLOAD_GetDloadType(void)
{
    if(DLOAD_MODE_NORMAL == BSP_DLOAD_GetCurMode())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetProductId
*
* 功能描述  : 获取产品名称字符串
*
* 输入参数  : BSP_S8 *str   :字符串缓冲区
*             BSP_S32 len   :字符串长度
* 输出参数  : BSP_S8 *str   :字符串缓冲区
*
* 返 回 值  : DLOAD_ERR_INVALID_PARA    :输入参数非法
*             VER_ERROR                :失败
*             其它                      :返回字符串长度
*
* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
*
*****************************************************************************/
BSP_S32 BSP_DLOAD_GetProductId(char *str, BSP_U32 len)
{
	static bool b_geted=false;
	static char* product_id=NULL;

    if((NULL == str) || (len == 0))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }

	if(!b_geted){
		b_geted=true;
		product_id = bsp_version_get_dloadid();
	}

	if(!product_id)
	{
		ver_print_error("get product dload ID failed !\n");
		return VER_ERROR;
	}

    /*lint -save -e732 -e713 */
	strncpy(str,product_id,len);
    /*lint -restore*/
	return VER_OK;
}

/*****************************************************************************
* 函 数 名  	: BSP_DLOAD_GetSoftwareVer
* 功能描述  	: 获取软件版本号
* 输入参数  	:
	BSP_S8 *str   	: 字符串缓冲区
*     BSP_S32 len   : 字符串长度

* 输出参数  	:
	BSP_S8 *str   	: 字符串缓冲区

* 返 回 值  	:
	BSP_ERROR    : 输入参数非法
*             其它 	: 返回字符串长度
* 其它说明  	:  AT模块调用
*             正常模式支持
*             下载模式支持
*****************************************************************************/
BSP_S32 BSP_DLOAD_GetSoftwareVer(char *str, BSP_U32 len)
{
	static bool b_geted=false;
	static char* software_ver=NULL;

    if((NULL == str) || (len == 0))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }


	if(!b_geted){
		b_geted=true;
		software_ver = bsp_version_get_firmware();
	}

	if(!software_ver)
	{
		ver_print_error("get software version failed !\n");
		return VER_ERROR;
	}

    /*lint -save -e732 -e713*/
	strncpy(str,software_ver,len);
    /*lint -restore*/
	return VER_OK;

}


/*****************************************************************
* 函 数 名  	: huawei_set_upinfo
* 功能描述  	: 检测set flag info
* 输入参数  	: 无
* 输出参数  	: 无
* 返 回 值  	: void
******************************************************************/
BSP_S32 huawei_set_upinfo(char* pData)
{
    (void)huawei_set_update_info(pData);
    return VER_OK;
}

void huawei_get_upinfo_times(BSP_U32* times)
{
    (void)huawei_get_update_info_times(times);
    return;
}
BSP_S32 huawei_get_spec_upinfo(char* pData, BSP_U32 pDataLen, BSP_U32 num)
{
    BSP_S32 ret = VER_ERROR;
    ret = huawei_get_spec_num_upinfo(pData, pDataLen, num);
    return ret;
}
/*****************************************************************
* 函 数 名  	: restart_func
* 功能描述  	: MBB 调用海思重启接口，延时2.5s
* 输入参数  	: 工作队列
* 输出参数  	: 无
* 返 回 值  	: void
******************************************************************/
static void restart_func(struct work_struct* data)
{
    huawei_smem_info *smem_data = NULL;
    smem_data = (huawei_smem_info *)SRAM_DLOAD_ADDR;
    if (NULL == smem_data)
    {
        return;
    }
    if(SMEM_SWITCH_PUCI_FLAG_NUM == smem_data->smem_switch_pcui_flag)
    {
        smem_data->smem_switch_pcui_flag = 0;
        printk("%s-> smem_switch_pcui_flag = 0\n",__func__);
    }
    /* 调用海思reboot接口 */
    printk(KERN_ERR "######ready to restart!######\n"); 
    DRV_SHUT_DOWN(DRV_SHUTDOWN_RESET);
    return;
}

/*****************************************************************
* 函 数 名  	: BSP_OM_SoftReboot
* 功能描述  	: 重启单板
* 输入参数  	:

* 输出参数  	:
* 返 回 值  	:

* 其它说明  : AT模块调用
*             正常模式支持
*             下载模式支持
******************************************************************/
void BSP_OM_SoftReboot(void)
{
    power_on_wdt_cnt_set();

    /* set 2.5s delay before restart */
    if(false == restart_workqueue_create_flag)
    {
        restart_workqueue = create_workqueue("huaweirestart");
        if(NULL == restart_workqueue)
        {
            printk(KERN_ERR "restart create_workqueue err and restart now!\n");
            DRV_SHUT_DOWN(DRV_SHUTDOWN_RESET);
            return;
        }
        INIT_DELAYED_WORK(&restart_process, restart_func);

        restart_workqueue_create_flag = true;
    }
    else
    {
        printk(KERN_ERR "restart create_workqueue have created!\n");
    }

    queue_delayed_work(restart_workqueue, 
        &restart_process, msecs_to_jiffies(HUAWEI_UPDATE_START_DELAY_TIME));
    return;
}

/*****************************************************************************
* 函 数 名  : BSP_DLOAD_GetNVBackupFlag
*
* 功能描述  : 获取升级前是否进行NV备份标志
*
* 输入参数  : 无
*
* 输出参数  : 无
*
* 返 回 值  : BSP_TRUE  :备份NV
*            BSP_FALSE :不备份NV
*
* 其它说明  : 此接口只对非一键式升级方式（SD升级/在线升级）有效，一键式升级会发AT命令设置
*
*****************************************************************************/
BSP_BOOL BSP_DLOAD_GetNVBackupFlag(void)
{
	return 0;
}

/*****************************************************************
* 函 数 名  	: BSP_DLOAD_BaselineVer
* 功能描述  	: 查询海思基线版本
* 输入参数  	:

* 输出参数  	:
* 返 回 值  	:

* 其它说明  : AT模块调用
*             
******************************************************************/
BSP_S32 BSP_DLOAD_BaselineVer(BSP_CHAR *str, BSP_U32 len)
{
    static bool b_geted = false;
    static char* base_software_ver = NULL;

    if((NULL == str) || (len <= 0))
    {
        ver_print_error("param error!\n");
        return VER_ERROR;
    }

    if(!b_geted)
    {
        b_geted = true;
        base_software_ver = bsp_version_get_baseline();
    }

    if(!base_software_ver)
    {
        ver_print_error("get software version failed !\n");
        return VER_ERROR;
    }

    strncpy(str,base_software_ver,len);
    return VER_OK;
}
