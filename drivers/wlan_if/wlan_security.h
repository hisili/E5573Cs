
#ifndef _wlan_security_h_
#define _wlan_security_h_

#include <linux/skbuff.h>
#include <linux/netdevice.h>

#ifdef __cplusplus
    #if __cplusplus
    extern "C" {
    #endif
#endif

/*****************************************************************************
 函数名称  : wlan_check_arp_spoofing
 功能描述  : 检测wlan的arp欺骗
 输入参数  : port_dev: 桥下挂设备, pskb: 需要检测的数据包
 输出参数  : NA
 返 回 值  : 0:不是ARP欺骗报文；1: 是ARP欺骗报文
*****************************************************************************/
int wlan_check_arp_spoofing(struct net_device *port_dev, struct sk_buff *pskb);

/*****************************************************************************
 函数名称  : wl_chk_pkt_inBSS
 功能描述  : 提供给BCM43236芯片驱动使用，用于判断STA->STA的报文是否满足arp欺骗的判断
 输入参数  : *p: 该包的skb指针 ,*from_device: wlan接口的net_device结构指针
 输出参数  : NA
 返 回 值  : 0:不是ARP欺骗报文；1: 是ARP欺骗报文
*****************************************************************************/
int wl_chk_pkt_inBSS(struct net_device *from_device, void *p);

#ifdef __cplusplus
    #if __cplusplus
    }
    #endif
#endif   
#endif /* _wlan_security_h_ */
