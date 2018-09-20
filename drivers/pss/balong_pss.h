#ifndef __BALONG_PSS_H
#define __BALONG_PSS_H


/*
 * BALONG SNAPSHOT driver data - see drivers/pss/balong_pss.c
 */


#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/spinlock_types.h>

struct pss_regs {
	int reg_offset;
	int sp_offset;
};

struct pss {
	struct list_head list;
	struct kprobe kp;
	unsigned int seq;
	
	unsigned int flags;
#define PSS_ENABLED		BIT(0)
#define PSS_PARA_STACK	BIT(1)
#define PSS_PARA_REG	BIT(2)
#define PSS_PARA_CUST	BIT(3)

	struct pss_regs arg;

	unsigned int count;
};
struct pss_record {
	/* with these two variables, the actual process scenario could be determined */
	unsigned long probe_addr; 
	unsigned long ret_addr;
	unsigned int timestamp;
	__be16  ipid;/* valid only for ipv4, set 0 for ipv6 */
	__be16 l3num;
	__be32 saddr;
	__be32 daddr;/* the last four bytes of the ip addr for IPv6 */
	unsigned int tcphdr[4]; /* the first 16 bytes of the tcphdr */
};

struct pss_flush_ops {
	unsigned int mtu;
	unsigned int (*pss_buffer_flush)(unsigned int rd_offset, 
		unsigned int wr_offset, char *buf, unsigned int size, unsigned int max);
};


struct pss_man {
	struct dentry *root;
	struct list_head list;
	struct timer_list timer;
	int 				first_record;
	int 				first_pss;
	struct net_device *wdev; /*wan device*/
	struct net_device *ldev; /*wan device*/
	__be32 dstip;
	spinlock_t lock;
	struct pss_record *records;
	unsigned int max; /* the maxium buffered record num */
	unsigned int cur_r; /* current read pointer of the buffer */
	unsigned int cur_w; /* current write pointer of the buffer */
	unsigned int full;
	unsigned int time_out;
	spinlock_t rw_lock;
	unsigned int seqnum;
	struct pss_flush_ops *ops;
	unsigned int debug;
#define PSS_DBG_TIME  BIT(0)
#define PSS_DBG_TCP  BIT(1)
#define PSS_DBG_UDP  BIT(2)
#define PSS_DBG_ICMP  BIT(3)

	unsigned int flags;
#define PSS_BUFFER_ROLLOVER BIT(0)
#define PSS_BUFFER_PROCESS_BUSY BIT(1)


};

extern void pss_set_flush_ops(struct pss_flush_ops *ops);
extern struct pss_man g_pss_man;

#define PSS_RECORD_MAX 0x100
#define PSS_ARM_SP 13

#endif
