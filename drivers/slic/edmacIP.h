#ifndef EDMAC_IP_H
#define EDMAC_IP_H

#ifndef LOCAL
#define LOCAL static
#endif

#ifndef UINT32
#define UINT32  unsigned int
#define UINT8   unsigned char
#endif

#ifndef NULL
#define NULL    (void *)(0)
#endif

/*
    Common DMA return define.
*/
#define DMA_SUCCESS         0
#define DMA_FAIL            -1

#define DMA_ERROR_BASE                          -100
#define DMA_CHANNEL_INVALID                     (DMA_ERROR_BASE-1)
#define DMA_TRXFERSIZE_INVALID                  (DMA_ERROR_BASE-2)
#define DMA_SOURCE_ADDRESS_INVALID              (DMA_ERROR_BASE-3)
#define DMA_DESTINATION_ADDRESS_INVALID         (DMA_ERROR_BASE-4)
#define DMA_MEMORY_ADDRESS_INVALID              (DMA_ERROR_BASE-5)
#define DMA_PERIPHERAL_ID_INVALID               (DMA_ERROR_BASE-6)
#define DMA_DIRECTION_ERROR                     (DMA_ERROR_BASE-7)
#define DMA_TRXFER_ERROR                        (DMA_ERROR_BASE-8)
#define DMA_LLIHEAD_ERROR                       (DMA_ERROR_BASE-9)
#define DMA_SWIDTH_ERROR                        (DMA_ERROR_BASE-0xa)
#define DMA_LLI_ADDRESS_INVALID                 (DMA_ERROR_BASE-0xb)
#define DMA_TRANS_CONTROL_INVALID               (DMA_ERROR_BASE-0xc)
#define DMA_MEMORY_ALLOCATE_ERROR               (DMA_ERROR_BASE-0xd)
#define DMA_NOT_FINISHED                        (DMA_ERROR_BASE-0xe)
#define DMA_CONFIG_ERROR                        (DMA_ERROR_BASE-0xf)

typedef void (*channel_isr)(UINT32 channel_arg, UINT32 int_status);

/*定义外设请求号*/
typedef enum _BALONG_DMA_REQ
{
    EDMA_DWSSI0_RX = 0,
    EDMA_DWSSI0_TX,
    EDMA_DWSSI1_RX,
    EDMA_DWSSI1_TX,
    EDMA_HIFISIO_RX,
    EDMA_HIFISIO_TX,
    EDMA_HSSPI_RX,
    EDMA_HSSPI_TX,
    EDMA_HSUART_RX,
    EDMA_HSUART_TX,
    EDMA_UART0_RX,
    EDMA_UART0_TX,
    EDMA_UART1_RX,
    EDMA_UART1_TX,
    EDMA_UART2_RX,
    EDMA_UART2_TX,
    EDMA_SCI_RX,
    EDMA_SCI_TX,
    EDMA_UART3_RX,
    EDMA_UART3_TX,
    EDMA_MMC0,     /*20*/
    EDMA_MMC1,
    EDMA_MMC2,
    EDMA_MEMORY,
    EDMA_REQ_MAX    /*如果设备请求不小于此值，则为非法请求*/
} BALONG_DMA_REQ;


#define BALONG_DMA_INT_DONE           1          /*DMA传输完成中断*/
#define BALONG_DMA_INT_LLT_DONE       2          /*链式DMA节点传输完成中断*/
#define BALONG_DMA_INT_CONFIG_ERR     4          /*DMA配置错误导致的中断*/
#define BALONG_DMA_INT_TRANSFER_ERR   8          /*DMA传输错误导致的中断*/
#define BALONG_DMA_INT_READ_ERR       16         /*DMA链表读错误导致的中断*/

#define BALONG_DMA_P2M      1
#define BALONG_DMA_M2P      2
#define BALONG_DMA_M2M      3

#define DMAC_GET_DEST_ADDR       0
#define DMAC_GET_SOUR_ADDR       1

#define EDMA_CHN_BUSY          0   /* 通道空闲 */
#define EDMA_CHN_FREE          1   /* 通道空闲 */

#define EDMAC_CHANNEL_DISABLE       0x0
#define EDMAC_CHANNEL_ENABLE        0x1
#define EDMAC_NEXT_LLI_ENABLE       0x2           /* Bit 1 */
#define EDMA_DATA_TIMEOUT      500

/*链式传输时的节点信息*/
typedef struct _BALONG_DMA_CB 
{
	volatile UINT32 lli;     /*指向下个LLI*/
	volatile UINT32 bindx; 
	volatile UINT32 cindx; 
	volatile UINT32 cnt1; 
	volatile UINT32 cnt0;   /*块传输或者LLI传输的每个节点数据长度 <= 65535字节*/
	volatile UINT32 src_addr; /*物理地址*/
	volatile UINT32 des_addr; /*物理地址*/
	volatile UINT32 config; 
} BALONG_DMA_CB;


#define P2M_CONFIG   (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(0x1) | EDMAC_TRANSFER_CONFIG_DEST_INC)
#define M2P_CONFIG   (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(0x1) | EDMAC_TRANSFER_CONFIG_SOUR_INC)
#define M2M_CONFIG   (EDMAC_TRANSFER_CONFIG_FLOW_DMAC(0x0) | EDMAC_TRANSFER_CONFIG_SOUR_INC | EDMAC_TRANSFER_CONFIG_DEST_INC)

#define EDMAC_BASIC_CONFIG(burst_width, burst_len) \
               ( EDMAC_TRANSFER_CONFIG_SOUR_BURST_LENGTH(burst_len) | EDMAC_TRANSFER_CONFIG_DEST_BURST_LENGTH(burst_len) \
               | EDMAC_TRANSFER_CONFIG_SOUR_WIDTH(burst_width) | EDMAC_TRANSFER_CONFIG_DEST_WIDTH(burst_width) )

/*addr:物理地址*/
#define BALONG_DMA_SET_LLI(addr, last)   ((last)?0:(EDMAC_MAKE_LLI_ADDR(addr) | EDMAC_NEXT_LLI_ENABLE)) 

#define BALONG_DMA_SET_CONFIG(req, direction, burst_width, burst_len) \
                 ( EDMAC_BASIC_CONFIG(burst_width, burst_len) | EDMAC_TRANSFER_CONFIG_REQUEST(req) \
                 | EDMAC_TRANSFER_CONFIG_INT_TC_ENABLE | EDMAC_TRANSFER_CONFIG_CHANNEL_ENABLE \
                 | ((direction == BALONG_DMA_M2M)?M2M_CONFIG:((direction == BALONG_DMA_P2M)?P2M_CONFIG:M2P_CONFIG))) 

void edmac_int (void);
int balong_dma_channel_init (BALONG_DMA_REQ req, channel_isr channel_isr, UINT32 channel_arg, UINT32 int_flag);
int balong_dma_current_transfer_address(UINT32 channel_id);
int balong_dma_channel_stop(UINT32 channel_id);
int balong_dma_channel_is_idle (UINT32 channel_id);     
int balong_dma_channel_set_config (UINT32 channel_id, UINT32 direction, UINT32 burst_width, UINT32 burst_len);
int balong_dma_channel_start (UINT32 channel_id, UINT32 src_addr, UINT32 des_addr, UINT32 len);
int balong_dma_channel_async_start (UINT32 channel_id, unsigned int src_addr, unsigned int des_addr, unsigned int len);
BALONG_DMA_CB *balong_dma_channel_get_lli_addr (UINT32 channel_id);
int balong_dma_channel_lli_start (UINT32 channel_id);
int balong_dma_channel_lli_async_start (UINT32 channel_id);
#endif
