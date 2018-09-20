#ifndef EDMAC_DRV_H
#define EDMAC_DRV_H

#ifndef DMA_TEST
#define DMA_TEST
#endif

#define HIDMAC_TRACE_LEVEL      1

#define HI_EDMAC_BASE            0x90181000 
#define BALONG_V3R2_EDMAC_IRQ	 119

#define EDMAC_DT_NUM       (16)
#define EDMAC_DT_ARM       (0)   /*ACPU=0,CCPU=1*/

#define EDMAC_MAX_CHANNEL        (16)
 
/*config------Bit 31*/
#define EDMAC_TRANSFER_CONFIG_SOUR_INC      (0X80000000)
/*Bit 30*/
#define EDMAC_TRANSFER_CONFIG_DEST_INC      (0X40000000)

/*Bit 27-24*/
#define EDMAC_TRANSFER_CONFIG_SOUR_BURST_LENGTH( _len )  ((UINT32)((_len)<<24))
/*Bit 23-20*/
#define EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH( _len )  ((UINT32)((_len)<<20))

/*Bit18-16*/
#define EDMAC_TRANSFER_CONFIG_SOUR_WIDTH( _len )  ((UINT32)((_len)<<16))

/*Bit14-12*/
#define EDMAC_TRANSFER_CONFIG_DEST_WIDTH( _len )  ((UINT32)((_len)<<12))

/*Bit9-4*/
#define EDMAC_TRANSFER_CONFIG_REQUEST( _ulReg )    ( (_ulReg ) << 4)

/*Bit3-2*/
#define EDMAC_TRANSFER_CONFIG_FLOW_DMAC( _len )  ((UINT32)((_len)<<2))

#define EDMAC_TRANSFER_CONFIG_INT_TC_ENABLE            ( 0x2 )
#define EDMAC_TRANSFER_CONFIG_INT_TC_DISABLE           ( 0x0 )

#define EDMAC_TRANSFER_CONFIG_CHANNEL_ENABLE           ( 0x1 )
#define EDMAC_TRANSFER_CONFIG_CHANNEL_DISABLE          ( 0x0 )

/*Bit 15*/
#define EDMAC_TRANSFER_CONFIG_EXIT_ADD_MODE_A_SYNC     ( 0UL )
#define EDMAC_TRANSFER_CONFIG_EXIT_ADD_MODE_AB_SYNC    ( 0x00008000 )

typedef struct 
{
   channel_isr chan_isr;
   UINT32 chan_arg;
   UINT32 int_status;
}Chan_Isr_Stru;

typedef struct
{
    UINT32  ulInitFlag;
    UINT32  ulFirstFreeNode; 
}EDMAC_FREE_NODE_HEADER_STRU;

typedef struct
{
	UINT32 s_alloc_virt_address;
	UINT32 s_alloc_phys_address;
}EDMA_LLI_ALLOC_ADDRESS_STRU;

typedef struct
{
    UINT32 ulSourAddr;
    UINT32 ulDestAddr;
    UINT32 ulLength;
    UINT32 ulConfig;
}DMA_SIMPLE_LLI_STRU;

typedef struct
{
    BALONG_DMA_CB     lli_node_info;
	volatile UINT32   ulPad10[8];
}EDMAC_TRANSFER_CONFIG_STRU;

typedef struct
{
	volatile UINT32 ulCxCurrCnt1;
	volatile UINT32 ulCxCurrCnt0;
	volatile UINT32 ulCxCurrSrcAddr;
	volatile UINT32 ulCxCurrDesAddr;
}EDMAC_CURR_STATUS_REG_STRU;

/*EDMA ¼Ä´æÆ÷½á¹¹Ìå*/
typedef struct
{
	/*0x0000---*/
	volatile UINT32 ulIntState;
	/*0x0004---*/
	volatile UINT32 ulIntTC1;
	/*0x0008---*/
	volatile UINT32 ulIntTC2;
	/*0x000C---*/
	volatile UINT32 ulIntErr1;
	/*0x0010---*/
	volatile UINT32 ulIntErr2;
	/*0x0014---*/
	volatile UINT32 ulIntErr3;
	/*0x0018---*/
	volatile UINT32 ulIntTC1Mask;
	/*0x001C---*/
	volatile UINT32 ulIntTC2Mask;
	/*0x0020---*/
	volatile UINT32 ulIntErr1Mask;
	/*0x0024---*/
	volatile UINT32 ulIntErr2Mask;
	/*0x0028---*/
	volatile UINT32 ulIntErr3Mask;	
	volatile UINT32 ulPad[(0x40-0x28)/4-1];	
}EDMAC_CPU_REG_STRU;

typedef struct
{
	volatile EDMAC_CPU_REG_STRU stCpuXReg[EDMAC_DT_NUM];
    
	volatile UINT32 ulPad0[(0x600-0x400)/4];
	/*0x0600-- */
	volatile UINT32 ulIntTC1Raw;
	volatile UINT32 ulPad1;
	/*0x0608-- */
	volatile UINT32 ulIntTC2Raw;
	volatile UINT32 ulPad2;
	/*0x0610-- */
	volatile UINT32 ulIntERR1Raw;
	volatile UINT32 ulPad3;
	/*0x0618-- */
	volatile UINT32 ulIntERR2Raw;
	volatile UINT32 ulPad4;
	/*0x0620-- */
	volatile UINT32 ulIntERR3Raw;
	volatile UINT32 ulPad5[(0x660-0x620)/4-1];
	/*0x0660--*/
	volatile UINT32 ulSingleReq;
    volatile UINT32 ulLastSingleReq;
    volatile UINT32 ulBurstReq;
    volatile UINT32 ulLastBurstReq;
    volatile UINT32 ulFlushReq;
    volatile UINT32 ulLastFlushReq;
	volatile UINT32 ulPad6[(0x688-0x674)/4-1];
	/*0x0688--*/
	volatile UINT32 ulChannelPrioritySet;
	volatile UINT32 ulPad7;
	/*0x0690--*/
    volatile UINT32 ulChannelState;
    volatile UINT32 ulPad8;	
	/* 0x0698 -- */
    volatile UINT32 ulDmaCtrl;
    volatile UINT32 ulPad9[(0x0700-0x698)/4-1];
    /* 0x0700 -- */
	volatile EDMAC_CURR_STATUS_REG_STRU stCurrStatusReg[EDMAC_MAX_CHANNEL];
	/* 0x0800 -- */
	volatile EDMAC_TRANSFER_CONFIG_STRU stTransferConfig[EDMAC_MAX_CHANNEL];	
}EDMAC_REG_STRU;

#define EDMAC_MAKE_LLI_ADDR( _p )   (UINT32)( (UINT32)(_p) & 0xFFFFFFE0 )
#define EDMAC_CHANNEL_CB(x)   (s_pstEDMACReg->stTransferConfig[x].lli_node_info)

#endif
