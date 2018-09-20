
#include "u_usb_ctrl_sd.h"
#include "f_mass_storage_api.h"
#include "mbb_usb_adp.h"

#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
#include "hw_pnp.h"
#endif

#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
#include <linux/gpio.h>
extern usb_set_switch_direction(int value);
extern usb_get_switch_direction(void);
extern chg_stm_get_chgr_type(void);
int direction_flags = 0;
#endif
/*------------------------------------------------------------
  函数原型:virtualsd_ioctl()
  描述:
  输出:
  返回值: USB_INT
-------------------------------------------------------------*/
USB_INT32 virtualsd_ioctl(struct file* file, USB_UINT cmd, USB_UINT32 arg)
{
    USB_INT ret = 0;
    USB_INT blocknum = 0;
    USB_INT err = 0;
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
    USB_INT value, __user *temp;
    USB_INT direction = 0;
    chg_chgr_type_t cur_chgr_type = CHG_CHGR_INVALID;
#endif
#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
    USB_INT usb_switch = 0;
#endif
    if (_IOC_TYPE(cmd) != SDDEV_MAJOR)
    {
        return -EINVAL;
    }
    if (_IOC_NR(cmd) > SDDEV_IOC_MAXNR)
    {
        return -EINVAL;
    }
    /* 根据命令类型，检测参数空间是否可以访问 */
    if (_IOC_DIR(cmd) & _IOC_READ)
    { err = !access_ok(VERIFY_WRITE, (USB_PVOID)arg, _IOC_SIZE(cmd)); }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    { err = !access_ok(VERIFY_READ, (USB_PVOID)arg, _IOC_SIZE(cmd)); }
    if (err)
    {
        return -EFAULT;
    }

    switch (cmd)
    {
        case IOCTL_DISK_BLOCK_NUM:
            /*获取设备号接口函数*/
            blocknum = usb_get_mmc_dev_idex();
            if ( blocknum < 0)
            {
                DBG_E(MBB_MASS,
                      "ioctl usb_get_mmc_dev_idex error, get mmcblocknum = %d\n", blocknum);
                ret = -1;
                break;
            }
            ret = __put_user(blocknum, (USB_INT*)arg);
            break;
#ifdef USB_SD
        case IOCTL_MOUNT_DISK:
            mass_storage_set_sd_card_workmode(USB_MODE);
            ret = mass_storage_open_usb_sd();
            if ( ret < 0)
            {
                DBG_W(MBB_MASS, "can not open usb_sd \n");
            }
            break;
        case IOCTL_UNMOUNT_DISK:
            mass_storage_set_sd_card_workmode(WEB_MODE);
            break;
        case IOCTL_IS_MOUNTED_DISK:
            ret = mass_storage_usb_sd_is_open();
            if ( ret < 0)
            {
                DBG_W(MBB_MASS, "usb_sd is not open\n");
            }
            break;
#endif
#if(FEATURE_ON == MBB_USB_A_TO_MINI_SELECT)
        case IOCTL_IS_MOUNTED_UDISK:
            temp = (USB_INT *)arg;
            ret = __get_user(value, temp);
            if(0 == value)
            {
                direction_flags = 1;
            }
            ret = usb_set_switch_direction(value);
            if(ret < 0)
            {
                DBG_W(MBB_MASS, "set port direction fail\n");
            }
            break;
        case IOCTL_HOST_STATUS:
            cur_chgr_type = chg_stm_get_chgr_type();
            if(CHG_USB_HOST_PC == cur_chgr_type)
            {
                value = 1;
            }
            else
            {
                value = 0;
            }
            ret = __put_user(value, (USB_INT *)arg);
            break;
        case IOCTL_PORT_STATUS:
            direction = usb_get_switch_direction();
            if(direction)
            {
                value = 1;
            }
            else
            {
                value = 0;
            }
            ret = __put_user(value, (USB_INT *)arg);
            break;
#endif
#if(FEATURE_ON == MBB_USB_FTEN_SWITCH)
        case IOCTL_FTEN_USB_STATUS:
            usb_switch = get_ften_supply_type();

            ret = __put_user(usb_switch, (USB_INT*)arg);
            break;
#endif
        default:
            DBG_E(MBB_MASS, "virtualsd_ioctl cmd error\r\n");
            return -ENOTTY;
    }
    return ret;
}



static const struct file_operations virtualsd_ops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = virtualsd_ioctl,
};


static struct miscdevice virtualsd_fd =
{
    MISC_DYNAMIC_MINOR, //动态的设备号，系统选择
    SDDEV_NAME,
    &virtualsd_ops         //关联文件操作
};

/*------------------------------------------------------------
  函数原型:virtualsd_init()
  描述:
  输出:
  返回值:
-------------------------------------------------------------*/
static USB_INT  __init virtualsd_init(USB_VOID)
{
    USB_INT ret = 0;
    ret = misc_register(&virtualsd_fd);
    if (ret < 0)
    {
        DBG_E(MBB_MASS, "virtualsd_init: failed,ret = %x\r\n", ret);
    }

    return ret;
}
/*------------------------------------------------------------
  函数原型:virtualsd_exit()
  描述:
  输出:
  返回值:
-------------------------------------------------------------*/
static USB_VOID __exit virtualsd_exit(USB_VOID)
{
    USB_INT ret = 0;
    ret = misc_deregister(&virtualsd_fd);
    if (ret < 0)
    {
        DBG_E(MBB_MASS, "virtualsd_exit: failed,ret = %x\n", ret);
    }
    return;
}
module_init(virtualsd_init);
module_exit(virtualsd_exit);


