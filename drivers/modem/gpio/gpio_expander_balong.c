
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/gpio.h>
#include <linux/i2c/pca953x.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include "bsp_om.h"
#ifdef BSP_CONFIG_BOARD_E5_E5770s
#include "bsp_shared_ddr.h"

#define NEW_SUB_BOARD_MUSK        (0x02)
#define OLD_SUB_BOARD_ID          1
#define NEW_SUB_BOARD_ID          0
#endif
//#include "bsp_hardtimer.h"

#define GPIO_I2C_EXPANDER_ADP 0
#define GPIO_I2C_EXPANDER_NUM 2

#define  gpio_print_error(fmt, ...)    (bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_GPIO, "[gpio_ex]: <%s> <%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define  gpio_print_info(fmt, ...)     (bsp_trace(BSP_LOG_LEVEL_ERROR,  BSP_MODU_GPIO, "[gpio_ex]: "fmt, ##__VA_ARGS__))

int	gpio_i2c_expander_setup(struct i2c_client *client, unsigned gpio, unsigned ngpio, void *context);
int	gpio_i2c_expander_teardown(struct i2c_client *client, unsigned gpio, unsigned ngpio, void *context);

static struct pca953x_platform_data gpio_i2c_expander_data[GPIO_I2C_EXPANDER_NUM] = {
		{
        	.gpio_base = 128,
        	.invert = 0,
        	.irq_base = -1,
        	.context = NULL,
        	.setup = gpio_i2c_expander_setup,
        	.teardown = gpio_i2c_expander_teardown,
		},
#ifdef BSP_CONFIG_BOARD_E5_E5770s
        {/*v711 E5770s*/
            .gpio_base = 144,
            .invert = 0,
            .irq_base = -1,
            .context = NULL,
            .setup = gpio_i2c_expander_setup,
            .teardown = gpio_i2c_expander_teardown,
        },
#else
		{/*v711 UDP E5*/
        	.gpio_base = 136,
        	.invert = 0,
        	.irq_base = -1,
        	.context = NULL,
        	.setup = gpio_i2c_expander_setup,
        	.teardown = gpio_i2c_expander_teardown,
		},
#endif
#if 0 
		{  /* V711 E5 */
        	.gpio_base = 144,
        	.invert = 0,
        	.irq_base = -1,
        	.context = NULL,
        	.setup = gpio_i2c_expander_setup,
        	.teardown = gpio_i2c_expander_teardown,
		},
#endif

};

static struct i2c_board_info expander_board_info[GPIO_I2C_EXPANDER_NUM] = {
#ifdef BSP_CONFIG_BOARD_E5_E5770s
        {
            .type = "tca6416",
            .addr = 0x21,
            .platform_data = &gpio_i2c_expander_data[0],
        },
        {
            .type = "tca6416",
            .addr = 0x20,
            .platform_data = &gpio_i2c_expander_data[1],
        },
#else
		{
        	.type = "pca9574",
        	.addr = 0x21,
        	.platform_data = &gpio_i2c_expander_data[0],
		},

		{/* v711 UDP E5*/
        	.type = "pca9574",
        	.addr = 0x20,
        	.platform_data = &gpio_i2c_expander_data[1],
		},
#endif
#if 0
		{/*v711 E5*/
        	.type = "pca9555",
        	.addr = 0x24,
        	.platform_data = &gpio_i2c_expander_data[2],
		},
#endif
};

struct i2c_client *gpio_expander_client[GPIO_I2C_EXPANDER_NUM] = {0};

int	gpio_i2c_expander_setup(struct i2c_client *client, unsigned gpio, unsigned ngpio, void *context)
{
    int ret = 0;
    unsigned int i = 0;

    for(i = 0;i < ngpio;i++)
    {
        ret = gpio_request(gpio + i, "gpio_expander");
        if(ret < 0)
        {
            gpio_print_error( "gpio%d request is fail, ret = %d.\n", gpio + i, ret);
            return -1;
        }

    }
    
    return 0;
    
}

int	gpio_i2c_expander_teardown(struct i2c_client *client, unsigned gpio, unsigned ngpio, void *context)
{
    unsigned int i = 0;

    for(i = 0;i < ngpio;i++)
    {
        gpio_free(gpio + i);
    }

    return 0;
    
}

/* read gpio expander reg */
int gpio_expander_register_debug(unsigned int num, int reg)
{
	int ret;

    struct i2c_client *client = NULL;

    if(num >= GPIO_I2C_EXPANDER_NUM || (reg >= 8))
    {
        gpio_print_error("para is error, num = %d, reg = 0x%x.\n", num, reg);
        return -1;
    }

    client = gpio_expander_client[num];

	if (num < 2)
		ret = i2c_smbus_read_byte_data(client, reg);
	else
		ret = i2c_smbus_read_word_data(client, reg << 1);

	if (ret < 0)
    {
		gpio_print_error("failed reading register.\n");
		return ret;
	}

	gpio_print_info("reg(0x%x) is 0x%x.\n", reg, ret);
    
	return 0;
}

#if 0
/* just for v7r2 test */
void gpio_i2c_expander_reset(void)
{

    int ret = 0;
    
    ret = gpio_request(GPIO_1_24, "gpio_expander");
    if(ret < 0)
    {
        gpio_print_error( "gpio_request is fail, ret = %d.\n", ret);
        return ;
    }
    
    gpio_direction_output(GPIO_1_24, 0);
    
    mdelay(10);
    
    gpio_direction_output(GPIO_1_24, 1);
    
}
#endif

/*i2c gpio expander*/
static int __devinit gpio_i2c_expander_init(void)
{
    unsigned int i = 0;
#ifdef BSP_CONFIG_BOARD_E5_E5770s
    u8 *pu32subId = (u8 *)(SHM_MEM_PMU_NPREG_ADDR + SHM_MEM_PMU_NPREG_SIZE - 1);
#endif

	struct i2c_adapter *adapter = NULL;

    //gpio_i2c_expander_reset();/* just for v7r2 test*/

	adapter = i2c_get_adapter(GPIO_I2C_EXPANDER_ADP);
	if (!adapter)
    {
		gpio_print_error("i2c_get_adapter failed.\n");
		return -1;
	}
    
    for(i = 0;i < GPIO_I2C_EXPANDER_NUM;i++)
    {
    	gpio_expander_client[i] = i2c_new_device(adapter, &expander_board_info[i]);
    	if (!gpio_expander_client[i]) {
    		gpio_print_error("i2c_new_device failed, i = %d, addr = 0x%x.\n", i, expander_board_info[i].addr);
    		return -1;
    	}
    }

#if 0
    ret = i2c_register_board_info(GPIO_I2C_EXPANDER_ADP, expander_board_info, sizeof(expander_board_info)/sizeof(struct i2c_board_info));
    if(ret < 0)
    {
        gpio_print_error("i2c_register_board_info is fail,ret = %d.\n", ret);
        return -1;
    }
#endif

	gpio_print_info("gpio ex init ok.\n");

#if (FEATURE_ON == MBB_CHG_APORT_EXTCHG)
#ifdef BSP_CONFIG_BOARD_E5_E5770s
    if (OLD_SUB_BOARD_ID == ((NEW_SUB_BOARD_MUSK & *pu32subId) >> 1))
    {
        gpio_request(OTG_ON_CTRL_GPIO, "OTG_ON_CTRL_GPIO");
        gpio_direction_output(OTG_ON_CTRL_GPIO, 1);
    }
    else
    {
        gpio_request(OTG_ON_CTRL_GPIO, "OTG_ON_CTRL_GPIO");
        gpio_direction_output(OTG_ON_CTRL_GPIO, 0);
    }
#else
    gpio_request(OTG_ON_CTRL_GPIO, "OTG_ON_CTRL_GPIO");
    gpio_direction_output(OTG_ON_CTRL_GPIO, 1);
#endif
#endif

	return 0;
}


static void __exit gpio_i2c_expander_exit(void)
{

}


subsys_initcall(gpio_i2c_expander_init);
module_exit(gpio_i2c_expander_exit);
MODULE_DESCRIPTION("balong GPIO I2C Expander device.");

