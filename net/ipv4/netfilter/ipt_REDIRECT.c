/* Redirect.  Simple mapping which alters dst to a local IP address. */
/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <linux/inetdevice.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/x_tables.h>
#include <net/netfilter/nf_nat_rule.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: Connection redirection to localhost");

#ifdef CONFIG_ATP_ONDEMAND_DIAL
extern int g_report_time ;
enum RNIC_DAIL_EVENT_TYPE_ENUM
{
    RNIC_DAIL_EVENT_UP                  = 0x0600,                               /*需要触发拨号*/
    RNIC_DAIL_EVENT_DOWN                        ,                               /*需要断开拨号 */
    RNIC_DAIL_EVENT_TYPE_BUTT
};
extern int  g_atp_redirect_dailmode;
enum AT_RNIC_DIAL_MODE_ENUM
{
    AT_RNIC_DIAL_MODE_MANUAL, /*Manual dial mode*/
    AT_RNIC_DIAL_MODE_DEMAND_CONNECT, /*Demand dial up*/
    AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT, /*Demand dial down*/
    AT_RNIC_DIAL_MODE_BUTT
};
#endif

/* FIXME: Take multiple ranges --RR */
static int redirect_tg_check(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	if (mr->range[0].flags & NF_NAT_RANGE_MAP_IPS) {
		pr_debug("bad MAP_IPS.\n");
		return -EINVAL;
	}
	if (mr->rangesize != 1) {
		pr_debug("bad rangesize %u.\n", mr->rangesize);
		return -EINVAL;
	}
	return 0;
}
#ifdef CONFIG_ATP_ONDEMAND_DIAL
static void redirect_atp_send_ondemand_event(void)
{
    /* 系统启动后的时间*/
    struct timespec ts; 
    static struct timespec oldts = {0};
    int  ulRet = 0;
    int  ulSize   = sizeof(DEVICE_EVENT);
    DEVICE_EVENT    stEvent;
    stEvent.device_id     = DEVICE_ID_WAN;
    stEvent.event_code    = RNIC_DAIL_EVENT_UP;
    stEvent.len           = 0;    
    memset(&ts,0,sizeof(struct timespec));   
    /* 获取系统启动时间*/
    do_posix_clock_monotonic_gettime(&ts);
    /*控制包数，默认5s发一个*/
    if(ts.tv_sec - oldts.tv_sec >= g_report_time )
    {
        /*刷新oldts.tv_sec*/
        memcpy(&oldts,&ts,sizeof(struct timespec));
        /*上报netlink事件*/
        ulRet = device_event_report(&stEvent,(int)ulSize); 
        if (0 != ulRet)
        {
           printk("%s\n","balong SendDialEvent failed"); 
           return ;
        }
       printk("%s\n","syswatch_rnic_connect send ondemand event");
    }
}
#endif

static unsigned int
redirect_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	__be32 newdst;
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;
	struct nf_nat_ipv4_range newrange;

	NF_CT_ASSERT(par->hooknum == NF_INET_PRE_ROUTING ||
		     par->hooknum == NF_INET_LOCAL_OUT);

	ct = nf_ct_get(skb, &ctinfo);
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	/* Local packets: make them go to loopback */
	if (par->hooknum == NF_INET_LOCAL_OUT)
		newdst = htonl(0x7F000001);
	else {
		struct in_device *indev;
		struct in_ifaddr *ifa;

		newdst = 0;

		rcu_read_lock();
		indev = __in_dev_get_rcu(skb->dev);
		if (indev && (ifa = indev->ifa_list))
			newdst = ifa->ifa_local;
		rcu_read_unlock();

		if (!newdst)
			return NF_DROP;
	}

	/* Transfer from original range. */
	newrange = ((struct nf_nat_ipv4_range)
		{ mr->range[0].flags | NF_NAT_RANGE_MAP_IPS,
		  newdst, newdst,
		  mr->range[0].min, mr->range[0].max });

    /*判断是否需要上报按需拨号事件*/
#ifdef CONFIG_ATP_ONDEMAND_DIAL
    if(AT_RNIC_DIAL_MODE_DEMAND_DISCONNECT == g_atp_redirect_dailmode)
    {
        redirect_atp_send_ondemand_event();
    }
#endif 

	/* Hand modified range to generic setup. */
	return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}

static struct xt_target redirect_tg_reg __read_mostly = {
	.name		= "REDIRECT",
	.family		= NFPROTO_IPV4,
	.target		= redirect_tg,
	.targetsize	= sizeof(struct nf_nat_ipv4_multi_range_compat),
	.table		= "nat",
	.hooks		= (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_OUT),
	.checkentry	= redirect_tg_check,
	.me		= THIS_MODULE,
};

static int __init redirect_tg_init(void)
{
	return xt_register_target(&redirect_tg_reg);
}

static void __exit redirect_tg_exit(void)
{
	xt_unregister_target(&redirect_tg_reg);
}

module_init(redirect_tg_init);
module_exit(redirect_tg_exit);
