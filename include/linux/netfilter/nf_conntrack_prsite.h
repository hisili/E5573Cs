
#ifndef _NF_NAT_PRSITE_H_
#define _NF_NAT_PRSITE_H_

#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/atomic.h>
#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <net/netfilter/nf_conntrack.h>

#define HTTP_PORT 80

/* start of 防止用户态配置下来的字符串长度加上 http:// */ 
#define HTTP_URL_MAX (256 + 32)
/* end of 防止用户态配置下来的字符串长度加上 http://  */
#define HTTP_TRACE_CHECK_TIMEOUT 5 //minutes
#define HTTP_TRACE_TIMEOUT 30 //minutes
#define HTTP_PROTO_HEAD_BUF_MAX 16
#define HTTP_RESPONSE_BUF_MAX (HTTP_URL_MAX + 256)
#define AFFINED_ADDR_BUF_MAX 16

#define AFFINED_ADDR_STATIC_BASE 8

#define HTTP_PROTO_NAME "HTTP"
#define HTTP_PROTO_GET  "GET"
#if defined(CONFIG_FORCE_APP)
typedef enum tagNF_PRSITE_TYPE{
    PRSITE_TYPE_BBOU = 0,
    PRSITE_TYPE_MAC = 1,
    PRSITE_TYPE_BUT
}NF_PRSITE_TYPE;
#endif
struct prsite_url_info
{
    int          lEnable;
    unsigned int ul_lan_addr;
    unsigned int ul_lan_mask;
    char         ac_stb_url[HTTP_URL_MAX];
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    char         ac_stb_url_ssid2[HTTP_URL_MAX];
#endif
    char         ac_computer_url[HTTP_URL_MAX];
    char         ac_wifi_url[HTTP_URL_MAX];
};
#if defined(CONFIG_FORCE_APP)
typedef struct tagmac_addr
{
    u8 mac_addr[6]; // the shortest mem.
}Mac_Addr;

struct prsite_mac_list
{
    u32 mac_num;
    s32 mac_list[0];
};
#endif
//for ioctl
struct affined_bind
{
    unsigned int addr;
    unsigned int mask;
    unsigned int flag;
    char         url[HTTP_URL_MAX];
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
	char         urlssid2[HTTP_URL_MAX];
#endif
};
struct user_agent_list
{
    char *key_type;
    char *key_version;
    int version_num;
};
#if defined(CONFIG_FORCE_APP)
struct mac_list
{
    struct hlist_node node;
    unsigned char mac_addr[6];
};
#endif
/*modify redirect by gaoxiangbing begin*/
void delrandall(void);
int showrandall(char *buffer, size_t len);
void delrandbyid(char *pid);
/*modify redirect by gaoxiangbing end*/
extern unsigned int nf_prsite_in(struct sk_buff *skb, const struct net_device *pin, const struct net_device *pout);

#endif
