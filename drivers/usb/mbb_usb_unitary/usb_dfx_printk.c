/******************************************************************************

  Copyright (C), 2001-2014, Huawei Tech. Co., Ltd.

 ******************************************************************************
  File Name     : usb_dfx.c
  Version       : Initial Draft
  Author        : wangjuntao
  Created       : 2014/5/26
  Last Modified :
  Description   : usb_dfx.c
  Function List :
  History       :
  1.Date        : 2014/5/26
    Author      : wangjuntao
    Modification: Created file
******************************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/unaligned.h>
#include <linux/kallsyms.h>
#include <linux/kobject.h>
#include <asm/io.h>

#define USB_DFX_SYSFS_NAME                  "usb_dfx"
#define COMMA_SIGN      ','
#define BLANK_SIGN      ' '
#define MAX_NAME_LEN    256
#define MAX_VALUE_LEN   256
#define MAX_ARG_COUNT    7     //参数个数暂定5个
#define HEX_FLAG       "0x"

struct timer_list usb_timer;

/*****************调试代码************************/
int fastboot = 10; //调试变量
void func1( void )
{
    printk( KERN_INFO"********************* \n" );
    printk( KERN_INFO"*fastboot ---->%d* \n", fastboot );
    printk( KERN_INFO"********************* \n" );
}
void func2( int a, int b )
{
    printk( KERN_INFO"********************* \n" );
    printk( KERN_INFO"*a,b ---->%d,%d* \n", a, b );
    printk( KERN_INFO"********************* \n" );
}
/*****************************************/

typedef unsigned int ( *call_ptr )( int arg1, ... );
typedef unsigned int ( *call_void )( void );

typedef struct param
{
    char param_value[MAX_VALUE_LEN];
} ARG_INFO;

static ARG_INFO arg[MAX_ARG_COUNT];
static int value[MAX_ARG_COUNT] = {0};
static char timer_func_name[MAX_NAME_LEN] = {0};
static int  timer_param_num = 0;
static int  timer_value[MAX_ARG_COUNT] = {0};


static char* filter_blank( char* string )
{
    int len = 0;
    char* p = NULL;
    if ( NULL == string )
    {
        return NULL;
    }

    while ( *string == ' ' )
    {
        string++;
    }
    len = strlen( string );
    if ( len == 0 )
    {
        return NULL;
    }
    p = strchr( string, BLANK_SIGN );
    if ( NULL != p )
    {
        *p = '\0';
    }
    return string;
}
static void  usb_dfx_help( void )
{
    printk(KERN_INFO"************************ Help Informations **************************\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"* Value_set Node is used to set Global variable value. as:          *\n" );
    printk(KERN_INFO"* echo variable_name,value >>/sys/usb_dfx/value_set                 *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"* Func_call Node is used to Realizes the function call. as:         *\n" );
    printk(KERN_INFO"* echo func_name,value,... >>/sys/usb_dfx/func_call                 *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"* Func_timer_call node is used to Realizes the function call every  *\n" );
    printk(KERN_INFO"* time mins. as:                                                    *\n" );
    printk(KERN_INFO"* echo func_name,time,value,...>>/sys/class/usb_dfx/func_timer_call *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"* if you want to delete the func's time that had set, you can do as:*\n" );
    printk(KERN_INFO"* echo func_name,0,value,... >> /sys/class/usb_dfx/func_timer_call  *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"*                                                                   *\n" );
    printk(KERN_INFO"************************** Author ******************************\n" );

}

static int  transform_string_format( char* string )
{
    int nValude  = 0;

    if ( NULL == string )
    {
        printk( KERN_INFO"pls input the format as: <name,value> \n" );
        /*输入格式有误*/
        return  -1;
    }

    if ( NULL == strstr( string, HEX_FLAG ) )
    {
        sscanf( string, "%d", &nValude );
    }
    else
    {
        /*加入长度限制判断*/
        sscanf( string, "%x", &nValude );
    }
    return nValude;

}


static int usb_value_set_parse( const char* buf, size_t size )
{
    char name[MAX_NAME_LEN] = {0};
    char param_value[MAX_VALUE_LEN] = {0};
    char* p = NULL;  //保存分隔符
    int result = -1;
    int ip_addr = -1;
    int i = 0;
    u32 ip_value = 0;
    if (NULL == buf)
    {
        printk( KERN_INFO"pls input the format as: <name,value> \n" );
        return -1;
    }

    memset( name , 0 , sizeof( name ) - 1 );
    memset( param_value , 0 , sizeof( param_value ) - 1 );
    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( name, buf, size - 1 );
        if (0 == strcmp(filter_blank ( name ), "help"))
        {
            usb_dfx_help();
            return 0;
        }
        printk( KERN_INFO"pls input the format as: <name,value> \n" );
        return -1;
    }
    p = strchr( buf, COMMA_SIGN );
    strncpy( name, buf, p - buf );
    p++;
    strncpy( param_value, p, strlen( p ) ); //增加宏处理
    result = transform_string_format( filter_blank( param_value ) );
    if ( -1 != result )
    {
        printk( KERN_INFO"usb_value_set_parse --> <%s>: <%d> \n", name , result );
        if (NULL == strstr( name, HEX_FLAG ))
        {
            *( ( u32* )kallsyms_lookup_name( filter_blank ( name ) ) ) = result;
            printk( KERN_INFO"usb_value_set_parse --> <%s> ip_addr --> <%p> \n", name,
                    ( ( u32* )kallsyms_lookup_name( filter_blank ( name ) ) ));
        }
        else
        {

            ip_addr = transform_string_format( filter_blank ( name ));
            if (-1 != ip_addr)
            {
                for (i = 0 ; i <= result; i++)
                {
                    ip_value = readl(ip_addr + (i * 0x01));
                    printk( KERN_INFO"ip_value_%d --> <0x%08x> \n", i, ip_value );
                }
            }
        }
        return 0;
    }
    return -1;
}

/*****************************************************************
Parameters    :  usb_value_set
Return        :
Description   :  全局变量重新赋值
*****************************************************************/
static ssize_t usb_value_set( struct device* dev, struct device_attribute* attr,
                              const char* buf, size_t size )
{
    int err = -1;
    err = usb_value_set_parse( buf, size );
    if ( err < 0 )
    {
        printk( KERN_INFO"usb_value_set_parse err!\n" );
        return -EINVAL;
    }
    return size;
}

/*****************************************************************
Parameters    :  usb_func_call
Return        :
Description   :  唤醒函数
*****************************************************************/
static ssize_t usb_func_call( struct device* dev, struct device_attribute* attr,
                              const char* buf, size_t size )
{
    char func_name[MAX_NAME_LEN] = {0};
    int param_num = 1;
    call_ptr func_addr;
    call_void void_func_addr;
    char* p = NULL;  //保存分隔符
    char* q = NULL;  //保存空格符
    int i = 0;

    memset( func_name, 0, sizeof( func_name ) );
    memset( arg, 0, sizeof( arg ) );
    memset( value, 0, sizeof( value ) );

    if ( NULL == buf )
    {
        printk( KERN_INFO"pls input the format as: <func_name,value,...> \n" );
        return -EINVAL;
    }

    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( func_name, buf, size - 1 );
        if (0 == strcmp(func_name, "help"))
        {
            usb_dfx_help();
            return size;
        }
        void_func_addr = ( call_void ) kallsyms_lookup_name( filter_blank( func_name ) );
        if ( !void_func_addr )
        {
            printk( KERN_INFO"func_addr, value = %d\n", void_func_addr );
            return -EINVAL;
        }
        void_func_addr();
        return size;
    }
    p = strchr( buf, COMMA_SIGN );
    strncpy( func_name, buf, p - buf );
    p++;
    q = p;
    while ( strchr( q, COMMA_SIGN ) )
    {
        param_num++;
        q = strchr( q, COMMA_SIGN );
        q++;
    }

    //循环解析
    for ( i = 0 ; i < param_num; i++ )
    {
        q = strchr( p, COMMA_SIGN );
        if ( NULL != q )
        {
            strncpy( arg[i].param_value, p, q - p );
            value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
            p = q + 1;
        }
        else
        {
            strncpy( arg[i].param_value, p, strlen( p ) );
            value[i] = transform_string_format( filter_blank ( arg[i].param_value ) );
        }
        printk( KERN_INFO"param_value is %d\n", value[i] );
    }
    func_addr = ( call_ptr ) kallsyms_lookup_name( filter_blank ( func_name ) );
    if ( !func_addr )
    {
        printk( KERN_INFO"null addr func_addr\n");
        return -EINVAL;
    }
    switch ( param_num )
    {
        case 1:
            func_addr( value[0] );
            break;
        case 2:
            func_addr( value[0], value[1] );
            break;
        case 3:
            func_addr( value[0], value[1], value[2] );
            break;
        case 4:
            func_addr( value[0], value[1], value[2], value[3] );
            break;
        case 5:
            func_addr( value[0], value[1], value[2], value[3], value[4] );
            break;

    }
    return size;
}
/*****************************************************************
Parameters    :  timer_function_call
Return        :
Description   :  定时唤醒函数处理
*****************************************************************/
void timer_function_call( unsigned long data )
{
    call_ptr func_addr;
    call_void void_func_addr;
    int time = 0;
    time = timer_value[0];

    func_addr = ( call_ptr ) kallsyms_lookup_name( filter_blank ( timer_func_name ) );
    if ( !func_addr )
    {
        printk( KERN_INFO"func_addr, value = %d\n", func_addr );
        return ;
    }
    switch ( timer_param_num )
    {
        case 1:
            void_func_addr = ( call_void ) kallsyms_lookup_name( filter_blank ( timer_func_name ) );
            void_func_addr();
            break;
        case 2:
            func_addr( timer_value[1] );
            break;
        case 3:
            func_addr( timer_value[1], timer_value[2] );
            break;
        case 4:
            func_addr( timer_value[1], timer_value[2], timer_value[3] );
            break;
        case 5:
            func_addr( timer_value[1], timer_value[2], timer_value[3], timer_value[4] );
            break;
        case 6:
            func_addr( timer_value[1], timer_value[2], timer_value[3], timer_value[4], timer_value[5] );
            break;
    }
    mod_timer( &usb_timer, jiffies + ( time * HZ ) );
    return;
}

/*****************************************************************
Parameters    :  usb_timer_call
Return        :
Description   :  定时唤醒函数处理
*****************************************************************/
static ssize_t usb_timer_call( struct device* dev, struct device_attribute* attr,
                               const char* buf, size_t size )
{
    char* p = NULL;  //保存分隔符
    char* q = NULL;  //保存空格符
    call_void void_func_addr;
    int i = 0;
    int time = 0;
    memset( arg, 0, sizeof( arg ) );
    memset( timer_value, 0, sizeof( timer_value ) );
    memset( timer_func_name, 0, sizeof( timer_func_name ) );
    timer_param_num = 1;

    if ( NULL == buf )
    {
        printk( KERN_INFO"pls input the format as: <func_name,time,value,...> \n" );
        return -EINVAL;
    }

    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( timer_func_name, buf, size - 1 );
        if (0 == strcmp(timer_func_name, "help"))
        {
            usb_dfx_help();
            return size;
        }
        void_func_addr = ( call_void ) kallsyms_lookup_name( filter_blank( timer_func_name ) );
        if ( !void_func_addr )
        {
            printk( KERN_INFO"func_addr, value = %d\n", void_func_addr );
            return -EINVAL;
        }
        void_func_addr();
        return size;
    }
    p = strchr( buf, COMMA_SIGN );
    strncpy( timer_func_name, buf, p - buf );
    p++;
    q = p;
    while ( strchr( q, COMMA_SIGN ) )
    {
        timer_param_num++;
        q = strchr( q, COMMA_SIGN );
        q++;
    }
    for ( i = 0 ; i < timer_param_num; i++ )
    {
        q = strchr( p, COMMA_SIGN );
        if ( NULL != q )
        {
            strncpy( arg[i].param_value, p, q - p );
            timer_value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
            p = q + 1;
        }
        else
        {
            strncpy( arg[i].param_value, p, strlen( p ) );
            timer_value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
        }
    }
    time = timer_value[0];

    if ( 0 == time )
    {
        printk( KERN_INFO"del timer  %d\n", time );
        if (timer_pending( &usb_timer ) )
        {
            printk( KERN_INFO"*********** Timer Delete Successful ***********\n");
            del_timer( &usb_timer );
        }
        else
        {
            printk( KERN_INFO"<%s> hav't runing before ! can't del !\n", timer_func_name );
        }
        return -EINVAL;
    }
    if (timer_pending( &usb_timer ) )
    {
        printk( KERN_INFO"del the old timer and creat a new one  \n");
        del_timer( &usb_timer );
    }

    usb_timer.expires = jiffies + ( time * HZ );
    usb_timer.data = 0;
    usb_timer.function = timer_function_call;
    add_timer( &usb_timer );
    printk(KERN_INFO"*********** Begin To Start Timer ***********\n" );
    printk(KERN_INFO"*                                          *\n" );
    printk(KERN_INFO"***************** PLS Wait *****************\n" );

    return size;
}

static DEVICE_ATTR( value_set, S_IWUSR, NULL, usb_value_set );
static DEVICE_ATTR( func_call, S_IWUSR, NULL, usb_func_call );
static DEVICE_ATTR( func_timer_call, S_IWUSR, NULL, usb_timer_call );

static struct attribute* dev_attrs[] =
{
    &dev_attr_value_set.attr,
    &dev_attr_func_call.attr,
    &dev_attr_func_timer_call.attr,
    NULL,
};


static struct attribute_group dev_attr_grp =
{
    .attrs = dev_attrs,
};

/*****************************************************************
Parameters    :  usb_dfx_init
Return        :
Description   :  usb_dfx初始化
*****************************************************************/
static int __init usb_dfx_init( void )
{
    int res = -1;
    struct kobject* dev_kobj = NULL;
    struct kobject* usb_kobj = NULL;
    memset( arg , 0 , sizeof( arg ) - 1 );
    dev_kobj = kobject_create_and_add( USB_DFX_SYSFS_NAME, usb_kobj );
    res = sysfs_create_group( dev_kobj, &dev_attr_grp );
    if (res)
    {
        printk( KERN_INFO"----->sysfs_create_group err\n" );
        return res;
    }
    init_timer( &usb_timer );
    func1();
    func2( 1, 2 );
    return 0;

}

static void __exit usb_dfx_exit( void )
{
    del_timer( &usb_timer );
    printk( KERN_INFO"----->usb_dfx_exit\n" );
}

module_init( usb_dfx_init );
module_exit( usb_dfx_exit );
