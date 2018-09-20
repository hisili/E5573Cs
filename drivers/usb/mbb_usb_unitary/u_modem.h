#ifndef _U_MODEM_H
#define _U_MODEM_H


#define ACM_MODEM_PREFIX             "acm_"
#define ACM_MODEM_DRV_NAME           "acm_modem"
#define ACM_MODEM_QUEUE_SIZE         16
#define ACM_MODEM_DFT_RD_BUF_SIZE    1536
#define ACM_MODEM_DFT_RD_REQ_NUM     8
#define ACM_MODEM_DFT_WT_REQ_NUM     256

#define ACM_MODEM_NAME_MAX   64




int gacm_modem_setup(struct usb_gadget *g, unsigned count);
void gacm_modem_cleanup(void);

int gacm_modem_line_state(struct gserial *gser, u32 state);
int gacm_modem_suspend(struct gserial *gser);
int gacm_modem_resume(struct gserial *gser);
int gacm_modem_connect(struct gserial *gser, u8 port_num);
void gacm_modem_disconnect(struct gserial *gser);
void usb_modem_init(void);
void usb_modem_exit(void);

#endif
