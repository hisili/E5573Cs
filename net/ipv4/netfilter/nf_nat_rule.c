/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/* Everything about the rules for NAT. */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <net/checksum.h>
#include <net/route.h>
#include <linux/bitops.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_nat_rule.h>

#if defined(CONFIG_ATP_ROUTE) && defined(CONFIG_IP_NF_TARGET_MASQUERADE)
#include <linux/inetdevice.h>
#endif

#define NAT_VALID_HOOKS ((1 << NF_INET_PRE_ROUTING) | \
			 (1 << NF_INET_POST_ROUTING) | \
			 (1 << NF_INET_LOCAL_OUT) | \
			 (1 << NF_INET_LOCAL_IN))

static const struct xt_table nat_table = {
	.name		= "nat",
	.valid_hooks	= NAT_VALID_HOOKS,
	.me		= THIS_MODULE,
	.af		= NFPROTO_IPV4,
};

/* Source NAT */
static unsigned int
ipt_snat_target(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	NF_CT_ASSERT(par->hooknum == NF_INET_POST_ROUTING ||
		     par->hooknum == NF_INET_LOCAL_IN);

	ct = nf_ct_get(skb, &ctinfo);

	/* Connection must be valid and new. */
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED ||
			    ctinfo == IP_CT_RELATED_REPLY));
	NF_CT_ASSERT(par->out != NULL);

#if defined(CONFIG_ATP_ROUTE) && defined(CONFIG_IP_NF_TARGET_MASQUERADE)
	struct nf_conn_nat *nat = nfct_nat(ct);
	if (nat)
	{
		nat->masq_index = par->out->ifindex;
	}
#endif

	return nf_nat_setup_info(ct, &mr->range[0], NF_NAT_MANIP_SRC);
}

static unsigned int
ipt_dnat_target(struct sk_buff *skb, const struct xt_action_param *par)
{
	struct nf_conn *ct;
	enum ip_conntrack_info ctinfo;
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	NF_CT_ASSERT(par->hooknum == NF_INET_PRE_ROUTING ||
		     par->hooknum == NF_INET_LOCAL_OUT);

	ct = nf_ct_get(skb, &ctinfo);

	/* Connection must be valid and new. */
	NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

	return nf_nat_setup_info(ct, &mr->range[0], NF_NAT_MANIP_DST);
}

static int ipt_snat_checkentry(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	/* Must be a valid range */
	if (mr->rangesize != 1) {
		pr_info("SNAT: multiple ranges no longer supported\n");
		return -EINVAL;
	}
	return 0;
}

static int ipt_dnat_checkentry(const struct xt_tgchk_param *par)
{
	const struct nf_nat_ipv4_multi_range_compat *mr = par->targinfo;

	/* Must be a valid range */
	if (mr->rangesize != 1) {
		pr_info("DNAT: multiple ranges no longer supported\n");
		return -EINVAL;
	}
	return 0;
}

static unsigned int
alloc_null_binding(struct nf_conn *ct, unsigned int hooknum)
{
	/* Force range to this IP; let proto decide mapping for
	   per-proto parts (hence not NF_NAT_RANGE_PROTO_SPECIFIED).
	*/
	struct nf_nat_ipv4_range range;

	range.flags = 0;
	pr_debug("Allocating NULL binding for %p (%pI4)\n", ct,
		 HOOK2MANIP(hooknum) == NF_NAT_MANIP_SRC ?
		 &ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3.ip :
		 &ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3.ip);

	return nf_nat_setup_info(ct, &range, HOOK2MANIP(hooknum));
}

int nf_nat_rule_find(struct sk_buff *skb,
		     unsigned int hooknum,
		     const struct net_device *in,
		     const struct net_device *out,
		     struct nf_conn *ct)
{
	struct net *net = nf_ct_net(ct);
	int ret;

	ret = ipt_do_table(skb, hooknum, in, out, net->ipv4.nat_table);

	if (ret == NF_ACCEPT) {
		if (!nf_nat_initialized(ct, HOOK2MANIP(hooknum)))
			/* NUL mapping */
			ret = alloc_null_binding(ct, hooknum);
	}
	return ret;
}

static struct xt_target ipt_snat_reg __read_mostly = {
	.name		= "SNAT",
	.target		= ipt_snat_target,
	.targetsize	= sizeof(struct nf_nat_ipv4_multi_range_compat),
	.table		= "nat",
	.hooks		= (1 << NF_INET_POST_ROUTING) | (1 << NF_INET_LOCAL_IN),
	.checkentry	= ipt_snat_checkentry,
	.family		= AF_INET,
};

static struct xt_target ipt_dnat_reg __read_mostly = {
	.name		= "DNAT",
	.target		= ipt_dnat_target,
	.targetsize	= sizeof(struct nf_nat_ipv4_multi_range_compat),
	.table		= "nat",
	.hooks		= (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_OUT),
	.checkentry	= ipt_dnat_checkentry,
	.family		= AF_INET,
};

static int __net_init nf_nat_rule_net_init(struct net *net)
{
	struct ipt_replace *repl;

	repl = ipt_alloc_initial_table(&nat_table);
	if (repl == NULL)
		return -ENOMEM;
	net->ipv4.nat_table = ipt_register_table(net, &nat_table, repl);
	kfree(repl);
	if (IS_ERR(net->ipv4.nat_table))
		return PTR_ERR(net->ipv4.nat_table);
	return 0;
}

static void __net_exit nf_nat_rule_net_exit(struct net *net)
{
	ipt_unregister_table(net, net->ipv4.nat_table);
}

static struct pernet_operations nf_nat_rule_net_ops = {
	.init = nf_nat_rule_net_init,
	.exit = nf_nat_rule_net_exit,
};

#if defined(CONFIG_ATP_ROUTE) && defined(CONFIG_IP_NF_TARGET_MASQUERADE)
static int
device_cmp(struct nf_conn *i, void *ifindex)
{
	const struct nf_conn_nat *nat = nfct_nat(i);

	if (!nat)
		return 0;

	return nat->masq_index == (int)(long)ifindex;
}

static int masq_device_event(struct notifier_block *this,
			     unsigned long event,
			     void *ptr)
{
	const struct net_device *dev = ptr;
	struct net *net = dev_net(dev);

	if (event == NETDEV_DOWN) {
		/* Device was downed.  Search entire table for
		   conntracks which were associated with that device,
		   and forget them. */
		NF_CT_ASSERT(dev->ifindex != 0);
		nf_ct_iterate_cleanup(net, device_cmp,
				      (void *)(long)dev->ifindex);
	}

	return NOTIFY_DONE;
}

static int masq_inet_event(struct notifier_block *this,
			   unsigned long event,
			   void *ptr)
{
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;
	return masq_device_event(this, event, dev);
}

static struct notifier_block masq_dev_notifier = {
	.notifier_call	= masq_device_event,
};

static struct notifier_block masq_inet_notifier = {
	.notifier_call	= masq_inet_event,
};
#endif

int __init nf_nat_rule_init(void)
{
	int ret;

	ret = register_pernet_subsys(&nf_nat_rule_net_ops);
	if (ret != 0)
		goto out;
	ret = xt_register_target(&ipt_snat_reg);
	if (ret != 0)
		goto unregister_table;

	ret = xt_register_target(&ipt_dnat_reg);
	if (ret != 0)
		goto unregister_snat;

#if defined(CONFIG_ATP_ROUTE) && defined(CONFIG_IP_NF_TARGET_MASQUERADE)
	/* Register for device down reports */
	register_netdevice_notifier(&masq_dev_notifier);
	/* Register IP address change reports */
	register_inetaddr_notifier(&masq_inet_notifier);
#endif

	return ret;

 unregister_snat:
	xt_unregister_target(&ipt_snat_reg);
 unregister_table:
	unregister_pernet_subsys(&nf_nat_rule_net_ops);
 out:
	return ret;
}

void nf_nat_rule_cleanup(void)
{
	xt_unregister_target(&ipt_dnat_reg);
	xt_unregister_target(&ipt_snat_reg);
	unregister_pernet_subsys(&nf_nat_rule_net_ops);
#if defined(CONFIG_ATP_ROUTE) && defined(CONFIG_IP_NF_TARGET_MASQUERADE)
	unregister_netdevice_notifier(&masq_dev_notifier);
	unregister_inetaddr_notifier(&masq_inet_notifier);
#endif

}
