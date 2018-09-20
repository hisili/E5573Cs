
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netlink.h>

#if (FEATURE_ON == MBB_CTF_COMMON)
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/fake/typedefs.h>
#include <linux/fake/osl.h>
#include <linux/fake/linux_osl.h>
#include <linux/fake/ctf/hndctf.h>
#endif /*  MBB_CTF_COMMON */


void mbb_usbnet_rx_set_mac_clone(struct sk_buff* skb);
void mbb_usbnet_tx_set_mac_clone(struct sk_buff* skb);

void mbb_usbnet_set_net_state(CRADLE_EVENT state);
void mbb_usbnet_set_last_net_state(CRADLE_EVENT state);
void mbb_usbnet_set_usb_state(USB_EVENT state);

void mbb_usbnet_net_state_notify(CRADLE_EVENT status);

void mbb_usbnet_usb_state_notify(USB_EVENT eventcode);

void mbb_usbnet_set_speed(CRADLE_SPEED speed);
int usb_lan_get_mac(char* eth_macAddr);
#if (FEATURE_ON == MBB_CTF_COMMON)

void mbb_usbnet_ctf_detach(ctf_t* ci, void* arg);
int mbb_usbnet_ctf_forward(struct sk_buff* skb);
void mbb_usbnet_ctf_enable(struct net_device* net);
void mbb_usbnet_ctf_disable(struct net_device* net);

#endif


