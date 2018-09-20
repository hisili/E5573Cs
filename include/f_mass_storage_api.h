
#ifndef __F_MASS_STORAGE_API_H__
#define __F_MASS_STORAGE_API_H__
#include "usb_debug.h"
#include "android.h"


USB_VOID usb_mass_storage_init(USB_VOID);

USB_VOID mass_function_add_lun(USB_CHAR* function_name, USB_CHAR* addname);

USB_VOID mass_storage_set_sd_card_workmode(USB_INT mode);

USB_INT  mass_storage_open_usb_sd(USB_VOID);


USB_INT mass_storage_close_usb_sd(USB_VOID);


USB_INT mass_storage_usb_sd_is_open(USB_VOID);


USB_VOID mass_storage_set_sd_card_status(USB_INT sd_removed);


USB_VOID mass_function_cleanup_lun_info(USB_CHAR* function_name);


USB_VOID mass_function_cleanup_alllun_info(USB_VOID);

#endif
