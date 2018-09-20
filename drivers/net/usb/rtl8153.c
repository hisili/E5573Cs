/*
 *  Copyright (c) 2013 Realtek Semiconductor Corp. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 */

#include <linux/init.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/mii.h>
#include <linux/mdio.h>
#include <linux/ethtool.h>
#include <linux/usb.h>
#include <linux/crc32.h>
#include <linux/if_vlan.h>
#include <linux/uaccess.h>
#include "compatibility.h"

#include <linux/netlink.h>

#include "hw_net.h"

    
/* Version Information */
#define DRIVER_VERSION "v0.96.0 (2013/05/07)"
#define DRIVER_AUTHOR "Albert Kuo <albertk@realtek.com>"
#define DRIVER_DESC "Realtek RTL8153 Based USB 3.0 Ethernet Adapters"
#define MODULENAME "r8153"

#define PATENTS        "This product is covered by one or more of the " \
    "following patents:\n" \
    "\t\tUS6,570,884, US6,115,776, and US6,327,625.\n"

#define R8153_PHY_ID        32

#define PLA_IDR            0xc000
#define PLA_RCR            0xc010
#define PLA_RMS            0xc016
#define PLA_RXFIFO_CTRL0    0xc0a0
#define PLA_RXFIFO_CTRL1    0xc0a4
#define PLA_RXFIFO_CTRL2    0xc0a8
#define PLA_FMC            0xc0b4
#define PLA_CFG_WOL        0xc0b6
#define PLA_MAR            0xcd00
#define PAL_BDC_CR        0xd1a0
#define PLA_LEDSEL        0xdd90
#define PLA_LED_FEATURE        0xdd92
#define PLA_PHYAR        0xde00
#define PLA_GPHY_INTR_IMR    0xe022
#define PLA_EEE_CR        0xe040
#define PLA_EEEP_CR        0xe080
#define PLA_MAC_PWR_CTRL    0xe0c0
#define PLA_TCR0        0xe610
#define PLA_TCR1        0xe612
#define PLA_TXFIFO_CTRL        0xe618
#define PLA_RSTTELLY        0xe800
#define PLA_CR            0xe813
#define PLA_CRWECR        0xe81c
#define PLA_CONFIG5        0xe822
#define PLA_PHY_PWR        0xe84c
#define PLA_OOB_CTRL        0xe84f
#define PLA_CPCR        0xe854
#define PLA_MISC_0        0xe858
#define PLA_MISC_1        0xe85a
#define PLA_OCP_GPHY_BASE    0xe86c
#define PLA_TELLYCNT        0xe890
#define PLA_SFF_STS_7        0xe8de
#define PLA_PHYSTATUS        0xe908
#define PLA_BP_BA        0xfc26
#define PLA_BP_0        0xfc28
#define PLA_BP_1        0xfc2a
#define PLA_BP_2        0xfc2c
#define PLA_BP_3        0xfc2e
#define PLA_BP_4        0xfc30
#define PLA_BP_5        0xfc32
#define PLA_BP_6        0xfc34
#define PLA_BP_7        0xfc36

#define USB_DEV_STAT        0xb808
#define USB_USB_CTRL        0xd406
#define USB_PHY_CTRL        0xd408
#define USB_TX_AGG        0xd40a
#define USB_RX_BUF_TH        0xd40c
#define USB_USB_TIMER        0xd428
#define USB_PM_CTRL_STATUS    0xd432
#define USB_TX_DMA        0xd434
#define USB_UPS_CTRL        0xd800
#define USB_BP_BA        0xfc26
#define USB_BP_0        0xfc28
#define USB_BP_1        0xfc2a
#define USB_BP_2        0xfc2c
#define USB_BP_3        0xfc2e
#define USB_BP_4        0xfc30
#define USB_BP_5        0xfc32
#define USB_BP_6        0xfc34
#define USB_BP_7        0xfc36

/* OCP Registers */
#define OCP_ALDPS_CONFIG    0xa430
#define OCP_EEE_CONFIG1        0x2080
#define OCP_EEE_CONFIG2        0x2092
#define OCP_EEE_CONFIG3        0x2094
#define OCP_EEE_AR        0xa41a
#define OCP_EEE_DATA        0xa41c

/* PLA_RCR */
#define RCR_AAP            0x00000001
#define RCR_APM            0x00000002
#define RCR_AM            0x00000004
#define RCR_AB            0x00000008
#define RCR_ACPT_ALL        (RCR_AAP | RCR_APM | RCR_AM | RCR_AB)

/* PLA_RXFIFO_CTRL0 */
#define RXFIFO_THR1_NORMAL    0x00080002
#define RXFIFO_THR1_OOB        0x01800003

/* PLA_RXFIFO_CTRL1 */
#define RXFIFO_THR2_FULL    0x00000060
#define RXFIFO_THR2_HIGH    0x00000038
#define RXFIFO_THR2_OOB        0x0000004a

/* PLA_RXFIFO_CTRL2 */
#define RXFIFO_THR3_FULL    0x00000078
#define RXFIFO_THR3_HIGH    0x00000048
#define RXFIFO_THR3_OOB        0x0000005a

/* PLA_TXFIFO_CTRL */
#define TXFIFO_THR_NORMAL    0x01000008

/* PLA_FMC */
#define FMC_FCR_MCU_EN        0x0001

/* PLA_EEEP_CR */
#define EEEP_CR_EEEP_TX        0x0002

/* PLA_TCR0 */
#define TCR0_TX_EMPTY        0x0800
#define TCR0_AUTO_FIFO        0x0080

/* PLA_TCR1 */
#define VERSION_MASK        0x7cf0

/* PLA_CR */
#define CR_RST            0x10
#define CR_RE            0x08
#define CR_TE            0x04

/* PLA_CRWECR */
#define CRWECR_NORAML        0x00
#define CRWECR_CONFIG        0xc0

/* PLA_OOB_CTRL */
#define NOW_IS_OOB        0x80
#define TXFIFO_EMPTY        0x20
#define RXFIFO_EMPTY        0x10
#define LINK_LIST_READY        0x02
#define DIS_MCU_CLROOB        0x01
#define FIFO_EMPTY        (TXFIFO_EMPTY | RXFIFO_EMPTY)

/* PLA_MISC_1 */
#define RXDY_GATED_EN        0x0008

/* PLA_SFF_STS_7 */
#define RE_INIT_LL        0x8000
#define MCU_BORW_EN        0x4000

/* PLA_CPCR */
#define CPCR_RX_VLAN        0x0040

/* PLA_CFG_WOL */
#define MAGIC_EN        0x0001

/* PAL_BDC_CR */
#define ALDPS_PROXY_MODE    0x0001

/* PLA_CONFIG5 */
#define LAN_WAKE_EN        0x0002

/* PLA_LED_FEATURE */
#define LED_MODE_MASK        0x0700

/* PLA_PHY_PWR */
#define TX_10M_IDLE_EN        0x0080
#define PFM_PWM_SWITCH        0x0040

/* PLA_MAC_PWR_CTRL */
#define D3_CLK_GATED_EN        0x00004000
#define MCU_CLK_RATIO        0x07010f07
#define MCU_CLK_RATIO_MASK    0x0f0f0f0f

/* PLA_GPHY_INTR_IMR */
#define GPHY_STS_MSK        0x0001
#define SPEED_DOWN_MSK        0x0002
#define SPDWN_RXDV_MSK        0x0004
#define SPDWN_LINKCHG_MSK    0x0008

/* PLA_PHYAR */
#define PHYAR_FLAG        0x80000000

/* PLA_EEE_CR */
#define EEE_RX_EN        0x0001
#define EEE_TX_EN        0x0002

/* USB_DEV_STAT */
#define STAT_SPEED_MASK        0x0006
#define STAT_SPEED_HIGH        0x0000
#define STAT_SPEED_FULL        0x0001

/* USB_TX_AGG */
#define TX_AGG_MAX_THRESHOLD    0x03

/* USB_RX_BUF_TH */
#define RX_BUF_THR        0x7a120180

/* USB_TX_DMA */
#define TEST_MODE_DISABLE    0x00000001
#define TX_SIZE_ADJUST1        0x00000100

/* USB_UPS_CTRL */
#define POWER_CUT        0x0100

/* USB_PM_CTRL_STATUS */
#define RWSUME_INDICATE        0x0001

/* USB_USB_CTRL */
#define RX_AGG_DISABLE        0x0010

/* OCP_ALDPS_CONFIG */
#define ENPWRSAVE        0x8000
#define ENPDNPS            0x0200
#define LINKENA            0x0100
#define DIS_SDSAVE        0x0010

/* OCP_EEE_CONFIG1 */
#define RG_TXLPI_MSK_HFDUP    0x8000
#define RG_MATCLR_EN        0x4000
#define EEE_10_CAP        0x2000
#define EEE_NWAY_EN        0x1000
#define TX_QUIET_EN        0x0200
#define RX_QUIET_EN        0x0100
#define SDRISETIME        0x0010    /* bit 4 ~ 6 */
#define RG_RXLPI_MSK_HFDUP    0x0008
#define SDFALLTIME        0x0007    /* bit 0 ~ 2 */

/* OCP_EEE_CONFIG2 */
#define RG_LPIHYS_NUM        0x7000    /* bit 12 ~ 15 */
#define RG_DACQUIET_EN        0x0400
#define RG_LDVQUIET_EN        0x0200
#define RG_CKRSEL        0x0020
#define RG_EEEPRG_EN        0x0010

/* OCP_EEE_CONFIG3 */
#define FST_SNR_EYE_R        0x1500    /* bit 7 ~ 15 */
#define RG_LFS_SEL        0x0060    /* bit 6 ~ 5 */
#define MSK_PH            0x0006    /* bit 0 ~ 3 */

/* OCP_EEE_AR */
/* bit[15:14] function */
#define FUN_ADDR        0x0000
#define FUN_DATA        0x4000
/* bit[4:0] device addr */
#define DEVICE_ADDR        0x0007

/* OCP_EEE_DATA */
#define EEE_ADDR        0x003C
#define EEE_DATA        0x0002

enum rtl_register_content
{
    _1000bps    = 0x10,
    _100bps        = 0x08,
    _10bps        = 0x04,
    LINK_STATUS    = 0x02,
    FULL_DUP    = 0x01,
};

#define RTL8153_MAX_RX        20
#define INTBUFSIZE        20

#define RTL8153_REQT_READ    0xc0
#define RTL8153_REQT_WRITE    0x40
#define RTL8153_REQ_GET_REGS    0x05
#define RTL8153_REQ_SET_REGS    0x05

#define BYTE_EN_DWORD        0xff
#define BYTE_EN_WORD        0x33
#define BYTE_EN_BYTE        0x11
#define BYTE_EN_SIX_BYTES    0x3f
#define BYTE_EN_START_MASK    0x0f
#define BYTE_EN_END_MASK    0xf0

#define RTL8153_RMS        (VLAN_ETH_FRAME_LEN + VLAN_HLEN)
#define RTL8153_TX_TIMEOUT    (HZ)

/* rtl8153 flags */
enum rtl8153_flags
{
    RTL8153_UNPLUG = 0,
    RX_URB_FAIL,
    RTL8153_SET_RX_MODE,
    WORK_ENABLE,
    RTL8153_TX_PAUSE
};

/* Define these values to match your device */
#define VENDOR_ID_REALTEK        0x0bda
#define VENDOR_ID_MELCO            0x0411
#define VENDOR_ID_MICRONET        0x3980
#define VENDOR_ID_LONGSHINE        0x07b8
#define VENDOR_ID_OQO            0x1557
#define VENDOR_ID_ZYXEL            0x0586

#define PRODUCT_ID_RTL8152        0x8152
#define PRODUCT_ID_RTL8153        0x8153
#define PRODUCT_ID_LUAKTX        0x0012
#define PRODUCT_ID_LCS8138TX        0x401a
#define PRODUCT_ID_SP128AR        0x0003
#define PRODUCT_ID_PRESTIGE        0x401a

#define MCU_TYPE_PLA            0x0100
#define MCU_TYPE_USB            0x0000

struct rx_desc
{
    u32 opts1;
#define RX_LEN_MASK            0x7fff
    u32 opts2;
    u32 opts3;
    u32 opts4;
    u32 opts5;
    u32 opts6;
};

struct tx_desc
{
    u32 opts1;
#define TX_FS            (1 << 31) /* First segment of a packet */
#define TX_LS            (1 << 30) /* Final segment of a packet */
#define TX_LEN_MASK        0xffff
    u32 opts2;
};

extern struct workqueue_struct* lte_data_process;  //hujuanli add

struct r8153
{
    unsigned long flags;
    unsigned long rx_lock;
    unsigned long tx_lock;
    struct usb_device* udev;
    struct tasklet_struct tl;
    //struct work_struct usbwork;
    struct net_device* netdev;
    struct urb* intr_urb;
    struct urb* tx_urb[RTL8153_MAX_RX];
    struct urb* rx_urb[RTL8153_MAX_RX];
    struct sk_buff* tx_skb[RTL8153_MAX_RX];
    struct delayed_work schedule;
    struct mii_if_info mii;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
    struct net_device_stats stats;
#endif
    int intr_interval;
    u32 msg_enable;
    u16 ocp_base;
    u8 tx_enqueue, tx_dequeue;
    u8 rx_enqueue, rx_dequeue;
    u8* rx_buf[RTL8153_MAX_RX];
    u8* intr_buff;
    u8 version;
    u8 speed;
};

enum rtl_cmd
{
    RTLTOOL_PLA_OCP_READ_DWORD = 0,
    RTLTOOL_PLA_OCP_WRITE_DWORD,
    RTLTOOL_USB_OCP_READ_DWORD,
    RTLTOOL_USB_OCP_WRITE_DWORD,
    RTLTOOL_PLA_OCP_READ,
    RTLTOOL_PLA_OCP_WRITE,
    RTLTOOL_USB_OCP_READ,
    RTLTOOL_USB_OCP_WRITE,
    RTLTOOL_USB_INFO,

    RTLTOOL_INVALID
};

struct usb_device_info
{
    __u16        idVendor;
    __u16        idProduct;
    __u16        bcdDevice;
    __u8        dev_addr[8];
    char        devpath[16];
};

struct rtltool_cmd
{
    __u32    cmd;
    __u32    offset;
    __u32    byteen;
    __u32    data;
    void*    buf;
    struct usb_device_info nic_info;
    struct sockaddr ifru_addr;
    struct sockaddr ifru_netmask;
    struct sockaddr ifru_hwaddr;
};

enum rtl_version
{
    RTL_VER_UNKNOWN = 0,
    RTL_VER_01,
    RTL_VER_02
};

/* Maximum number of multicast addresses to filter (vs. Rx-all-multicast).
 * The RTL chips use a 64 element hash table based on the Ethernet CRC.
 */
static const int multicast_filter_limit = 32;
//static int rx_buf_sz = 32768;
static int rx_buf_sz = 16384;
//static int rx_buf_sz = 1600;


static struct r8153* rtl_tp = NULL;

static int g_usb_ether_bind = 0; //µ±Ç°Íø¿ÚÊÇ·ñopen

static struct usb_device* g_udev = NULL;

static
int get_registers(struct r8153* tp, u16 value, u16 index, u16 size, void* data)
{
    return usb_control_msg(tp->udev, usb_rcvctrlpipe(tp->udev, 0),
                           RTL8153_REQ_GET_REGS, RTL8153_REQT_READ,
                           value, index, data, size, 500);
}

static
int set_registers(struct r8153* tp, u16 value, u16 index, u16 size, void* data)
{
    return usb_control_msg(tp->udev, usb_sndctrlpipe(tp->udev, 0),
                           RTL8153_REQ_SET_REGS, RTL8153_REQT_WRITE,
                           value, index, data, size, 500);
}

static int generic_ocp_read(struct r8153* tp, u16 index, u16 size,
                            void* data, u16 type)
{
    u16    limit = 64;
    int    ret = 0;

    if (test_bit(RTL8153_UNPLUG, &tp->flags))
    { return -ENODEV; }

    /* both size and indix must be 4 bytes align */
    if ((size & 3) || !size || (index & 3) || !data)
    { return -EPERM; }

    if ((u32)index + (u32)size > 0xffff)
    { return -EPERM; }

    while (size)
    {
        if (size > limit)
        {
            ret = get_registers(tp, index, type, limit, data);
            if (ret < 0)
            { break; }

            index += limit;
            data += limit;
            size -= limit;
        }
        else
        {
            ret = get_registers(tp, index, type, size, data);
            if (ret < 0)
            { break; }

            index += size;
            data += size;
            size = 0;
            break;
        }
    }

    return ret;
}

static int generic_ocp_write(struct r8153* tp, u16 index, u16 byteen,
                             u16 size, void* data, u16 type)
{
    int    ret;
    u16    byteen_start, byteen_end, byen;
    u16    limit = 512;

    if (test_bit(RTL8153_UNPLUG, &tp->flags))
    { return -ENODEV; }

    /* both size and indix must be 4 bytes align */
    if ((size & 3) || !size || (index & 3) || !data)
    { return -EPERM; }

    if ((u32)index + (u32)size > 0xffff)
    { return -EPERM; }

    byteen_start = byteen & BYTE_EN_START_MASK;
    byteen_end = byteen & BYTE_EN_END_MASK;

    byen = byteen_start | (byteen_start << 4);
    ret = set_registers(tp, index, type | byen, 4, data);
    if (ret < 0)
    { goto error1; }

    index += 4;
    data += 4;
    size -= 4;

    if (size)
    {
        size -= 4;

        while (size)
        {
            if (size > limit)
            {
                ret = set_registers(tp, index,
                                    type | BYTE_EN_DWORD,
                                    limit, data);
                if (ret < 0)
                { goto error1; }

                index += limit;
                data += limit;
                size -= limit;
            }
            else
            {
                ret = set_registers(tp, index,
                                    type | BYTE_EN_DWORD,
                                    size, data);
                if (ret < 0)
                { goto error1; }

                index += size;
                data += size;
                size = 0;
                break;
            }
        }

        byen = byteen_end | (byteen_end >> 4);
        ret = set_registers(tp, index, type | byen, 4, data);
        if (ret < 0)
        { goto error1; }
    }

error1:
    return ret;
}

static inline
int pla_ocp_read(struct r8153* tp, u16 index, u16 size, void* data)
{
    return generic_ocp_read(tp, index, size, data, MCU_TYPE_PLA);
}

static inline
int pla_ocp_write(struct r8153* tp, u16 index, u16 byteen, u16 size, void* data)
{
    return generic_ocp_write(tp, index, byteen, size, data, MCU_TYPE_PLA);
}

static inline
int usb_ocp_read(struct r8153* tp, u16 index, u16 size, void* data)
{
    return generic_ocp_read(tp, index, size, data, MCU_TYPE_USB);
}

static inline
int usb_ocp_write(struct r8153* tp, u16 index, u16 byteen, u16 size, void* data)
{
    return generic_ocp_write(tp, index, byteen, size, data, MCU_TYPE_USB);
}

static u32 ocp_read_dword(struct r8153* tp, u16 type, u16 index)
{
    u32 data;

    if (type == MCU_TYPE_PLA)
    { pla_ocp_read(tp, index, sizeof(data), &data); }
    else
    { usb_ocp_read(tp, index, sizeof(data), &data); }

    return __le32_to_cpu(data);
}

static void ocp_write_dword(struct r8153* tp, u16 type, u16 index, u32 data)
{
    if (type == MCU_TYPE_PLA)
    { pla_ocp_write(tp, index, BYTE_EN_DWORD, sizeof(data), &data); }
    else
    { usb_ocp_write(tp, index, BYTE_EN_DWORD, sizeof(data), &data); }
}

static u16 ocp_read_word(struct r8153* tp, u16 type, u16 index)
{
    u32 data;
    u8 shift = index & 2;

    index &= ~3;

    if (type == MCU_TYPE_PLA)
    { pla_ocp_read(tp, index, sizeof(data), &data); }
    else
    { usb_ocp_read(tp, index, sizeof(data), &data); }

    data = __le32_to_cpu(data);
    data >>= (shift * 8);
    data &= 0xffff;

    return (u16)data;
}

static void ocp_write_word(struct r8153* tp, u16 type, u16 index, u32 data)
{
    u32 tmp, mask = 0xffff;
    u16 byen = BYTE_EN_WORD;
    u8 shift = index & 2;

    data &= mask;

    if (index & 2)
    {
        byen <<= shift;
        mask <<= (shift * 8);
        data <<= (shift * 8);
        index &= ~3;
    }

    if (type == MCU_TYPE_PLA)
    { pla_ocp_read(tp, index, sizeof(tmp), &tmp); }
    else
    { usb_ocp_read(tp, index, sizeof(tmp), &tmp); }

    tmp = __le32_to_cpu(tmp) & ~mask;
    tmp |= data;
    tmp = __cpu_to_le32(tmp);

    if (type == MCU_TYPE_PLA)
    { pla_ocp_write(tp, index, byen, sizeof(tmp), &tmp); }
    else
    { usb_ocp_write(tp, index, byen, sizeof(tmp), &tmp); }
}

static u8 ocp_read_byte(struct r8153* tp, u16 type, u16 index)
{
    u32 data;
    u8 shift = index & 3;

    index &= ~3;

    if (type == MCU_TYPE_PLA)
    { pla_ocp_read(tp, index, sizeof(data), &data); }
    else
    { usb_ocp_read(tp, index, sizeof(data), &data); }

    data = __le32_to_cpu(data);
    data >>= (shift * 8);
    data &= 0xff;

    return (u8)data;
}

static void ocp_write_byte(struct r8153* tp, u16 type, u16 index, u32 data)
{
    u32 tmp, mask = 0xff;
    u16 byen = BYTE_EN_BYTE;
    u8 shift = index & 3;

    data &= mask;

    if (index & 3)
    {
        byen <<= shift;
        mask <<= (shift * 8);
        data <<= (shift * 8);
        index &= ~3;
    }

    if (type == MCU_TYPE_PLA)
    { pla_ocp_read(tp, index, sizeof(tmp), &tmp); }
    else
    { usb_ocp_read(tp, index, sizeof(tmp), &tmp); }

    tmp = __le32_to_cpu(tmp) & ~mask;
    tmp |= data;
    tmp = __cpu_to_le32(tmp);

    if (type == MCU_TYPE_PLA)
    { pla_ocp_write(tp, index, byen, sizeof(tmp), &tmp); }
    else
    { usb_ocp_write(tp, index, byen, sizeof(tmp), &tmp); }
}

static void r8153_mdio_write(struct r8153* tp, u32 reg_addr, u32 value)
{
    u32    ocp_data;
    int    i;

    ocp_data = PHYAR_FLAG | ((reg_addr & 0x1f) << 16) |
               (value & 0xffff);

    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_PHYAR, ocp_data);

    for (i = 20; i > 0; i--)
    {
        udelay(25);
        ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_PHYAR);
        if (!(ocp_data & PHYAR_FLAG))
        { break; }
    }
    udelay(20);
}

static int r8153_mdio_read(struct r8153* tp, u32 reg_addr)
{
    u32    ocp_data;
    int    i;

    ocp_data = (reg_addr & 0x1f) << 16;
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_PHYAR, ocp_data);

    for (i = 20; i > 0; i--)
    {
        udelay(25);
        ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_PHYAR);
        if (ocp_data & PHYAR_FLAG)
        { break; }
    }
    udelay(20);

    if (!(ocp_data & PHYAR_FLAG))
    { return -EAGAIN; }

    return (u16)(ocp_data & 0xffff);
}

static int read_mii_word(struct net_device* netdev, int phy_id, int reg)
{
    struct r8153* tp = netdev_priv(netdev);

    if (phy_id != R8153_PHY_ID)
    { return -EINVAL; }

    return r8153_mdio_read(tp, reg);
}

static
void write_mii_word(struct net_device* netdev, int phy_id, int reg, int val)
{
    struct r8153* tp = netdev_priv(netdev);

    if (phy_id != R8153_PHY_ID)
    { return; }

    r8153_mdio_write(tp, reg, val);
}

static u16 ocp_reg_read(struct r8153* tp, u16 addr)
{
    u16 ocp_base, ocp_index;

    ocp_base = addr & 0xf000;
    if (ocp_base != tp->ocp_base)
    {
        ocp_write_word(tp, MCU_TYPE_PLA, PLA_OCP_GPHY_BASE, ocp_base);
        tp->ocp_base = ocp_base;
    }

    ocp_index = (addr & 0x0fff) | 0xb000;
    return ocp_read_word(tp, MCU_TYPE_PLA, ocp_index);
}

static void ocp_reg_write(struct r8153* tp, u16 addr, u16 data)
{
    u16 ocp_base, ocp_index;

    ocp_base = addr & 0xf000;
    if (ocp_base != tp->ocp_base)
    {
        ocp_write_word(tp, MCU_TYPE_PLA, PLA_OCP_GPHY_BASE, ocp_base);
        tp->ocp_base = ocp_base;
    }

    ocp_index = (addr & 0x0fff) | 0xb000;
    ocp_write_word(tp, MCU_TYPE_PLA, ocp_index, data);
}

static int r8153_submit_rx(struct r8153* tp, int index, gfp_t mem_flags);

static inline void set_ethernet_addr(struct r8153* tp)
{
    struct net_device* dev = tp->netdev;
    u8 node_id[8] = {0};// = {0x00, 0xe0, 0x4c, 0x68, 0x00, 0x04, 0x00, 0x00};

    if (pla_ocp_read(tp, PLA_IDR, sizeof(node_id), node_id) < 0)
    { netif_notice(tp, probe, dev, "inet addr fail\n"); }
    else
    {
        memcpy(dev->dev_addr, node_id, sizeof(node_id));
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,13)
        memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);
#endif
    }
}

static int rtl8153_set_mac_address(struct net_device* netdev, void* p)
{
    struct r8153* tp = netdev_priv(netdev);
    struct sockaddr* addr = p;

    if (!is_valid_ether_addr(addr->sa_data))
    { return -EADDRNOTAVAIL; }

    memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);

    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_CONFIG);
    pla_ocp_write(tp, PLA_IDR, BYTE_EN_SIX_BYTES, 8, addr->sa_data);
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_NORAML);

    return 0;
}

static inline void* agg_align(void* data)
{
    return (void*)ALIGN((uintptr_t)data, 8);
}

static void free_all_urbs(struct r8153* tp)
{
    int i;

    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        if (tp->rx_urb[i])
        {
            usb_free_urb(tp->rx_urb[i]);
            tp->rx_urb[i] = NULL;
        }

        if (tp->tx_urb[i])
        {
            usb_free_urb(tp->tx_urb[i]);
            tp->tx_urb[i] = NULL;
        }
    }

    if (tp->intr_urb)
    {
        usb_free_urb(tp->intr_urb);
        tp->intr_urb = NULL;
    }
}

static int alloc_all_urbs(struct r8153* tp)
{
    int i;

    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        tp->rx_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
        if (!tp->rx_urb[i])
        {
            free_all_urbs(tp);
            return 0;
        }

        tp->tx_urb[i] = usb_alloc_urb(0, GFP_KERNEL);
        if (!tp->tx_urb[i])
        {
            free_all_urbs(tp);
            return 0;
        }
    }

    tp->intr_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!tp->intr_urb)
    {
        free_all_urbs(tp);
        return 0;
    }

    return 1;
}

static struct net_device_stats* rtl8153_get_stats(struct net_device* dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
    struct rtl8153* tp = netdev_priv(dev);

    return (struct net_device_stats*)&tp->stats;
#else
    return &dev->stats;
#endif
}

static void read_bulk_callback(struct urb* urb)
{
    struct r8153* tp;
    struct net_device* netdev;
    int status = urb->status;
    u8 rx_enqueue;
    int result, i;

    tp = urb->context;
    if (!tp)
    { return; }

    if (test_bit(RTL8153_UNPLUG, &tp->flags))
    { return; }

    netdev = tp->netdev;
    if (!netif_device_present(netdev))
    { return; }

    rx_enqueue = tp->rx_enqueue;
    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        if (agg_align(tp->rx_buf[rx_enqueue]) == urb->transfer_buffer)
        { break; }
        rx_enqueue = (rx_enqueue + 1) % RTL8153_MAX_RX;
    }
    tp->rx_enqueue = (rx_enqueue + 1) % RTL8153_MAX_RX;

    switch (status)
    {
        case 0:
            if (urb->actual_length < ETH_ZLEN)
            {
                urb->actual_length = 0;
                break;
            }
            set_bit(rx_enqueue, &tp->rx_lock);
            tasklet_schedule(&tp->tl);
            //queue_work(lte_data_process,&tp->usbwork);//hujuanli add
            return;
        case -ESHUTDOWN:
            set_bit(RTL8153_UNPLUG, &tp->flags);
            netif_device_detach(tp->netdev);
            return;
        case -ENOENT:
            return;    /* the urb is in unlink state */
        case -ETIME:
            if (printk_ratelimit())
                netif_warn(tp, rx_err, netdev,
                           "may be reset is needed?..\n");
            break;
        default:
            if (printk_ratelimit())
                netif_warn(tp, rx_err, netdev,
                           "Rx status %d\n", status);
            break;
    }

    result = r8153_submit_rx(tp, rx_enqueue, GFP_ATOMIC);
    if (result == -ENODEV)
    {
        netif_device_detach(tp->netdev);
    }
    else if (result)
    {
        urb->actual_length = 0;
        set_bit(RX_URB_FAIL, &tp->flags);
        set_bit(rx_enqueue, &tp->rx_lock);
        tasklet_schedule(&tp->tl);
        //queue_work(lte_data_process,&tp->usbwork);//hujuanli add
    }
    else
    {
        clear_bit(RX_URB_FAIL, &tp->flags);
    }
}

static void rx_bottom(unsigned long data)
//static void rx_bottom()
{
    struct net_device_stats* stats;
    struct net_device* netdev;
    struct rx_desc* rx_desc;
    struct sk_buff* skb;
    struct urb* urb;
    struct r8153* tp;
    unsigned pkt_len;
    int len_used;
    u8* rx_data;
    u8 rx_dequeue;
    int i, ret;

    tp = (struct r8153*)data;

    //tp = rtl_tp;
    netdev = tp->netdev;

    if (!netif_running(netdev))
    { return; }

    stats = rtl8153_get_stats(netdev);

    rx_dequeue = tp->rx_dequeue;
    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        if (test_bit(rx_dequeue, &tp->rx_lock))
        { break; }

        rx_dequeue = (rx_dequeue + 1) % RTL8153_MAX_RX;
    }

    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        if (!test_bit(rx_dequeue, &tp->rx_lock))
        { break; }

        urb = tp->rx_urb[rx_dequeue];
        if (urb->actual_length < ETH_ZLEN)
        {
            clear_bit(rx_dequeue, &tp->rx_lock);
            ret = r8153_submit_rx(tp, rx_dequeue, GFP_ATOMIC);
            if (ret && ret != -ENODEV)
            {
                set_bit(rx_dequeue, &tp->rx_lock);
                tp->rx_dequeue = rx_dequeue;
                tasklet_schedule(&tp->tl);
                //queue_work(lte_data_process,&tp->usbwork);//hujuanli add

                break;
            }
            else
            {
                rx_dequeue = (rx_dequeue + 1) % RTL8153_MAX_RX;
                tp->rx_dequeue = rx_dequeue;
                continue;
            }
        }

        len_used = 0;
        rx_desc = agg_align(tp->rx_buf[rx_dequeue]);
        rx_data = agg_align(tp->rx_buf[rx_dequeue]);
        smp_wmb();
        pkt_len = le32_to_cpu(rx_desc->opts1) & RX_LEN_MASK;
        len_used += sizeof(struct rx_desc) + pkt_len;

        while (urb->actual_length >= len_used)
        {
            if (pkt_len < ETH_ZLEN)
            { break; }

            pkt_len -= 4; /* CRC */
            rx_data += sizeof(struct rx_desc);


            skb = netdev_alloc_skb_ip_align(netdev, pkt_len);
            if (!skb)
            {
                stats->rx_dropped++;
                break;
            }
            memcpy(skb->data, rx_data, pkt_len);

            mbb_usbnet_rx_set_mac_clone(skb);


            skb_put(skb, pkt_len);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
            skb->dev = netdev;
#endif

#if (FEATURE_ON == MBB_CTF_COMMON)
            if (BCME_OK == mbb_usbnet_ctf_forward(skb))
            {
                goto next_step;
            }
#endif

            skb->protocol = eth_type_trans(skb, netdev);
            netif_rx(skb);

        next_step:

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
            netdev->last_rx = jiffies;
#endif
            stats->rx_packets++;
            stats->rx_bytes += pkt_len;

            rx_data = agg_align(rx_data + pkt_len + 4);
            rx_desc = (struct rx_desc*)rx_data;
            smp_wmb();
            pkt_len = le32_to_cpu(rx_desc->opts1) & RX_LEN_MASK;
            len_used += sizeof(struct rx_desc) + pkt_len;
        }

        clear_bit(rx_dequeue, &tp->rx_lock);
        ret = r8153_submit_rx(tp, rx_dequeue, GFP_ATOMIC);
        if (ret && ret != -ENODEV)
        {
            urb->actual_length = 0;
            set_bit(rx_dequeue, &tp->rx_lock);
            tp->rx_dequeue = rx_dequeue;
            tasklet_schedule(&tp->tl);
            //queue_work(lte_data_process,&tp->usbwork);//hujuanli add
            break;
        }
        else
        {
            rx_dequeue = (rx_dequeue + 1) % RTL8153_MAX_RX;
            tp->rx_dequeue = rx_dequeue;
        }
    }
}

static void write_bulk_callback(struct urb* urb)
{
    struct net_device_stats* stats;
    struct r8153* tp;
    int status = urb->status;

    tp = urb->context;
    if (!tp)
    { return; }

    stats = rtl8153_get_stats(tp->netdev);
    if (status)
    {
        dev_info(&urb->dev->dev, "%s: Tx status %d\n",
                 tp->netdev->name, status);
        stats->tx_errors++;
    }
    else
    {
        stats->tx_packets++;
        stats->tx_bytes += tp->tx_skb[tp->tx_dequeue]->len;
    }
    dev_kfree_skb_irq(tp->tx_skb[tp->tx_dequeue]);
    clear_bit(tp->tx_dequeue, &tp->tx_lock);

    tp->tx_dequeue = (tp->tx_dequeue + 1) % RTL8153_MAX_RX;
    if (test_bit(RTL8153_TX_PAUSE, &tp->flags) && !test_bit(tp->tx_enqueue, &tp->tx_lock))
    {
        netif_wake_queue(tp->netdev);
        clear_bit(RTL8153_TX_PAUSE, &tp->flags);
    }

    //    if (!netif_device_present(tp->netdev))
    //        return;
}

#if 0
static void intr_callback(struct urb* urb)
{
    struct rtl8153* tp;
    struct net_device_stats* stats;
    __u8* d;
    int status = urb->status;
    int res;

    tp = urb->context;
    if (!tp)
    { return; }
    switch (status)
    {
        case 0:            /* success */
            break;
        case -ECONNRESET:    /* unlink */
        case -ESHUTDOWN:
            netif_device_detach(tp->netdev);
        case -ENOENT:
            return;
        case -EOVERFLOW:
            dev_info(&urb->dev->dev, "%s: intr status -EOVERFLOW\n",
                     tp->netdev->name);
            goto resubmit;
            /* -EPIPE:  should clear the halt */
        default:
            dev_info(&urb->dev->dev, "%s: intr status %d\n",
                     tp->netdev->name, status);
            goto resubmit;
    }

    stats = rtl8153_get_stats(tp->netdev);
    d = urb->transfer_buffer;
    if (d[0] & TSR_ERRORS)
    {
        stats->tx_errors++;
        if (d[INT_TSR] & (TSR_ECOL | TSR_JBR))
        { stats->tx_aborted_errors++; }
        if (d[INT_TSR] & TSR_LCOL)
        { stats->tx_window_errors++; }
        if (d[INT_TSR] & TSR_LOSS_CRS)
        { stats->tx_carrier_errors++; }
    }
    /* Report link status changes to the network stack */
    if ((d[INT_MSR] & MSR_LINK) == 0)
    {
        if (netif_carrier_ok(tp->netdev))
        {
            netif_carrier_off(tp->netdev);
            dbg("%s: LINK LOST\n", __func__);
        }
    }
    else
    {
        if (!netif_carrier_ok(tp->netdev))
        {
            netif_carrier_on(tp->netdev);
            dbg("%s: LINK CAME BACK\n", __func__);
        }
    }

resubmit:
    res = usb_submit_urb (urb, GFP_ATOMIC);
    if (res == -ENODEV)
    {
        netif_device_detach(tp->netdev);
    }
    else if (res)
        printk ("can't resubmit intr, %s-%s/input0, status %d",
                tp->udev->bus->bus_name,
                tp->udev->devpath, res);
}
#endif

/*
**
**    network related part of the code
**
*/

static int r8153_submit_rx(struct r8153* tp, int index, gfp_t mem_flags)
{
    usb_fill_bulk_urb(tp->rx_urb[index], tp->udev, usb_rcvbulkpipe(tp->udev, 1),
                      agg_align(tp->rx_buf[index]), rx_buf_sz,
                      (usb_complete_t)read_bulk_callback, tp);

    return usb_submit_urb(tp->rx_urb[index], mem_flags);
}

static void rtl8153_tx_timeout(struct net_device* netdev)
{
    struct r8153* tp = netdev_priv(netdev);
    struct net_device_stats* stats = rtl8153_get_stats(netdev);
    int i;

    netif_warn(tp, tx_err, netdev, "Tx timeout.\n");
    for (i = 0; i < RTL8153_MAX_RX; i++)
    { usb_unlink_urb(tp->tx_urb[i]); }
    stats->tx_errors++;
}

static void rtl8153_set_rx_mode(struct net_device* netdev)
{
    struct r8153* tp = netdev_priv(netdev);
    u32 tmp, mc_filter[2];    /* Multicast hash filter */
    u32 ocp_data;

    if (in_atomic())
    {
        if (tp->speed & LINK_STATUS)
        { set_bit(RTL8153_SET_RX_MODE, &tp->flags); }
        return;
    }

    clear_bit(RTL8153_SET_RX_MODE, &tp->flags);
    netif_stop_queue(netdev);
    ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
    ocp_data &= ~RCR_ACPT_ALL;
    ocp_data |= RCR_AB | RCR_APM;

    if (netdev->flags & IFF_PROMISC)
    {
        /* Unconditionally log net taps. */
        netif_notice(tp, link, netdev, "Promiscuous mode enabled\n");
        ocp_data |= RCR_AM | RCR_AAP;
        mc_filter[1] = mc_filter[0] = 0xffffffff;
    }
    else if ((netdev_mc_count(netdev) > multicast_filter_limit) ||
             (netdev->flags & IFF_ALLMULTI))
    {
        /* Too many to filter perfectly -- accept all multicasts. */
        ocp_data |= RCR_AM;
        mc_filter[1] = mc_filter[0] = 0xffffffff;
    }
    else
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
        struct dev_mc_list* mclist;
        unsigned int i;

        mc_filter[1] = mc_filter[0] = 0;
        for (i = 0, mclist = netdev->mc_list; mclist && i < netdev->mc_count;
             i++, mclist = mclist->next)
        {
            int bit_nr;
            bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;
            mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
            ocp_data |= RCR_AM;
        }
#else
        struct netdev_hw_addr* ha;

        mc_filter[1] = mc_filter[0] = 0;
        netdev_for_each_mc_addr(ha, netdev)
        {
            int bit_nr = ether_crc(ETH_ALEN, ha->addr) >> 26;
            mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
            ocp_data |= RCR_AM;
        }
#endif
    }

    tmp = mc_filter[0];
    mc_filter[0] = __cpu_to_le32(swab32(mc_filter[1]));
    mc_filter[1] = __cpu_to_le32(swab32(tmp));

    pla_ocp_write(tp, PLA_MAR, BYTE_EN_DWORD, sizeof(mc_filter), mc_filter);
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);
    netif_wake_queue(netdev);
}

static netdev_tx_t rtl8153_start_xmit(struct sk_buff* skb,
                                      struct net_device* netdev)
{
    struct r8153* tp = netdev_priv(netdev);
    struct net_device_stats* stats = rtl8153_get_stats(netdev);
    struct tx_desc* tx_desc;
    u8 tx_enqueue;
    int len, res;


    mbb_usbnet_tx_set_mac_clone(skb);

    len = skb->len;
    if (skb_header_cloned(skb) || skb_headroom(skb) < sizeof(*tx_desc))
    {
        struct sk_buff* tx_skb;

        tx_skb = skb_copy_expand(skb, sizeof(*tx_desc), 0, GFP_ATOMIC);
        dev_kfree_skb_any(skb);
        if (!tx_skb)
        {
            stats->tx_dropped++;
            //            netif_wake_queue(netdev);
            return NETDEV_TX_OK;
        }
        skb = tx_skb;
    }
    tx_desc = (struct tx_desc*)skb_push(skb, sizeof(*tx_desc));
    memset(tx_desc, 0, sizeof(*tx_desc));
    tx_desc->opts1 = cpu_to_le32((len & TX_LEN_MASK) | TX_FS | TX_LS);
    tx_enqueue = tp->tx_enqueue;
    set_bit(tx_enqueue, &tp->tx_lock);
    tp->tx_skb[tx_enqueue] = skb;
    skb_tx_timestamp(skb);
    usb_fill_bulk_urb(tp->tx_urb[tx_enqueue], tp->udev, usb_sndbulkpipe(tp->udev, 2),
                      skb->data, skb->len,
                      (usb_complete_t)write_bulk_callback, tp);
    res = usb_submit_urb(tp->tx_urb[tx_enqueue], GFP_ATOMIC);
    if (res)
    {
        /* Can we get/handle EPIPE here? */
        if (res == -ENODEV)
        {
            netif_device_detach(tp->netdev);
        }
        else
        {
            netif_warn(tp, tx_err, netdev,
                       "failed tx_urb %d\n", res);
            stats->tx_dropped++;
            clear_bit(tx_enqueue, &tp->tx_lock);
            tp->tx_skb[tx_enqueue] = NULL;
            dev_kfree_skb_any(skb);
            return NETDEV_TX_OK;
        }
    }

    tx_enqueue = (tx_enqueue + 1) % RTL8153_MAX_RX;
    if (test_bit(tx_enqueue, &tp->tx_lock))
    {
        netif_stop_queue(netdev);
        set_bit(RTL8153_TX_PAUSE, &tp->flags);
    }

    tp->tx_enqueue = tx_enqueue;

    return NETDEV_TX_OK;
}

static void r8153_reset_packet_filter(struct r8153* tp)
{
    u32    ocp_data;

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_FMC);
    ocp_data &= ~FMC_FCR_MCU_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_FMC, ocp_data);
    ocp_data |= FMC_FCR_MCU_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_FMC, ocp_data);
}

static void rtl8153_nic_reset(struct r8153* tp)
{
    int    i;

    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CR, CR_RST);

    for (i = 0; i < 1000; i++)
    {
        if (!(ocp_read_byte(tp, MCU_TYPE_PLA, PLA_CR) & CR_RST))
        { break; }
        udelay(100);
    }
}

static inline u8 rtl8153_get_speed(struct r8153* tp)
{
    return ocp_read_byte(tp, MCU_TYPE_PLA, PLA_PHYSTATUS);
}

static int rtl8153_enable(struct r8153* tp)
{
    u32 ocp_data;
    int i, ret;
    u8 speed;

    speed = rtl8153_get_speed(tp);
    if (speed & _10bps)
    {
        ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR);
        ocp_data |= EEEP_CR_EEEP_TX;
        ocp_write_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR, ocp_data);
    }
    else
    {
        ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR);
        ocp_data &= ~EEEP_CR_EEEP_TX;
        ocp_write_word(tp, MCU_TYPE_PLA, PLA_EEEP_CR, ocp_data);
    }
    if (speed & _10bps)
    {
        mbb_usbnet_set_speed(CRADLE_SPEED_10M);
    }
    else if(speed & _100bps)
    {
        mbb_usbnet_set_speed(CRADLE_SPEED_100M);
    }
    else
    {
        mbb_usbnet_set_speed(CRADLE_SPEED_1000M);
    }

    r8153_reset_packet_filter(tp);

    ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_CR);
    ocp_data |= CR_RE | CR_TE;
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CR, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_MISC_1);
    ocp_data &= ~RXDY_GATED_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_MISC_1, ocp_data);

    tp->rx_lock = tp->tx_lock = 0;
    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        ret = r8153_submit_rx(tp, i, GFP_KERNEL);
        if (ret)
        { break; }
    }

    return ret;
}

static void rtl8153_disable(struct r8153* tp)
{
    u32    ocp_data;
    int    i;

    mbb_usbnet_set_speed(CRADLE_SPEED_UNKOWN);

    usb_kill_urb(tp->intr_urb);

    ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
    ocp_data &= ~RCR_ACPT_ALL;
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);

    for (i = 0; i < RTL8153_MAX_RX; i++)
    { usb_kill_urb(tp->tx_urb[i]); }

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_MISC_1);
    ocp_data |= RXDY_GATED_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_MISC_1, ocp_data);

    for (i = 0; i < 1000; i++)
    {
        ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
        if ((ocp_data & FIFO_EMPTY) == FIFO_EMPTY)
        { break; }
        mdelay(1);
    }

    for (i = 0; i < 1000; i++)
    {
        if (ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR0) & TCR0_TX_EMPTY)
        { break; }
        mdelay(1);
    }

    for (i = 0; i < RTL8153_MAX_RX; i++)
    { usb_kill_urb(tp->rx_urb[i]); }

    rtl8153_nic_reset(tp);
}

static void r8153_exit_oob(struct r8153* tp)
{
    u32    ocp_data;
    int    i;

    ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
    ocp_data &= ~RCR_ACPT_ALL;
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_MISC_1);
    ocp_data |= RXDY_GATED_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_MISC_1, ocp_data);

    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CRWECR, CRWECR_NORAML);
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CR, 0x00);

    ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
    ocp_data &= ~NOW_IS_OOB;
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
    ocp_data &= ~MCU_BORW_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

    for (i = 0; i < 1000; i++)
    {
        ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
        if (ocp_data & LINK_LIST_READY)
        { break; }
        mdelay(1);
    }

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
    ocp_data |= RE_INIT_LL;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

    for (i = 0; i < 1000; i++)
    {
        ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
        if (ocp_data & LINK_LIST_READY)
        { break; }
        mdelay(1);
    }

    rtl8153_nic_reset(tp);

    /* rx share fifo credit full threshold */
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL0, RXFIFO_THR1_NORMAL);
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1, RXFIFO_THR2_FULL);
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2, 0x0170);
    /* TX share fifo free credit full threshold */
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_TXFIFO_CTRL, TXFIFO_THR_NORMAL);

#if 0
    ocp_write_dword(tp, MCU_TYPE_USB, 0xd42c, 0x1e837a12);
    ocp_write_dword(tp, MCU_TYPE_USB, 0xd40c, 0x7a120180);
#else
    ocp_write_dword(tp, MCU_TYPE_USB, 0xd42c, 0x0f007a12);
    ocp_write_dword(tp, MCU_TYPE_USB, 0xd40c, 0x7a120180);
#endif

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_CPCR);
    ocp_data &= ~CPCR_RX_VLAN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_CPCR, ocp_data);

    ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, RTL8153_RMS);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR0);
    ocp_data |= TCR0_AUTO_FIFO;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_TCR0, ocp_data);
}

static void r8153_enter_oob(struct r8153* tp)
{
    u32    ocp_data;
    int    i;

    ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
    ocp_data &= ~NOW_IS_OOB;
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

    //    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL0, RXFIFO_THR1_OOB);
    //    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL1, RXFIFO_THR2_OOB);
    //    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RXFIFO_CTRL2, RXFIFO_THR3_OOB);

    rtl8153_disable(tp);

    for (i = 0; i < 1000; i++)
    {
        ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
        if (ocp_data & LINK_LIST_READY)
        { break; }
        mdelay(1);
    }

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7);
    ocp_data |= RE_INIT_LL;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_SFF_STS_7, ocp_data);

    for (i = 0; i < 1000; i++)
    {
        ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
        if (ocp_data & LINK_LIST_READY)
        { break; }
        mdelay(1);
    }

    ocp_write_word(tp, MCU_TYPE_PLA, PLA_RMS, RTL8153_RMS);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_CFG_WOL);
    ocp_data |= MAGIC_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_CFG_WOL, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_CPCR);
    ocp_data |= CPCR_RX_VLAN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_CPCR, ocp_data);

    ocp_data = ocp_read_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL);
    ocp_data |= NOW_IS_OOB;
    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_OOB_CTRL, ocp_data);

    ocp_write_byte(tp, MCU_TYPE_PLA, PLA_CONFIG5, LAN_WAKE_EN);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_MISC_1);
    ocp_data &= ~RXDY_GATED_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_MISC_1, ocp_data);

    ocp_data = ocp_read_dword(tp, MCU_TYPE_PLA, PLA_RCR);
    ocp_data |= RCR_APM | RCR_AM | RCR_AB;
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_RCR, ocp_data);
}

static void r8153_disable_aldps(struct r8153* tp)
{
    u16 data;

    data = ocp_reg_read(tp, OCP_ALDPS_CONFIG);
    data &= ~0x0004;
    ocp_reg_write(tp, OCP_ALDPS_CONFIG, data);
    msleep(20);
}

static void r8153_enable_aldps(struct r8153* tp)
{
    u16 data;

    data = ocp_reg_read(tp, OCP_ALDPS_CONFIG);
    data |= 0x0004;
    ocp_reg_write(tp, OCP_ALDPS_CONFIG, data);
}

static int rtl8153_set_speed(struct r8153* tp, u8 autoneg, u16 speed, u8 duplex)
{
    u16 bmcr, anar, gbcr;
    int ret = 0;

    cancel_delayed_work_sync(&tp->schedule);
    r8153_mdio_write(tp, 0x1f, 0x0000);
    anar = r8153_mdio_read(tp, MII_ADVERTISE);
    anar &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
              ADVERTISE_100HALF | ADVERTISE_100FULL);
    gbcr = r8153_mdio_read(tp, MII_CTRL1000);
    gbcr &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

    if (autoneg == AUTONEG_DISABLE)
    {
        if (speed == SPEED_10)
        {
            bmcr = 0;
            anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
        }
        else if (speed == SPEED_100)
        {
            bmcr = BMCR_SPEED100;
            anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
        }
        else if (speed == SPEED_1000)
        {
            bmcr = BMCR_SPEED1000;
            gbcr |= ADVERTISE_1000FULL | ADVERTISE_1000HALF;
        }
        else
        {
            ret = -EINVAL;
            goto out;
        }

        if (duplex == DUPLEX_FULL)
        { bmcr |= BMCR_FULLDPLX; }
    }
    else
    {
        if (speed == SPEED_10)
        {
            if (duplex == DUPLEX_FULL)
            { anar |= ADVERTISE_10HALF | ADVERTISE_10FULL; }
            else
            { anar |= ADVERTISE_10HALF; }
        }
        else if (speed == SPEED_100)
        {
            if (duplex == DUPLEX_FULL)
            {
                anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
                anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
            }
            else
            {
                anar |= ADVERTISE_10HALF;
                anar |= ADVERTISE_100HALF;
            }
        }
        else if (speed == SPEED_1000)
        {
            if (duplex == DUPLEX_FULL)
            {
                anar |= ADVERTISE_10HALF | ADVERTISE_10FULL;
                anar |= ADVERTISE_100HALF | ADVERTISE_100FULL;
                gbcr |= ADVERTISE_1000FULL | ADVERTISE_1000HALF;
            }
            else
            {
                anar |= ADVERTISE_10HALF;
                anar |= ADVERTISE_100HALF;
                gbcr |= ADVERTISE_1000HALF;
            }
        }
        else
        {
            ret = -EINVAL;
            goto out;
        }

        bmcr = BMCR_ANENABLE | BMCR_ANRESTART;
    }

    r8153_mdio_write(tp, MII_CTRL1000, gbcr);
    r8153_mdio_write(tp, MII_ADVERTISE, anar);
    r8153_mdio_write(tp, MII_BMCR, bmcr);

out:
    schedule_delayed_work(&tp->schedule, 5 * HZ);

    return ret;
}

static void rtl8153_down(struct r8153* tp)
{
    u32    ocp_data;

    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, 0xd80a);
    ocp_data &= ~0x0001;
    ocp_write_word(tp, MCU_TYPE_USB, 0xd80a, ocp_data);

    r8153_disable_aldps(tp);
    r8153_enter_oob(tp);
    r8153_enable_aldps(tp);
}

static void set_carrier(struct r8153* tp)
{
    struct net_device* netdev = tp->netdev;
    u8 speed;

    speed = rtl8153_get_speed(tp);

    if (speed & LINK_STATUS)
    {
        printk(KERN_INFO"CRADLE_INSERT\n");
        mbb_usbnet_net_state_notify(CRADLE_INSERT);
        if (!(tp->speed & LINK_STATUS))
        {
            rtl8153_enable(tp);
            set_bit(RTL8153_SET_RX_MODE, &tp->flags);
            netif_carrier_on(netdev);
        }
    }
    else
    {
        printk(KERN_INFO"CRADLE_REMOVE\n");
        mbb_usbnet_net_state_notify(CRADLE_REMOVE);

        if (tp->speed & LINK_STATUS)
        {
            netif_carrier_off(netdev);
            rtl8153_disable(tp);

        }
    }
    tp->speed = speed;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)

void rtl_work_func_t(void* data)
{
    struct r8153* tp = (struct r8153*)data;

    if (!test_bit(WORK_ENABLE, &tp->flags))
    { goto out1; }

    if (test_bit(RTL8153_UNPLUG, &tp->flags))
    { goto out1; }

    set_carrier(tp);

    if (test_bit(RTL8153_SET_RX_MODE, &tp->flags))
    { rtl8153_set_rx_mode(tp->netdev); }

    schedule_delayed_work(&tp->schedule, HZ);

out1:
    return;
}

#else

static void rtl_work_func_t(struct work_struct* work)
{
    struct r8153* tp = container_of(work, struct r8153, schedule.work);

    if (!test_bit(WORK_ENABLE, &tp->flags))
    { goto out1; }

    if (test_bit(RTL8153_UNPLUG, &tp->flags))
    { goto out1; }

    set_carrier(tp);

    if (test_bit(RTL8153_SET_RX_MODE, &tp->flags))
    { rtl8153_set_rx_mode(tp->netdev); }

    schedule_delayed_work(&tp->schedule, HZ);

out1:
    return;
}

#endif

static int rtl8153_open(struct net_device* netdev)
{
    struct r8153* tp = netdev_priv(netdev);
    int res = 0;

    tp->speed = rtl8153_get_speed(tp);
    if (tp->speed & LINK_STATUS)
    {
        res = rtl8153_enable(tp);
        if (res)
        {
            if (res == -ENODEV)
            { netif_device_detach(tp->netdev); }

            netif_err(tp, ifup, netdev,
                      "rtl8152_open failed: %d\n", res);
            return res;
        }

        netif_carrier_on(netdev);
    }
    else
    {
        netif_stop_queue(netdev);
        netif_carrier_off(netdev);
    }

    rtl8153_set_speed(tp, AUTONEG_ENABLE, SPEED_1000, DUPLEX_FULL);

#if (FEATURE_ON == MBB_CTF_COMMON)
    mbb_usbnet_ctf_enable(netdev);
#endif
    netif_start_queue(netdev);
    set_bit(WORK_ENABLE, &tp->flags);
    schedule_delayed_work(&tp->schedule, 0);
    g_usb_ether_bind = 1;
    g_udev = tp->udev;

    return res;
}

static int rtl8153_close(struct net_device* netdev)
{
    struct r8153* tp = netdev_priv(netdev);
    int res = 0;

#if (FEATURE_ON == MBB_CTF_COMMON)
    mbb_usbnet_ctf_disable(netdev);

#endif /* MBB_CTF_COMMON */

    clear_bit(WORK_ENABLE, &tp->flags);
    cancel_delayed_work_sync(&tp->schedule);
    netif_stop_queue(netdev);
    rtl8153_disable(tp);

    g_usb_ether_bind = 0;
    g_udev = NULL;

    return res;
}

static void rtl_clear_bp(struct r8153* tp)
{
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_BP_0, 0);
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_BP_2, 0);
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_BP_4, 0);
    ocp_write_dword(tp, MCU_TYPE_PLA, PLA_BP_6, 0);
    ocp_write_dword(tp, MCU_TYPE_USB, USB_BP_0, 0);
    ocp_write_dword(tp, MCU_TYPE_USB, USB_BP_2, 0);
    ocp_write_dword(tp, MCU_TYPE_USB, USB_BP_4, 0);
    ocp_write_dword(tp, MCU_TYPE_USB, USB_BP_6, 0);
    mdelay(3);
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_BP_BA, 0);
    ocp_write_word(tp, MCU_TYPE_USB, USB_BP_BA, 0);
}

static void r8153_enable_eee(struct r8153* tp)
{
    u32 ocp_data;
    u16 data;

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_EEE_CR);
    ocp_data |= EEE_RX_EN | EEE_TX_EN;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_EEE_CR, ocp_data);
    data = ocp_reg_read(tp, 0xa432);
    data |= 0x0010;
    ocp_reg_write(tp, 0xa432, data);
    data = ocp_reg_read(tp, 0xa5d0);
    data |= 0x0006;
    ocp_reg_write(tp, 0xa5d0, data);
}

static void r8153_enable_fc(struct r8153* tp)
{
    u16 anar;

    r8153_mdio_write(tp, 0x1f, 0x0000);
    anar = r8153_mdio_read(tp, MII_ADVERTISE);
    anar |= ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM;
    r8153_mdio_write(tp, MII_ADVERTISE, anar);
}

static void r8153_firmware(struct r8153* tp)
{
    tp->ocp_base = 0xa000;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_OCP_GPHY_BASE, tp->ocp_base);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xb820);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0290);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa012);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0000);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa014);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c04);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c18);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c45);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c45);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd502);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8301);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8306);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8208);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd501);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xe018);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0308);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x60f2);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8404);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x607d);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc117);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c16);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc116);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c16);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x607d);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc117);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa404);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0800);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd501);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x62d2);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x615d);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc115);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa404);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc307);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd502);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8301);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8306);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8208);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c42);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc114);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8404);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc317);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd701);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x435d);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa208);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd502);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa306);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa301);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c42);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8404);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x613d);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc115);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc307);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd502);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8301);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8306);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x8208);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x2c42);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc114);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xc317);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd701);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x40dd);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa208);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd502);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa306);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xa301);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd500);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0xd702);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0800);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa01a);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0000);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa006);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0fff);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa004);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0fff);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa002);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x05a3);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xa000);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x3591);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb436, 0xb820);
    ocp_write_word(tp, MCU_TYPE_PLA, 0xb438, 0x0210);
}

static void r8153_hw_phy_cfg(struct r8153* tp)
{
    u32 ocp_data;
    u16 data;

    r8153_mdio_write(tp, 0x1f, 0x0000);
    r8153_mdio_write(tp, MII_BMCR, BMCR_ANENABLE);
    r8153_disable_aldps(tp);

    if (tp->version == RTL_VER_01)
    {
        r8153_firmware(tp);

        data = ocp_reg_read(tp, 0xa432);
        data &= ~0x0040;
        ocp_reg_write(tp, 0xa432, data);
    }

    data = ocp_reg_read(tp, OCP_ALDPS_CONFIG);
    data |= 0x8000;
    ocp_reg_write(tp, OCP_ALDPS_CONFIG, data);

    data = ocp_reg_read(tp, 0xa442);
    data |= 0x0080;
    ocp_reg_write(tp, 0xa442, data);
    data = ocp_reg_read(tp, 0xa430);
    data |= 0x0001;
    ocp_reg_write(tp, 0xa430, data);
    ocp_reg_write(tp, 0xa436, 0x8084);
    data = ocp_reg_read(tp, 0xa438);
    data &= ~0x6000;
    ocp_reg_write(tp, 0xa438, data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR);
    ocp_data |= PFM_PWM_SWITCH;
    ocp_write_word(tp, MCU_TYPE_PLA, PLA_PHY_PWR, ocp_data);

    ocp_reg_write(tp, 0xa436, 0x8021);
    data = ocp_reg_read(tp, 0xa438);
    data |= 0x8000;
    ocp_reg_write(tp, 0xa438, data);
}

static void r8153_init(struct r8153* tp)
{
    u32 ocp_data;
    int i;

    rtl_clear_bp(tp);

    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, 0xd80a);
    ocp_data &= ~0x0001;
    ocp_write_word(tp, MCU_TYPE_USB, 0xd80a, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, 0xd81a);
    ocp_data &= ~RWSUME_INDICATE;
    ocp_write_word(tp, MCU_TYPE_USB, 0xd81a, ocp_data);

    r8153_hw_phy_cfg(tp);

    r8153_exit_oob(tp);

    r8153_enable_eee(tp);
    r8153_enable_aldps(tp);
    r8153_enable_fc(tp);

    r8153_mdio_write(tp, 0x1f, 0x0000);
    r8153_mdio_write(tp, MII_BMCR, BMCR_RESET | BMCR_ANENABLE |
                     BMCR_ANRESTART);
    for (i = 0; i < 100; i++)
    {
        udelay(100);
        if (!(r8153_mdio_read(tp, MII_BMCR) & BMCR_RESET))
        { break; }
    }

    /* disable rx aggregation */
#if 0
    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_USB_CTRL);
    ocp_data |= RX_AGG_DISABLE;
    ocp_write_word(tp, MCU_TYPE_USB, USB_USB_CTRL, ocp_data);
#else
    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, USB_USB_CTRL);
    ocp_data &= ~RX_AGG_DISABLE;
    ocp_write_word(tp, MCU_TYPE_USB, USB_USB_CTRL, ocp_data);
#endif
}

static int rtl8153_suspend(struct usb_interface* intf, pm_message_t message)
{
    struct r8153* tp = usb_get_intfdata(intf);

    netif_device_detach(tp->netdev);

    if (netif_running(tp->netdev))
    {
        clear_bit(WORK_ENABLE, &tp->flags);
        cancel_delayed_work_sync(&tp->schedule);
    }

    rtl8153_down(tp);
    usb_control_msg(tp->udev, usb_sndctrlpipe(tp->udev, 0),
                    USB_REQ_SET_FEATURE, USB_RECIP_DEVICE,
                    USB_DEVICE_REMOTE_WAKEUP, 0, NULL, 0,
                    500);

    return 0;
}

static int rtl8153_resume(struct usb_interface* intf)
{
    struct r8153* tp = usb_get_intfdata(intf);

    usb_control_msg(tp->udev, usb_sndctrlpipe(tp->udev, 0),
                    USB_REQ_CLEAR_FEATURE, USB_RECIP_DEVICE,
                    USB_DEVICE_REMOTE_WAKEUP, 0, NULL, 0,
                    500);

    r8153_init(tp);
    netif_device_attach(tp->netdev);
    if (netif_running(tp->netdev))
    {
        rtl8153_enable(tp);
        set_bit(WORK_ENABLE, &tp->flags);
        set_bit(RTL8153_SET_RX_MODE, &tp->flags);
        schedule_delayed_work(&tp->schedule, 0);
    }

    return 0;
}

static void rtl8153_get_drvinfo(struct net_device* netdev,
                                struct ethtool_drvinfo* info)
{
    struct r8153* tp = netdev_priv(netdev);

    strncpy(info->driver, MODULENAME, ETHTOOL_BUSINFO_LEN);
    strncpy(info->version, DRIVER_VERSION, ETHTOOL_BUSINFO_LEN);
    usb_make_path(tp->udev, info->bus_info, sizeof(info->bus_info));
}

static
int rtl8153_get_settings(struct net_device* netdev, struct ethtool_cmd* ecmd)
{
    struct r8153* tp = netdev_priv(netdev);
    u16 bmcr, bmsr, stat1000 = 0;

    ecmd->supported =
        (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
         SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
         SUPPORTED_Autoneg | SUPPORTED_MII);

    /* only supports twisted-pair */
    ecmd->port = PORT_MII;

    /* only supports internal transceiver */
    ecmd->transceiver = XCVR_INTERNAL;
    ecmd->phy_address = 32;
    ecmd->mdio_support = MDIO_SUPPORTS_C22;
    ecmd->advertising = ADVERTISED_MII;

    r8153_mdio_write(tp, 0x1f, 0x0000);
    bmcr = r8153_mdio_read(tp, MII_BMCR);
    bmsr = r8153_mdio_read(tp, MII_BMSR);
    stat1000 = r8153_mdio_read(tp, MII_STAT1000);

    if (bmcr & BMCR_ANENABLE)
    {
        int advert;

        ecmd->advertising |= ADVERTISED_Autoneg;
        ecmd->autoneg = AUTONEG_ENABLE;

        advert = r8153_mdio_read(tp, MII_ADVERTISE);
        if (advert & ADVERTISE_10HALF)
        { ecmd->advertising |= ADVERTISED_10baseT_Half; }
        if (advert & ADVERTISE_10FULL)
        { ecmd->advertising |= ADVERTISED_10baseT_Full; }
        if (advert & ADVERTISE_100HALF)
        { ecmd->advertising |= ADVERTISED_100baseT_Half; }
        if (advert & ADVERTISE_100FULL)
        { ecmd->advertising |= ADVERTISED_100baseT_Full; }
        if (advert & ADVERTISE_PAUSE_CAP)
        { ecmd->advertising |= ADVERTISED_Pause; }
        if (advert & ADVERTISE_PAUSE_ASYM)
        { ecmd->advertising |= ADVERTISED_Asym_Pause; }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
        if (bmsr & BMSR_ANEGCOMPLETE)
        {
            advert = r8153_mdio_read(tp, MII_LPA);
            if (advert & LPA_LPACK)
            { ecmd->lp_advertising |= ADVERTISED_Autoneg; }
            if (advert & ADVERTISE_10HALF)
                ecmd->lp_advertising |=
                    ADVERTISED_10baseT_Half;
            if (advert & ADVERTISE_10FULL)
                ecmd->lp_advertising |=
                    ADVERTISED_10baseT_Full;
            if (advert & ADVERTISE_100HALF)
                ecmd->lp_advertising |=
                    ADVERTISED_100baseT_Half;
            if (advert & ADVERTISE_100FULL)
                ecmd->lp_advertising |=
                    ADVERTISED_100baseT_Full;

            if (stat1000 & LPA_1000HALF)
                ecmd->lp_advertising |=
                    ADVERTISED_1000baseT_Half;
            if (stat1000 & LPA_1000FULL)
                ecmd->lp_advertising |=
                    ADVERTISED_1000baseT_Full;
        }
        else
        {
            ecmd->lp_advertising = 0;
        }
#endif
    }
    else
    {
        ecmd->autoneg = AUTONEG_DISABLE;
    }

    if (tp->speed & _100bps)
    { ecmd->speed = SPEED_100; }
    else if (tp->speed & _10bps)
    { ecmd->speed = SPEED_10; }
    else if (tp->speed & _1000bps)
    { ecmd->speed = SPEED_1000; }

    ecmd->duplex = (tp->speed & FULL_DUP) ? DUPLEX_FULL : DUPLEX_HALF;

    return 0;
}

static int rtl8153_set_settings(struct net_device* dev, struct ethtool_cmd* cmd)
{
    struct r8153* tp = netdev_priv(dev);

    return rtl8153_set_speed(tp, cmd->autoneg, cmd->speed, cmd->duplex);
}

static struct ethtool_ops ops =
{
    .get_drvinfo = rtl8153_get_drvinfo,
    .get_settings = rtl8153_get_settings,
    .set_settings = rtl8153_set_settings,
    .get_link = ethtool_op_get_link,
};

static int rtltool_ioctl(struct r8153* tp, struct ifreq* ifr)
{
    struct rtltool_cmd my_cmd, *myptr;
    struct usb_device_info* uinfo;
    struct usb_device* udev;
    __le32    ocp_data;
    void*    buffer;
    int    ret;

    myptr = (struct rtltool_cmd*)ifr->ifr_data;
    if (copy_from_user(&my_cmd, myptr, sizeof(my_cmd)))
    { return -EFAULT; }

    ret = 0;

    switch (my_cmd.cmd)
    {
        case RTLTOOL_PLA_OCP_READ_DWORD:
            pla_ocp_read(tp, (u16)my_cmd.offset, sizeof(ocp_data), &ocp_data);
            my_cmd.data = __le32_to_cpu(ocp_data);

            if (copy_to_user(myptr, &my_cmd, sizeof(my_cmd)))
            {
                ret = -EFAULT;
                break;
            }
            break;

        case RTLTOOL_PLA_OCP_WRITE_DWORD:
            ocp_data = __cpu_to_le32(my_cmd.data);
            pla_ocp_write(tp, (u16)my_cmd.offset, (u16)my_cmd.byteen, sizeof(ocp_data), &ocp_data);
            break;

        case RTLTOOL_USB_OCP_READ_DWORD:
            usb_ocp_read(tp, (u16)my_cmd.offset, sizeof(ocp_data), &ocp_data);
            my_cmd.data = __le32_to_cpu(ocp_data);

            if (copy_to_user(myptr, &my_cmd, sizeof(my_cmd)))
            {
                ret = -EFAULT;
                break;
            }
            break;


        case RTLTOOL_USB_OCP_WRITE_DWORD:
            ocp_data = __cpu_to_le32(my_cmd.data);
            usb_ocp_write(tp, (u16)my_cmd.offset, (u16)my_cmd.byteen, sizeof(ocp_data), &ocp_data);
            break;

        case RTLTOOL_PLA_OCP_READ:
            buffer = kmalloc(my_cmd.data, GFP_KERNEL);
            if (!buffer)
            {
                ret = -ENOMEM;
                break;
            }

            pla_ocp_read(tp, (u16)my_cmd.offset, my_cmd.data, buffer);

            if (copy_to_user(myptr->buf, buffer, my_cmd.data))
            { ret = -EFAULT; }

            kfree(buffer);
            break;

        case RTLTOOL_PLA_OCP_WRITE:
            buffer = kmalloc(my_cmd.data, GFP_KERNEL);
            if (!buffer)
            {
                ret = -ENOMEM;
                break;
            }

            if (copy_from_user(buffer, myptr->buf, my_cmd.data))
            {
                ret = -EFAULT;
                kfree(buffer);
                break;
            }

            pla_ocp_write(tp, (u16)my_cmd.offset, (u16)my_cmd.byteen, my_cmd.data, buffer);
            kfree(buffer);
            break;

        case RTLTOOL_USB_OCP_READ:
            buffer = kmalloc(my_cmd.data, GFP_KERNEL);
            if (!buffer)
            {
                ret = -ENOMEM;
                break;
            }

            usb_ocp_read(tp, (u16)my_cmd.offset, my_cmd.data, buffer);

            if (copy_to_user(myptr->buf, buffer, my_cmd.data))
            { ret = -EFAULT; }

            kfree(buffer);
            break;

        case RTLTOOL_USB_OCP_WRITE:
            buffer = kmalloc(my_cmd.data, GFP_KERNEL);
            if (!buffer)
            {
                ret = -ENOMEM;
                break;
            }

            if (copy_from_user(buffer, myptr->buf, my_cmd.data))
            {
                ret = -EFAULT;
                kfree(buffer);
                break;
            }

            usb_ocp_write(tp, (u16)my_cmd.offset, (u16)my_cmd.byteen, my_cmd.data, buffer);
            kfree(buffer);
            break;

        case RTLTOOL_USB_INFO:
            uinfo = (struct usb_device_info*)&my_cmd.nic_info;
            udev = tp->udev;
            uinfo->idVendor = udev->descriptor.idVendor;
            uinfo->idProduct = udev->descriptor.idProduct;
            uinfo->bcdDevice = udev->descriptor.bcdDevice;
            memcpy(uinfo->devpath, udev->devpath, sizeof(udev->devpath));
            pla_ocp_read(tp, PLA_IDR, sizeof(uinfo->dev_addr), uinfo->dev_addr);

            if (copy_to_user(myptr, &my_cmd, sizeof(my_cmd)))
            { ret = -EFAULT; }

            break;
        default:
            ret = -EOPNOTSUPP;
            break;
    }

    return ret;
}

static int rtl8153_ioctl(struct net_device* netdev, struct ifreq* rq, int cmd)
{
    struct r8153* tp = netdev_priv(netdev);
    struct mii_ioctl_data* data = if_mii(rq);
    int res = 0;

    switch (cmd)
    {
        case SIOCGMIIPHY:
            data->phy_id = R8153_PHY_ID; /* Internal PHY */
            break;

        case SIOCGMIIREG:
            r8153_mdio_write(tp, 0x1f, 0x0000);
            data->val_out = r8153_mdio_read(tp, data->reg_num);
            break;

        case SIOCSMIIREG:
            if (!capable(CAP_NET_ADMIN))
            {
                res = -EPERM;
                break;
            }
            r8153_mdio_write(tp, 0x1f, 0x0000);
            r8153_mdio_write(tp, data->reg_num, data->val_in);
            break;

        case SIOCDEVPRIVATE:
            res = rtltool_ioctl(tp, rq);
            break;

        default:
            res = -EOPNOTSUPP;
    }

    return res;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static const struct net_device_ops rtl8153_netdev_ops =
{
    .ndo_open        = rtl8153_open,
    .ndo_stop        = rtl8153_close,
    .ndo_do_ioctl        = rtl8153_ioctl,
    .ndo_start_xmit        = rtl8153_start_xmit,
    .ndo_tx_timeout        = rtl8153_tx_timeout,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,1,0)
    .ndo_set_multicast_list = rtl8153_set_rx_mode,
#else
    .ndo_set_rx_mode    = rtl8153_set_rx_mode,
#endif
    .ndo_set_mac_address    = rtl8153_set_mac_address,

    .ndo_change_mtu        = eth_change_mtu,
    .ndo_validate_addr    = eth_validate_addr,
};
#endif

static void r8153_get_version(struct r8153* tp)
{
    u32    ocp_data;
    u16    version;

    ocp_data = ocp_read_word(tp, MCU_TYPE_PLA, PLA_TCR1);
    version = (u16)(ocp_data & VERSION_MASK);

    switch (version)
    {
        case 0x5c00:
            tp->version = RTL_VER_01;
            break;
        case 0x5c10:
            tp->version = RTL_VER_02;
            break;
        default:
            netif_info(tp, probe, tp->netdev,
                       "Unknown version 0x%04x\n", version);
            break;
    }
}

static int rtl8153_probe(struct usb_interface* intf,
                         const struct usb_device_id* id)
{
    struct usb_device* udev = interface_to_usbdev(intf);
    struct r8153* tp;
    struct net_device* netdev;
    int node, i;

    if (udev->actconfig->desc.bConfigurationValue != 1)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
        dev_err(&intf->dev, "The kernel too old to set configuration!!!");
#else
        usb_driver_set_configuration(udev, 1);
#endif
        return -ENODEV;
    }

    //    usb_reset_device(udev);
    netdev = alloc_etherdev(sizeof(struct r8153));
    if (!netdev)
    {
        dev_err(&intf->dev, "Out of memory");
        return -ENOMEM;
    }

    SET_NETDEV_DEV(netdev, &intf->dev);
    tp = netdev_priv(netdev);
    memset(tp, 0, sizeof(*tp));
    tp->msg_enable = 0x7FFF;

    tp->intr_buff = kmalloc(INTBUFSIZE, GFP_KERNEL);
    if (!tp->intr_buff)
    {
        free_netdev(netdev);
        return -ENOMEM;
    }
    tasklet_init(&tp->tl, rx_bottom, (unsigned long)tp);

    rtl_tp = tp ;
    //INIT_WORK (&tp->usbwork, rx_bottom);


    INIT_DELAYED_WORK(&tp->schedule, rtl_work_func_t);

    tp->udev = udev;
    tp->netdev = netdev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
    netdev->netdev_ops = &rtl8153_netdev_ops;
#else
    netdev->open = rtl8153_open;
    netdev->stop = rtl8153_close;
    netdev->get_stats = rtl8153_get_stats;
    netdev->hard_start_xmit = rtl8153_start_xmit;
    netdev->tx_timeout = rtl8153_tx_timeout;
    //    netdev->change_mtu = eth_change_mtu;
    netdev->set_mac_address = rtl8153_set_mac_address;
    netdev->do_ioctl = rtl8153_ioctl;
    netdev->set_multicast_list = rtl8153_set_rx_mode;
#endif /* HAVE_NET_DEVICE_OPS */
    netdev->watchdog_timeo = RTL8153_TX_TIMEOUT;
    netdev->features &= ~NETIF_F_IP_CSUM;

    SET_ETHTOOL_OPS(netdev, &ops);
    tp->intr_interval = 100;    /* 100ms */
    tp->speed = 0;

    tp->mii.dev = netdev;
    tp->mii.mdio_read = read_mii_word;
    tp->mii.mdio_write = write_mii_word;
    tp->mii.phy_id_mask = 0x3f;
    tp->mii.reg_num_mask = 0x1f;
    tp->mii.phy_id = R8153_PHY_ID;
    tp->mii.supports_gmii = 1;

    r8153_get_version(tp);
    r8153_init(tp);
    set_ethernet_addr(tp);

    if (!alloc_all_urbs(tp))
    {
        netif_err(tp, probe, netdev, "out of memory");
        goto out;
    }

    node = netdev->dev.parent ? dev_to_node(netdev->dev.parent) : -1;

    for (i = 0; i < RTL8153_MAX_RX; i++)
    {
        tp->rx_buf[i] = kmalloc_node(rx_buf_sz + 8, GFP_KERNEL, node);
        if (!tp->rx_buf[i])
        { goto out1; }
    }

    usb_set_intfdata(intf, tp);


    if (register_netdev(netdev) != 0)
    {
        netif_err(tp, probe, netdev, "couldn't register the device");
        goto out2;
    }

    netif_info(tp, probe, netdev, "%s", PATENTS);

    mbb_usbnet_set_last_net_state(CRADLE_REMOVE);
    mbb_usbnet_set_net_state(CRADLE_REMOVE);
    mbb_usbnet_usb_state_notify(USB_CRADLE_ATTACH);
    mbb_usbnet_set_speed(CRADLE_SPEED_UNKOWN);

    return 0;

out2:
    usb_set_intfdata(intf, NULL);
out1:
    for (i = 0; i < RTL8153_MAX_RX; i++)
        if (tp->rx_buf[i])
        { kfree(tp->rx_buf[i]); }

    free_all_urbs(tp);
out:
    kfree(tp->intr_buff);
    free_netdev(netdev);
    return -EIO;
}

static void rtl8153_unload(struct r8153* tp)
{
    u32    ocp_data;

    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, 0xd80a);
    ocp_data |= 0x0001;
    ocp_write_word(tp, MCU_TYPE_USB, 0xd80a, ocp_data);

    ocp_data = ocp_read_word(tp, MCU_TYPE_USB, 0xd81a);
    ocp_data &= ~RWSUME_INDICATE;
    ocp_write_word(tp, MCU_TYPE_USB, 0xd81a, ocp_data);
}

static void rtl8153_disconnect(struct usb_interface* intf)
{
    struct r8153* tp = usb_get_intfdata(intf);


    mbb_usbnet_net_state_notify(CRADLE_REMOVE);
    mbb_usbnet_usb_state_notify(USB_CRADLE_REMOVE);

    usb_set_intfdata(intf, NULL);
    if (tp)
    {
        int i;

        set_bit(RTL8153_UNPLUG, &tp->flags);
        tasklet_kill(&tp->tl);
        unregister_netdev(tp->netdev);
        rtl8153_unload(tp);
        free_all_urbs(tp);
        for (i = 0; i < RTL8153_MAX_RX; i++)
            if (tp->rx_buf[i])
            { kfree(tp->rx_buf[i]); }
        kfree(tp->intr_buff);
        free_netdev(tp->netdev);
    }
#if (FEATURE_ON == MBB_CTF_COMMON)
    if(tp)
    {
        mbb_usbnet_ctf_disable(tp->netdev);
    }

#endif /* MBB_CTF_COMMON */

    g_usb_ether_bind = 0;
    g_udev = NULL;
    mbb_usbnet_set_speed(CRADLE_SPEED_INVAILD);


}

/* table of devices that work with this driver */
static struct usb_device_id rtl8153_table[] =
{
    {USB_DEVICE(VENDOR_ID_REALTEK, PRODUCT_ID_RTL8153)},
    {}
};

MODULE_DEVICE_TABLE(usb, rtl8153_table);

static struct usb_driver rtl8153_driver =
{
    .name =        MODULENAME,
    .probe =    rtl8153_probe,
    .disconnect =    rtl8153_disconnect,
    .id_table =    rtl8153_table,
    .suspend =    rtl8153_suspend,
    .resume =    rtl8153_resume
};

static int __init usb_rtl8153_init(void)
{
    return usb_register(&rtl8153_driver);
}

static void __exit usb_rtl8153_exit(void)
{
    usb_deregister(&rtl8153_driver);
}
extern int g_usb_ether_bind;
typedef struct r8153 r8153_t;
void rtl8153_set_low_power()
{
    unsigned short ocp_data = 0;
    r8153_t* tp_tmp;
    /*¼ì²âµ××ùÔÚÎ»´ÎÊý*/
    unsigned int wait_times = 100;

    tp_tmp = rtl_tp;
    do
    {
        wait_times--;
        if ((NULL == tp_tmp) || (0 == g_usb_ether_bind))
        {
            printk(KERN_ERR"hi,the af22 is not ok,please wait\r\n");
        }
        else
        {
            break;
        }
    }
    while (wait_times);
    if ( 0 == wait_times )
    {
        printk(KERN_ERR"hi,the af22 is not in\r\n");
        return;
    }
    if (NULL != tp_tmp)
    {
        clear_bit(WORK_ENABLE, &tp_tmp->flags);
        cancel_delayed_work_sync(&tp_tmp->schedule);
    }
    /*0xd80aÊý×ÖÊÇRealtek³§ÉÌÌá¹©£¬Ã»ÓÐ¼Ä´æÆ÷ÊÖ²á*/
    ocp_data = ocp_read_word(tp_tmp, MCU_TYPE_USB, 0xd80a);
    /*0x0009Êý×ÖÊÇRealtek³§ÉÌÌá¹©£¬Ã»ÓÐ¼Ä´æÆ÷ÊÖ²á*/
    ocp_data |= 0x0009;
    /*0xd80aÊý×ÖÊÇRealtek³§ÉÌÌá¹©£¬Ã»ÓÐ¼Ä´æÆ÷ÊÖ²á*/
    ocp_write_word(tp_tmp, MCU_TYPE_USB, 0xd80a, ocp_data);

    /*0xd81aÊý×ÖÊÇRealtek³§ÉÌÌá¹©£¬Ã»ÓÐ¼Ä´æÆ÷ÊÖ²á*/
    ocp_data = ocp_read_word(tp_tmp, MCU_TYPE_USB, 0xd81a);
    ocp_data &= ~RWSUME_INDICATE;
    /*0xd81aÊý×ÖÊÇRealtek³§ÉÌÌá¹©£¬Ã»ÓÐ¼Ä´æÆ÷ÊÖ²á*/
    ocp_write_word(tp_tmp, MCU_TYPE_USB, 0xd81a, ocp_data);
}
EXPORT_SYMBOL(rtl8153_set_low_power);
/*************************************************
  Function:     usb_r8153_loop
  Description:  ÉèÖÃÐ¾Æ¬ÎªÄÚ»·»ØÄ£Ê½£¬ÓÃ»§ÀÏ»¯²âÊÔ
  Calls:           rtlË½ÓÐÃüÁî
  Called By:     ÓÃ»§Ì¬Í¨¹ýecallÀ´µ÷ÓÃ
  Input:        ÎÞ
  Output:      ÎÞ
  Return:      ÎÞ
  Others:    ¸Ãº¯Êý±»µ÷ÓÃºó£¬Ð¾Æ¬´¦ÓÚ²âÊÔÄ£Ê½£¬ÎÞ·¨Õý³£
               Ê¹ÓÃ£¬ÈçÐè»Ö¸´£¬²å°Îµ××ù¡£
*************************************************/
void usb_r8153_loop()
{
    __le32 data;

    struct usb_device* udev;

    if ((NULL == g_udev) || (0 == g_usb_ether_bind))
    {
        printk(KERN_ERR"hi,you cannot call this function,the usb ether is not bind\r\n");

        return;
    }

    udev = g_udev;

    if (__le16_to_cpu(udev->descriptor.idVendor) != 0x0bda ||
        __le16_to_cpu(udev->descriptor.idProduct) != 0x8153)
    { return; }

    data = __cpu_to_le32(0x0000a000);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40, 0xe86c, 0x0133,
                    &data, sizeof(data), 500);

    data = __cpu_to_le32(0x00001200);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40, 0xb400, 0x0133,
                    &data, sizeof(data), 500);

    usb_control_msg(udev, usb_rcvctrlpipe(udev, 0), 0x05, 0xc0,
                    0xe610, 0x0100, &data, sizeof(data), 500);
    data &= __cpu_to_le32(~0x00040000);
    data |= __cpu_to_le32(0x0002e000);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40, 0xe610, 0x01ff,
                    &data, sizeof(data), 500);

    data = __cpu_to_le32(0x0000002c);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40, 0xcf00, 0x0011,
                    &data, sizeof(data), 500);
}
EXPORT_SYMBOL(usb_r8153_loop);

/*************************************************
  Function:     usb_r8153_nway
  Description:   ¸´Î»rtl8153Ð¾Æ¬
  Calls:           rtlË½ÓÐÃüÁî
  Called By:     ÓÃ»§Ì¬Í¨¹ýecallÀ´µ÷ÓÃ
  Input:        ÎÞ
  Output:      ÎÞ
  Return:      ÎÞ
  Others:
*************************************************/


void usb_r8153_nway()
{
    __le32 data;

    struct usb_device* udev;

    if ((NULL == g_udev) || (0 == g_usb_ether_bind))
    {
        printk(KERN_ERR"hi,you cannot call this function,the usb ether is not bind\r\n");

        return;
    }

    udev = g_udev;

    if (__le16_to_cpu(udev->descriptor.idVendor) != 0x0bda ||
        __le16_to_cpu(udev->descriptor.idProduct) != 0x8153)
    { return; }

    data = __cpu_to_le32(0x0000a000);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40,
                    0xe86c, 0x0133, &data, sizeof(data), 500);

    data = __cpu_to_le32(0x00001200);
    usb_control_msg(udev, usb_sndctrlpipe(udev, 0), 0x05, 0x40,
                    0xb400, 0x0133, &data, sizeof(data), 500);
}

EXPORT_SYMBOL(usb_r8153_nway);

module_init(usb_rtl8153_init);
module_exit(usb_rtl8153_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

