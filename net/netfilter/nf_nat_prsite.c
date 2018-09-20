/* (C) 2001-2002 Magnus Boden <mb@ozaba.mine.nu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Version: 0.0.7
 *
 * Thu 21 Mar 2002 Harald Welte <laforge@gnumonks.org>
 *  - Port to newnat API
 *
 * This module currently supports DNAT:
 * iptables -t nat -A PREROUTING -d x.x.x.x -j DNAT --to-dest x.x.x.y
 *
 * and SNAT:
 * iptables -t nat -A POSTROUTING { -j MASQUERADE , -j SNAT --to-source x.x.x.x }
 *
 * It has not been tested with
 * -j SNAT --to-source x.x.x.x-x.x.x.y since I only have one external ip
 * If you do test this please let me know if it works or not.
 *
 */

#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_conntrack_prsite.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <linux/slab.h>


#define  IGNORESTRING  "/?updataredirect="
#define  SUFFIXLEN     8
#define  DATALENGTH    1500
#define  RANDTIMEOUT   5
#define  RANDLENGTH    11

DEFINE_SPINLOCK(prsiterand_lock);
#if defined(CONFIG_FORCE_APP)
DEFINE_SPINLOCK(prsitemaclist_lock);
#endif
MODULE_LICENSE("GPL");

MODULE_DESCRIPTION("http NAT helper");
MODULE_ALIAS("nf_nat_http");
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
#define ACCESS_SSID2_MARK 0xfff1 
#endif

#if 1
 #define PRSITE_DEBUGP(format, args...) printk(format, ## args)
#else
 #define PRSITE_DEBUGP(format, args...)
#endif
#if 1
 #define PRSITE_STATIC_DEBUGP(format, args...) do {printk(format, ## args);} while (0)
#else
 #define PRSITE_STATIC_DEBUGP(format, args...)
#endif

#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

extern int (*prsite_add_hook)(char __user * optval);
extern int (*prsite_del_hook)(char __user * optval);
extern int (*prsite_ipconflict_hook)(char __user * optval);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
extern int (*ssid2_checkurl_del_hook)(char __user * optval);
#endif
#if defined(CONFIG_FORCE_APP)
extern int (*prsite_setmaclist_hook)(char __user * optval, unsigned int optlen);
#endif

/*************** start of 添加静态IP地址，默认Computer。add by c60023298 ***************/
struct st_randinfo
{
    __be32 id;
    long   ts; 
    struct list_head          randlist;
};
struct prsite_url_info g_stPrsiteUrlInfo = {0};

struct list_head  randomlist;
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
int g_whitelist_enable = 0;
int g_ssid2_checkurl = 0;/*是否check ssid2 的url 0表示不check 1表示check*/
char g_whitelist[HTTP_URL_MAX] = "ymobile.jp";
#endif

static struct user_agent_list g_browser_list[] =
{
    //Safari浏览器
    {"Safari", "",         0         },
    //IE浏览器
    {"MSIE", "",         0         },
    //Chrome浏览器
    {"Chrome","",          0         },
    //Opera浏览器
    //{"Opera", "OPR/",   12          },
    {"Opera","",          0         },
    //Firefox浏览器
    {"Firefox","",          0         },
    //UCWEB浏览器
    {"UCWEB","",          0         },
    //IE11
    {"rv:11.0","",          0         },
    {NULL, NULL ,      0         }
};

static char g_blackurlsuffix [][SUFFIXLEN] = 
{
    "gif","jpeg","tiff","ico",
    "xbm","xpm","png","erf",
    "jpg","jpe","rgb","svf",
    "tif","pict","bmp"
};


#if defined(CONFIG_FORCE_APP)
int gMacSwitch = 0;

struct hlist_head hhlist;

int gdelmacenable = 0;

void delmac(int enable)
{
    gdelmacenable = enable;
}

void printmaclist(void)
{
    struct mac_list *m;
    struct hlist_node *n;

    if (hlist_empty(&hhlist)){
        PRSITE_DEBUGP("list empty.\n");
    }
    spin_lock_bh(&prsitemaclist_lock);
    hlist_for_each_entry(m, n, &hhlist, node)
    {
        PRSITE_DEBUGP("mac=%02X:%02X:%02X:%02X:%02X:%02X\n",
            m->mac_addr[0],
            m->mac_addr[1],
            m->mac_addr[2],
            m->mac_addr[3],
            m->mac_addr[4],
            m->mac_addr[5]);
    }
    spin_unlock_bh(&prsitemaclist_lock);
}

static void addtomaclist(unsigned char *mac)
{
    struct mac_list *m;
    struct hlist_node *n;

    spin_lock_bh(&prsitemaclist_lock);
    hlist_for_each_entry(m, n, &hhlist, node)
    {
        if(!memcmp(m->mac_addr, mac, 6))
        {
            PRSITE_DEBUGP("has same one.\n");
            return;
        }
    }

    m = kmalloc(sizeof(struct mac_list), GFP_KERNEL);
    memcpy(&m->mac_addr, mac, 6);
    INIT_HLIST_NODE(&m->node);
    hlist_add_head(&m->node, &hhlist);
    spin_unlock_bh(&prsitemaclist_lock);
    PRSITE_DEBUGP("mac list added.\n");
}

static int check_mac_match(unsigned char *mac)
{
    struct mac_list *m;
    struct hlist_node *n;

    if (hlist_empty(&hhlist))
    {
        return 0;
    }
    spin_lock_bh(&prsitemaclist_lock);
    hlist_for_each_entry(m, n, &hhlist, node)
    {
        if(!memcmp(m->mac_addr, mac, 6))
        {
            return 1;
        }
    }
    spin_unlock_bh(&prsitemaclist_lock);
    return 0;
}

static void del_from_maclist(unsigned char *mac)
{
    struct mac_list *m;
    struct hlist_node *n, *t;
    if (hlist_empty(&hhlist))
    {
        PRSITE_DEBUGP("list empty.\n");
        return;
    }
    spin_lock_bh(&prsitemaclist_lock);
    hlist_for_each_entry_safe(m, t, n, &hhlist, node)
    {
        if(!memcmp(m->mac_addr, mac, 6))
        {
            hlist_del_init(&m->node);
            kfree(m);
            m = NULL;
        }
    }
    spin_unlock_bh(&prsitemaclist_lock);

}

static void free_maclist()
{
    struct hlist_node *n, *t;
    struct mac_list *m;
    if (hlist_empty(&hhlist))
    {
        PRSITE_DEBUGP("list empty.\n");
        return;
    }
    spin_lock_bh(&prsitemaclist_lock);
    hlist_for_each_entry_safe(m, t, n, &hhlist, node)
    {
        hlist_del_init(&m->node);
        kfree(m);
        m = NULL;
    }
    spin_unlock_bh(&prsitemaclist_lock);

}

void prsite_dump()
{
    PRSITE_DEBUGP("++++++++++++++++++++++++++++++++++++++++++++++\n");
    PRSITE_DEBUGP("g_stPrsiteUrlInfo.lEnable = %d\n", g_stPrsiteUrlInfo.lEnable);
    PRSITE_DEBUGP("g_stPrsiteUrlInfo.ac_stb_url = %s\n", g_stPrsiteUrlInfo.ac_stb_url);
    PRSITE_DEBUGP("g_stPrsiteUrlInfo.ul_lan_addr = %x\n", g_stPrsiteUrlInfo.ul_lan_addr);
    PRSITE_DEBUGP("gMacSwitch = %x\n", gMacSwitch);
    PRSITE_DEBUGP("++++++++++++++++++++++++++++++++++++++++++++++\n");
}

#endif
static int http_set_appdata(struct sk_buff *poldskb, void *pdata, int len)
{
    struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    u_int32_t tcplen;
    u_int32_t datalen;
    struct sk_buff *newskb = NULL;
    unsigned int taddr = 0;           /* Temporary IP holder */
    short tport = 0;
    u_int32_t tseq = 0;
    struct ethhdr *peth = NULL;
    unsigned char t_hwaddr[ETH_ALEN] = {0};
    struct net_device *dev = NULL;
    struct sk_buff *pskb = NULL;
    int tmplen = 0;
    int tmptcplen = 0;


    if (NULL == poldskb || (NULL == pdata))
    {
        return 1;
    }
    
    pskb = skb_copy(poldskb, GFP_ATOMIC);
    
    if (NULL == pskb)
    {
        return 1;
    }
    iph = ip_hdr(pskb);
    
    if (NULL == iph)
    {
        kfree_skb(pskb);
        return 1;
    } 
    tcph = (struct tcphdr *)(pskb->data + iph->ihl * 4);
    if(NULL == tcph)
    {
        kfree_skb(pskb);
        return 1;
    }
#if 0    
    if (skb_shinfo(pskb)->frag_list)
    {
        newskb = skb_copy(pskb, GFP_ATOMIC);
        if (!newskb)
        {
            return 1;
        }

        kfree_skb(pskb);
        pskb = newskb;
    }
#endif
    if ((pskb->end - pskb->data - iph->ihl * 4 - tcph->doff * 4) < len)
    {   
        PRSITE_DEBUGP("data len %d expand len %d\n",
                       pskb->end - pskb->data - iph->ihl * 4 - tcph->doff * 4,len);
        struct sk_buff * skb2 = skb_copy_expand(pskb, 0, len, GFP_ATOMIC);
        if (NULL == skb2)
        {
            kfree_skb(pskb);
            return 1;
        }

        kfree_skb(pskb);
        pskb = skb2;
    }
    
    iph = ip_hdr(pskb);
    
    if (NULL == iph)
    {
        kfree_skb(pskb);
        return 1;
    } 
    tcph = (struct tcphdr *)(pskb->data + iph->ihl * 4);
    if(NULL == tcph)
    {
        kfree_skb(pskb);
        return 1;
    }

    tmplen = pskb->len - iph->ihl * 4 - sizeof(struct tcphdr);
    PRSITE_DEBUGP("tcp data %d \n",tmplen);
    memset(pskb->data + iph->ihl * 4 + sizeof(struct tcphdr),
           0, pskb->len - iph->ihl * 4 - sizeof(struct tcphdr));
      

    memcpy(pskb->data + iph->ihl * 4 + sizeof(struct tcphdr),
           pdata, len);

    tmptcplen = (tcph->doff * 4);
    tcph->doff  = sizeof(struct tcphdr) / 4;
    tport = tcph->source;
    tcph->source = tcph->dest;
    tcph->dest = tport;
    
    tseq = tcph->ack_seq;
  
    //tcph->ack_seq = htonl(ntohl(tcph->seq) + tcph->syn + tcph->fin +
    //                pskb->len - ip_hdrlen(pskb) -
    //                (tcph->doff << 2)); 
    tcph->ack_seq = htonl(ntohl(tcph->seq) + tcph->syn + tcph->fin +
                      pskb->len - ip_hdrlen(pskb) -
                      tmptcplen);                       
    tcph->seq = tseq;
    tcph->ack = 1;  
   // tcph->fin = 1;

    taddr = iph->saddr;
    iph->saddr = iph->daddr;
    iph->daddr = taddr;
    iph->ttl -= 1;

    iph->tot_len = htons(iph->ihl * 4 + sizeof(struct tcphdr) + len);
    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
    /* fix checksum information */
    if (tmplen > len)
    {
        tcplen  = pskb->len - iph->ihl * 4 - (tmplen - len);    
    }
    else
    {
       tcplen  = pskb->len - iph->ihl * 4;
    }
    
    datalen = tcplen - tcph->doff * 4;
    pskb->csum = csum_partial((char *)tcph + tcph->doff * 4, datalen, 0);

    tcph->check = 0;
    tcph->check = tcp_v4_check(tcplen, iph->saddr, iph->daddr,
                               csum_partial((char *)tcph, tcph->doff * 4,
                                            pskb->csum));

    //pskb->pkt_type = PACKET_OTHERHOST;
    pskb->pkt_type = PACKET_OUTGOING;
//===================================


    pskb->data = (unsigned char *)pskb->mac_header;
    pskb->tail = pskb->data + ETH_HLEN  + sizeof(struct iphdr) + sizeof(struct tcphdr) + len;
    //pskb->tail = pskb->data + ETH_HLEN  + (iph->ihl * 4) + sizeof(struct tcphdr) + len;
    pskb->len = pskb->tail - pskb->data;
    peth = pskb->mac_header;
    if (NULL == peth)
    {
        PRSITE_DEBUGP("eth_hdr error ======>\n");
        kfree_skb(pskb);
        return 1;
    }        
    if (pskb->tail > pskb->end)
    {
        PRSITE_DEBUGP("prsite deal with error,ignore this buf \n");
        kfree_skb(pskb);
        return 1;
    }
   
    memcpy(t_hwaddr, peth->h_dest, ETH_ALEN); 
    memcpy(peth->h_dest, peth->h_source, ETH_ALEN);  
    memcpy(peth->h_source, t_hwaddr, ETH_ALEN);
    
//==========================================    

    
    dev_queue_xmit(pskb);

    return 0;
}
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
int ssid2_checkurl_del(char __user *optval)
{
    g_ssid2_checkurl = 0;
    PRSITE_DEBUGP(" g_ssid2_checkurl = 0\n");
    return 0;
}
#endif
#if defined(CONFIG_FORCE_APP)
int prsite_setmaclist(char __user *optval, unsigned int optlen)
{
    int ret = 0;
    int count = 0;
    struct prsite_mac_list *prsite_mlist;
    Mac_Addr *mlist = NULL;
    prsite_mlist = (struct prsite_mac_list *)kmalloc(optlen, GFP_KERNEL);
    memset(prsite_mlist, 0, optlen);
    
    if (copy_from_user(prsite_mlist, optval, optlen))
    {
        ret = 1;
        return ret;
    }

    free_maclist();

    mlist = (Mac_Addr*)prsite_mlist->mac_list;

    for (count = 0; count < ((prsite_mlist->mac_num <= 10)?prsite_mlist->mac_num:10); ++count)
    {
        addtomaclist(mlist[count].mac_addr);
    }

    kfree(prsite_mlist);
    prsite_mlist = NULL;
    return 0;
}

#endif
/*增加强制门户节点*/
int prsite_add(char __user *optval)
{
    int ret = 0;
    struct affined_bind bind   = {0};

    PRSITE_DEBUGP("prsite_add ======>\n");
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    memset(&g_stPrsiteUrlInfo,0,sizeof(g_stPrsiteUrlInfo));
#endif
#if defined(CONFIG_FORCE_APP)
    free_maclist();
#endif

    if (copy_from_user(&bind, optval, sizeof(struct affined_bind)))
    {
        ret = 1;
        return ret;
    }

    if (strlen(bind.url) == 0)
    {
        g_stPrsiteUrlInfo.lEnable = 0;
        return ret;
    }
    else
    {
        g_stPrsiteUrlInfo.ul_lan_addr = ntohl(bind.addr);
        g_stPrsiteUrlInfo.ul_lan_mask = ntohl(bind.mask);
        strncpy(g_stPrsiteUrlInfo.ac_stb_url, bind.url, HTTP_URL_MAX);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
        strncpy(g_stPrsiteUrlInfo.ac_stb_url_ssid2, bind.urlssid2, HTTP_URL_MAX);
#endif
        g_stPrsiteUrlInfo.lEnable = bind.flag;
    }

#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    if(0 != strstr(g_stPrsiteUrlInfo.ac_stb_url,"limitspeed.html"))
    {
        PRSITE_DEBUGP("g_whitelist_enable = 1\n");
        g_whitelist_enable = 1;
    }
    else
    {
        g_whitelist_enable = 0;
    }
    g_ssid2_checkurl = 1;
    PRSITE_DEBUGP("g_ssid2_checkurl = 1\n");

    PRSITE_DEBUGP("++++++ ul_lan_addr %u ul_lan_mask %u ac_stb_url %s ac_stb_url_ssid2 %s g_stPrsiteUrlInfo.lEnable %d\n",
        g_stPrsiteUrlInfo.ul_lan_addr,g_stPrsiteUrlInfo.ul_lan_mask,g_stPrsiteUrlInfo.ac_stb_url,g_stPrsiteUrlInfo.ac_stb_url_ssid2, g_stPrsiteUrlInfo.lEnable);
#else
    PRSITE_DEBUGP("++++++ ul_lan_addr %u ul_lan_mask %u ac_stb_url %s g_stPrsiteUrlInfo.lEnable %d\n",
        g_stPrsiteUrlInfo.ul_lan_addr,g_stPrsiteUrlInfo.ul_lan_mask,g_stPrsiteUrlInfo.ac_stb_url,g_stPrsiteUrlInfo.lEnable);
#endif

   // PRSITE_STATIC_DEBUGP("\n\t add ipaddr <%u.%u.%u.%u>", NIPQUAD(g_stPrsiteUrlInfo.ul_lan_addr));
  //  PRSITE_STATIC_DEBUGP("\n\t add mask <%u.%u.%u.%u>", NIPQUAD(g_stPrsiteUrlInfo.ul_lan_mask));
    return ret;
}

/*删除强制门户节点*/
int prsite_del(char __user *optval)
{
    PRSITE_DEBUGP("prsite_del ======>\n");
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    g_stPrsiteUrlInfo.lEnable = 0;
    g_whitelist_enable = 0;
#else
    memset(&g_stPrsiteUrlInfo,0,sizeof(g_stPrsiteUrlInfo));
#endif
#if defined(CONFIG_FORCE_APP)
    free_maclist();
#endif

    return 0;
}
/*IP冲突，更新网管地址*/
int prsite_ipconflict(char __user *optval)
{
    int ret = 0;
    struct affined_bind bind   = {0};
    char oldaddress[AFFINED_ADDR_BUF_MAX] = {0};
    char newurl[HTTP_URL_MAX] = {0};
    char *strsplit = NULL;
    unsigned int haddr = 0;

    PRSITE_DEBUGP("prsite_ipconflict enter\n");
    if (copy_from_user(&bind, optval, sizeof(struct affined_bind)))
    {
        ret = 1;
        return ret;
    }

    snprintf(oldaddress, AFFINED_ADDR_BUF_MAX, "%u.%u.%u.%u", NIPQUAD(g_stPrsiteUrlInfo.ul_lan_addr));

    if (strncmp(oldaddress, g_stPrsiteUrlInfo.ac_stb_url, strlen(oldaddress)) == 0)
    {
        strsplit = g_stPrsiteUrlInfo.ac_stb_url + strlen(oldaddress);
        haddr = ntohl(bind.addr);
        snprintf(newurl, HTTP_URL_MAX, "%u.%u.%u.%u%s", NIPQUAD(haddr), strsplit);
        snprintf(g_stPrsiteUrlInfo.ac_stb_url, HTTP_URL_MAX, "%s", newurl);
    }

#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    if (strncmp(oldaddress, g_stPrsiteUrlInfo.ac_stb_url_ssid2, strlen(oldaddress)) == 0)
    {
        strsplit = g_stPrsiteUrlInfo.ac_stb_url_ssid2 + strlen(oldaddress);
        haddr = ntohl(bind.addr);
        snprintf(newurl, HTTP_URL_MAX, "%u.%u.%u.%u%s", NIPQUAD(haddr), strsplit);
        snprintf(g_stPrsiteUrlInfo.ac_stb_url_ssid2, HTTP_URL_MAX, "%s", newurl);
    }
#endif

    g_stPrsiteUrlInfo.ul_lan_addr = ntohl(bind.addr);
    g_stPrsiteUrlInfo.ul_lan_mask = ntohl(bind.mask);
    return ret;
}
int isspace(int x)
{
    if(' ' == x || '\t' == x || '\n' == x || '\f' == x || '\b' == x || '\r' == x)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int isdigit(int x)
{
    if(x <= '9' && x >= '0')
    {
        return 1;
    }
    else 
    {
        return 0;
    }
}

//atoi实现原型
int s_atoi(const char *nptr)
{
    int c;              /* current char */
    int total;         /* current total */
    int sign;           /* if '-', then negative, otherwise positive */

    /* skip whitespace */
    while ( isspace((int)(unsigned char)*nptr) )
    {
        ++nptr;
    }
    c = (int)(unsigned char)*nptr++;
    sign = c;           /* save sign indication */
    if ('-' == c || '+' == c)
    {
        c = (int)(unsigned char)*nptr++;    /* skip sign */
    }

    total = 0;

    while (isdigit(c)) 
    {
        total = 10 * total + (c - '0');     /* accumulate digit */
        c = (int)(unsigned char)*nptr++;    /* get next char */
    }

    if ('-' == sign)
    {
        return -total;
    }
    else
    {
        return total;   /* return result, negated if necessary */
    }
}

int check_version_white_list(char *puser_agent,int version)
{
    int num;
    if ( NULL == puser_agent)
    {
        return 1;
    }
    num = s_atoi(puser_agent);
    PRSITE_DEBUGP("change to num = %d\n",num);
    if( num >= version)
    {
        return 0;
    }
    return 1;
}

static int checkuser_agent_version(char *puser_agent)
{
    int i,j = 0;
    char *temp = NULL;
    char str_version[32] = {0};
    temp = puser_agent;
    for (i = 0; NULL != g_browser_list[i].key_type; i++)
    {
        //关键字在浏览器列表中
        if (NULL != strstr(puser_agent,g_browser_list[i].key_type))
        {
            //判断版本号
            temp = strstr(puser_agent,g_browser_list[i].key_version);
            if (NULL != temp)
            {
                temp = temp + strlen(g_browser_list[i].key_version);
                while (*temp != ' ')
                {
                    if (j >= 31)
                    {
                        return 1;
                    }

                     str_version[j] = *temp;
                     j++;
                     temp++;
                }
                PRSITE_DEBUGP("str_version = %s\n",str_version);

                if(!check_version_white_list(str_version,g_browser_list[i].version_num))
                {
                    return 0;
                }
            }
            PRSITE_DEBUGP("key_type %s is not in list\n",g_browser_list[i].key_type);
        }
    }
    return 1;
}


static void checkrandtimeout(time_t tv)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;

    struct st_randinfo* listret  = NULL;

    list_for_each_safe(listp, listn, &randomlist)
    {
        listret = list_entry(listp, struct st_randinfo, randlist);
        if ((tv - listret->ts) >= RANDTIMEOUT)
        {
            list_del(listp);
            kfree(listret);
        }
    }
    return;
}
    

void delrandall(void)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_randinfo* listret  = NULL;
    
    spin_lock_bh(&prsiterand_lock);
    list_for_each_safe(listp, listn, &randomlist)
    {
        listret = list_entry(listp, struct st_randinfo, randlist);
        list_del(listp);
        kfree(listret);
    }
    spin_unlock_bh(&prsiterand_lock);
    return;
}


void delrandbyid(char *pid)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_randinfo* listret  = NULL;
    __be32 id = simple_strtoul(pid, NULL, 10);
    printk("delrandbyid %u\n",id);
    spin_lock_bh(&prsiterand_lock);
    list_for_each_safe(listp, listn, &randomlist)
    {
        listret = list_entry(listp, struct st_randinfo, randlist);
        if(listret->id == id)
        {    
            printk("delete rand %u\n",id);
            list_del(listp);
            kfree(listret);
        }
    }
    spin_unlock_bh(&prsiterand_lock);
    return;
}


int showrandall(char *buffer, size_t len)
{
    struct list_head* listp  = NULL;
    struct list_head* listn  = NULL;
    struct st_randinfo* listret  = NULL;
    struct timespec ts; 
    int retlen = 0;
    memset(&ts, 0, sizeof(ts));
    do_posix_clock_monotonic_gettime(&ts);
    spin_lock_bh(&prsiterand_lock);
    list_for_each_safe(listp, listn, &randomlist)
    {
        listret = list_entry(listp, struct st_randinfo, randlist);
        if (((ts.tv_sec - listret->ts) < RANDTIMEOUT)
            && ((len - retlen) > RANDLENGTH))
        {
            retlen += sprintf(buffer + retlen, "%u\n", listret->id);
        }
    }
    spin_unlock_bh(&prsiterand_lock);
    return retlen;
}

static __be32 getrandomnum(void)
{
    struct st_randinfo *prandominfo = NULL;
    struct timespec ts; 
    memset(&ts, 0, sizeof(ts));
    do_posix_clock_monotonic_gettime(&ts);

    prandominfo = kmalloc(sizeof(struct st_randinfo), GFP_ATOMIC);
    if (NULL == prandominfo)
    {
        return 0;
    }
    get_random_bytes(&prandominfo->id, sizeof(__be32));
    prandominfo->ts = ts.tv_sec;
    spin_lock_bh(&prsiterand_lock);
    checkrandtimeout(ts.tv_sec);
    list_add(&prandominfo->randlist,&randomlist);
    spin_unlock_bh(&prsiterand_lock);
    return prandominfo->id;
}
#if defined(CONFIG_FORCE_APP)
static int http_changeto_appurl(struct sk_buff *pskb, char *url)
{
    char http_newdata[HTTP_RESPONSE_BUF_MAX + 1] = {0};

    PRSITE_DEBUGP("http_changeto_newurl ======>\n");
    snprintf(http_newdata,HTTP_RESPONSE_BUF_MAX + 1,
        "%s\r\nContent-length: 0\r\nLocation: http://%s\r\nConnection: Close\r\n\r\n",
        "HTTP/1.1 307 Temporary Redirect",g_stPrsiteUrlInfo.ac_stb_url);

    PRSITE_DEBUGP("HTTP ALG: new http response Redirect Success.\n");

    return http_set_appdata(pskb, http_newdata, strlen(http_newdata));
}
#endif

static int http_changeto_newurl(struct sk_buff *pskb, char *url)
{
    char http_newdata[HTTP_RESPONSE_BUF_MAX + 1] = {0};
    /*
    HTTP/1.1 302 Redirection\r\n
    Content-length: 0\r\n
    Location: http://192.168.1.1/html/update.html?updataredirect=10.14.10.153/onlineupdate/\r\n
    Connection: Close\r\n\r\n
     */
    //snprintf(http_newdata,HTTP_RESPONSE_BUF_MAX + 1,
    //    "%s\r\nContent-length: 0\r\nLocation: http://%s?updataredirect=%s\r\nConnection: Close\r\n\r\n",
    //    "HTTP/1.1 302 Redirection",g_stPrsiteUrlInfo.ac_stb_url,url);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    if ((pskb->mark & ACCESS_SSID2_MARK) == ACCESS_SSID2_MARK)
    {
        PRSITE_DEBUGP("http_changeto_newurl ssid2======>\n");
        snprintf(http_newdata,HTTP_RESPONSE_BUF_MAX + 1,
            "%s\r\nContent-length: 0\r\nLocation: http://%s?randid=%u?updataredirect=%s\r\nConnection: Close\r\n\r\n",
            "HTTP/1.1 307 Temporary Redirect",g_stPrsiteUrlInfo.ac_stb_url_ssid2,getrandomnum(),url);
    }
    else
    {
#endif
        PRSITE_DEBUGP("http_changeto_newurl ======>\n");
            snprintf(http_newdata,HTTP_RESPONSE_BUF_MAX + 1,
            "%s\r\nContent-length: 0\r\nLocation: http://%s?randid=%u?updataredirect=%s\r\nConnection: Close\r\n\r\n",
            "HTTP/1.1 307 Temporary Redirect",g_stPrsiteUrlInfo.ac_stb_url,getrandomnum(),url);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    }
#endif

    PRSITE_DEBUGP("HTTP ALG: new http response Redirect Success.\n");

    return http_set_appdata(pskb, http_newdata, strlen(http_newdata));
}


static int checkurlsuffix(char *purl)
{
    char *p = purl; 
    char *ptmp = NULL;
    if (0 == strcmp("/",purl))
    {
        PRSITE_DEBUGP("checkurlsuffix+++++++++++++++\n");
        return 0;
    }
    else
    {
        ptmp = strrchr(p,'.');
        if(NULL == ptmp)
        {
            return 1;
        }
        else
        {
            ptmp += 1;
            if(0 == strcmp(ptmp,"jsp")
               || 0 == strcmp(ptmp,"asp")
               || 0 == strcmp(ptmp,"htm")
               || 0 == strcmp(ptmp,"xhtml")
               || 0 == strcmp(ptmp,"aspx")
               || 0 == strcmp(ptmp,"php")
               || 0 == strcmp(ptmp,"cgi"))
            {
                return 0;
            }
            else
            {
                if(0 == strcmp(ptmp,"html"))
                {
                    if(strstr(purl, "library/test/success"))
                    {
                        return 1;
                    }
                    else
                    {
                        return 0;
                    }
                }
                return 1;
            }
        }
    }
}


static int checkblacksuffix(char *purl)
{
    char *p = purl; 
    int i = 0;
    char *ptmp = NULL;
    if (0 == strcmp("/",purl))
    {
        return 0;
    }
    else
    {
        ptmp = strrchr(p,'.');
        if(NULL == ptmp)
        {
            return 0;
        }
        else
        {
            ptmp += 1;
            for(i = 0; i< sizeof(g_blackurlsuffix)/sizeof(g_blackurlsuffix[0]); i++)
            {
                if (0 == strcmp(ptmp,g_blackurlsuffix[i]))
                {
                    PRSITE_DEBUGP("+++++++++++++++blacksuffix %s ignore!!\n",g_blackurlsuffix[i]);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static unsigned int http_prsite_info(unsigned char *pdata,unsigned char *purl,int useflag)
{
    unsigned char *p = NULL;
    unsigned char *ptmp = NULL;
    char uri[224]  = {0}; 
    char user_agent[224] = {0};   
    char host[64] = {0};
    int i = 0 ;
    p = pdata + 4;
    while (*p != ' ')
    {
        if (i >= 223)
        {
            return 1;
        }        
        uri[i] = *p;
        i++;
        p++;    
    }
    PRSITE_DEBUGP("uri %s\n", uri);
    if (strstr(uri,IGNORESTRING))
    {
        return 1;
    }


#ifndef BSP_CONFIG_BOARD_401HW
#ifndef BSP_CONFIG_BOARD_506HW
#ifndef BSP_CONFIG_BOARD_506HW_2
    if (checkblacksuffix(uri))
    {
        return 1;
    }
#endif
#endif
#endif

    if (2 == useflag)
    {
        if(checkurlsuffix(uri))
        {
            PRSITE_DEBUGP("checkurlsuffix not need======>\n");
            return 1;
        }
    }
    ptmp = strstr(p, "\r\nHost: "); 
    if (NULL == ptmp) 
    {
        PRSITE_DEBUGP("no HOST\n");
        return 1; 
    }
    ptmp = ptmp + strlen("\r\nHost: "); 
    i = 0;  
    while ((*ptmp != '\r') && (*ptmp != '\n'))
    {
        if (i >= 63)     
        {
            return 1; 
        }
        host[i] = *ptmp;   
        i++;
        ptmp++; 
    }
    PRSITE_DEBUGP("host %s\n", host);  
    ptmp = NULL;   
    ptmp = strstr(p, "\r\nUser-Agent: ");  
    if (NULL == ptmp) 
    {
        PRSITE_DEBUGP("User-Agent not need======>\n");
        return 1;     
    }
    ptmp = ptmp + strlen("\r\nUser-Agent: "); 

    i = 0;   
    while ((*ptmp != '\r') && (*ptmp != '\n'))  
    {
        if (i >= 223)  
        {
            user_agent[223] = 0;
            break;   
        }
        user_agent[i] = *ptmp;  
        i++;   
        ptmp++;
    }
     
    if(checkuser_agent_version(user_agent)) 
    {        
        PRSITE_DEBUGP("checkuser_agent,the boswer is not support======>\n");
        return 1; 
    }
    if (0 == strcmp("/",uri))   
    {
        snprintf(purl, HTTP_URL_MAX,"%s", host);   
    }    
    else  
    {     
        snprintf(purl, HTTP_URL_MAX,"%s%s", host, uri); 
    } 
    return 0;
}



unsigned int nf_prsite_in(struct sk_buff *skb, const struct net_device *pin, const struct net_device *pout)
{
    struct iphdr *iph = NULL;
    struct tcphdr *tcph = NULL;
    
    unsigned char url[HTTP_URL_MAX] = {0};
    int useflag = g_stPrsiteUrlInfo.lEnable;
    char data[DATALENGTH] = {0};
#if defined(CONFIG_FORCE_APP)
    struct ethhdr *peth = NULL;
#endif

#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    /*对ssid2向网关的访问进行过滤 如果是访问的强制门户网页则修改mark让其能访问*/
    if (((skb->mark & ACCESS_SSID2_MARK) == ACCESS_SSID2_MARK) && (1 == g_ssid2_checkurl))
    {
        iph = ip_hdr(skb);

        if (NULL == iph)
        {
            return NF_ACCEPT;   
        }

        if (iph->frag_off & htons(IP_OFFSET))
        {
            return NF_ACCEPT;
        }

        if (iph->protocol != IPPROTO_TCP)
        {
            return NF_ACCEPT;   
        }

        if (iph->daddr == g_stPrsiteUrlInfo.ul_lan_addr)
        {
            tcph = (void *)iph + iph->ihl * 4;

            if (tcph->dest == htons(80))
            {
                PRSITE_DEBUGP("ssid2 skb->len %d - %d - %d = %d>\n", skb->len, tcph->doff * 4, iph->ihl * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
                if (tcph->doff * 4 + iph->ihl * 4 == skb->len)
                {
                    skb->mark = skb->mark & (~ACCESS_SSID2_MARK);
                    return NF_ACCEPT;
                }

                if (skb->len - tcph->doff * 4 - iph->ihl * 4 < DATALENGTH - 1)
                {
                    strncpy(data, (void *)tcph + tcph->doff * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
                }
                else
                {
                    return NF_ACCEPT;
                }

                if ((strstr(data,"startupredirectforssidb") != NULL) || (strstr(data,"limitspeedforssidb") != NULL) || (strstr(data,"foundnewversionforssidb") != NULL))
                {
                    PRSITE_DEBUGP("it is PRSITE request,so change mark\n");
                    skb->mark = skb->mark & (~ACCESS_SSID2_MARK);
                }   
            }
            return NF_ACCEPT;
        }
    }
#endif
        
    if (!useflag)
    {
        //PRSITE_STATIC_DEBUGP("prsite is not enable\n");
        return NF_ACCEPT;
    }
#if defined(CONFIG_FORCE_APP)
    if (PRSITE_TYPE_MAC == gMacSwitch)
    {
        //只有当前强制门户升级失效后才强制APP下载
        peth = eth_hdr(skb);
        
        if (NULL == peth)
        {
            return NF_ACCEPT;
        }
        
        if (!check_mac_match(peth->h_source))
        {
            return NF_ACCEPT;
        }
    }
#endif
    iph = ip_hdr(skb);

    if (NULL == iph)
    {
        return NF_ACCEPT;   
    }
    
    if (iph->frag_off & htons(IP_OFFSET))   
    {
        return NF_ACCEPT;
    }

    if (iph->protocol != IPPROTO_TCP)
    {
        return NF_ACCEPT;   
    }

    tcph = (void *)iph + iph->ihl * 4;

    if (tcph->dest != htons(80) && tcph->dest != htons(8080))
    {
        return NF_ACCEPT; 
    }
    
    if ((iph->saddr == htonl(0x7F000001))||(iph->daddr == htonl(0x7F000001)))
    {
        PRSITE_DEBUGP("lo accpet ======>\n");
        return NF_ACCEPT; 
    }
    
    if ((iph->saddr == g_stPrsiteUrlInfo.ul_lan_addr)
        ||(iph->daddr == g_stPrsiteUrlInfo.ul_lan_addr)) 
    {
        return NF_ACCEPT;   
    }   
    if ((iph->saddr & g_stPrsiteUrlInfo.ul_lan_mask) == (iph->daddr & g_stPrsiteUrlInfo.ul_lan_mask))  
    {
        //PRSITE_STATIC_DEBUGP("prsite ipaddr is local address(lan)\n");    
        return NF_ACCEPT;  
    }
    
    if (tcph->doff * 4 + iph->ihl * 4 == skb->len)
    {
        return NF_ACCEPT;
    }
    
    /*data = (void *)tcph + tcph->doff * 4;

    if (NULL == data)
    {
        return NF_ACCEPT;
    }*/

    if (strncmp((void *)tcph + tcph->doff * 4, HTTP_PROTO_GET, strlen(HTTP_PROTO_GET)) != 0)
    {
        return NF_ACCEPT;
    }

    PRSITE_DEBUGP("skb->len %d - %d - %d = %d>\n", skb->len, tcph->doff * 4, iph->ihl * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
    if (skb->len - tcph->doff * 4 - iph->ihl * 4 < DATALENGTH - 1)
    {
        strncpy(data, (void *)tcph + tcph->doff * 4, skb->len - tcph->doff * 4 - iph->ihl * 4);
    }
    else
    {
        return NF_ACCEPT;
    }

#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    /*白名单让其在强制门户下能访问购买流量的网站*/
    if (1 == g_whitelist_enable)
    {
        if (strstr(data,g_whitelist))
        {   
            PRSITE_DEBUGP("data in whitelist\n");
            return NF_ACCEPT;
        }
    }
#endif    
    
    if (http_prsite_info(data,url,useflag) == 1)
    {
        return NF_ACCEPT;
    }
#if defined(CONFIG_FORCE_APP)
    if (PRSITE_TYPE_MAC == gMacSwitch)
    {
        if (http_changeto_appurl(skb, url) == 1)
        {
            return NF_ACCEPT;
        }
        if (gdelmacenable)
        {
            del_from_maclist(peth->h_source);
            PRSITE_DEBUGP("delete mac after prsite.\n");
        }
        else
        {
            PRSITE_DEBUGP("dont delete mac after prsite.\n");
        }
    }
    else
    {
#endif
        if (http_changeto_newurl(skb, url) == 1)
        {
            return NF_ACCEPT;
        }
#if defined(CONFIG_FORCE_APP)
    }

    PRSITE_DEBUGP("%d redirected!\n",__LINE__);
#endif
    /*重定向一次*/
    if (2 == useflag)
    {
        g_stPrsiteUrlInfo.lEnable = 0;
    }
    
    return NF_DROP;
}
EXPORT_SYMBOL_GPL(nf_prsite_in);

static void __exit nf_nat_http_fini(void)
{
    rcu_assign_pointer(prsite_add_hook, NULL);
    rcu_assign_pointer(prsite_del_hook, NULL);
    rcu_assign_pointer(prsite_ipconflict_hook, NULL);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW) \
 || defined(BSP_CONFIG_BOARD_506HW_2)
    rcu_assign_pointer(ssid2_checkurl_del_hook, NULL);
#endif
#if defined(CONFIG_FORCE_APP)
    rcu_assign_pointer(prsite_setmaclist_hook, NULL);

    free_maclist();
#endif
    synchronize_rcu();
}

static int __init nf_nat_http_init(void)
{
    INIT_LIST_HEAD(&randomlist);
#if defined(CONFIG_FORCE_APP)
    INIT_HLIST_HEAD(&hhlist);
#endif
    PRSITE_DEBUGP("nf_nat_http_init <*********** start ************>\n");
    BUG_ON(rcu_dereference(prsite_add_hook) != NULL);
    BUG_ON(rcu_dereference(prsite_del_hook) != NULL);
    BUG_ON(rcu_dereference(prsite_ipconflict_hook) != NULL);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW)
    BUG_ON(rcu_dereference(ssid2_checkurl_del_hook) != NULL);
#endif
#if defined(CONFIG_FORCE_APP)
    BUG_ON(rcu_dereference(prsite_setmaclist_hook) != NULL);
#endif

    rcu_assign_pointer(prsite_add_hook, prsite_add);
    rcu_assign_pointer(prsite_del_hook, prsite_del);
    rcu_assign_pointer(prsite_ipconflict_hook, prsite_ipconflict);
#if defined(BSP_CONFIG_BOARD_401HW) || defined(BSP_CONFIG_BOARD_506HW)
    rcu_assign_pointer(ssid2_checkurl_del_hook, ssid2_checkurl_del);
#endif
#if defined(CONFIG_FORCE_APP)
    rcu_assign_pointer(prsite_setmaclist_hook, prsite_setmaclist);
#endif
    return 0;
}

module_init(nf_nat_http_init);
module_exit(nf_nat_http_fini);
