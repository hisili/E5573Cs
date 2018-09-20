#ifndef _HI_MCI_H_
#define _HI_MCI_H_



/*for fgpa low hz*/
//#define CONFIG_MMC_V7R2_FPGA

/*for mmc0-sd else mmc0-sdio*/
/*#define CONFIG_MMC_SDIO_LOOP*/


#define MMC0_CCLK			    	UHS_SDR50_MAX_DTR /*sdio card 96MHz*/
/*wifi*/
#define MMC1_CCLK			    	UHS_SDR104_MAX_DTR
#ifdef CONFIG_MMC_V7R2
#ifdef CONFIG_MMC_V7R2_FPGA
/*sd*/
#define MMC0_CCLK			    	UHS_SDR25_MAX_DTR
/*wifi*/
#define MMC1_CCLK			    	UHS_SDR25_MAX_DTR
#else
/*sd*/
#define MMC0_CCLK			    	UHS_SDR50_MAX_DTR /*sdio card 96MHz*/
/*wifi*/
#define MMC1_CCLK			    	UHS_SDR104_MAX_DTR
#endif
#elif (defined (CONFIG_MMC_V711))
#undef MMC0_CCLK
#undef MMC1_CCLK
#define MMC0_CCLK			    	(150000000) /*sdio card 100MHz*/
/*wifi*/
#define MMC1_CCLK			    	(150000000)
#endif

#define MMC_CLOCK_SOURCE_480M	480000000
#define MMC_CLOCK_SOURCE_600M	600000000
#if (FEATURE_ON == MBB_COMMON)
#define MMC_CLOCK_SOURCE_15M    15000000
#endif
#define MMC_CLOCK_SOURCE_400K   400000
#define MMC_CLOCK_SOURCE_15M   	15000000
#define ENUM_SPEED_BUS_SPEED	0xf
#define MMC_RESOURCES_SIZE	SZ_4K

#define HI_MAX_REQ_SIZE     (128*1024)

/* maximum size of one mmc block */
#define MAX_BLK_SIZE        512

/* maximum number of bytes in one req */
#define MAX_REQ_SIZE        HI_MAX_REQ_SIZE

/* maximum number of blocks in one req */
#define MAX_BLK_COUNT       (HI_MAX_REQ_SIZE/512)

/* maximum number of segments, see blk_queue_max_phys_segments */
#define MAX_SEGS            (HI_MAX_REQ_SIZE/512)

/* maximum size of segments, see blk_queue_max_segment_size */
#define MAX_SEG_SIZE        HI_MAX_REQ_SIZE





#define DBG(host, fmt, ...)                   \
do { \
	dev_dbg(host->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)

#define ERROR(host, fmt, ...)                   \
do { \
	dev_err(host->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)
	
#define INFO(host, fmt, ...)                   \
do { \
	 dev_info(host->dev, "[%s] "fmt, __func__, ##__VA_ARGS__); \
} while(0)


#endif
