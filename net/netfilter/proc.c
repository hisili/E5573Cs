

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/namei.h>
#include <linux/if.h>
#include <asm/uaccess.h>

#include <linux/stat.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/mount.h>
#include <linux/pid_namespace.h>
#include <linux/parser.h>
#ifdef CONFIG_PRSITE 
#include <linux/netfilter/nf_conntrack_prsite.h>
#endif


#define ATP_PROC_PRSITE_DIR_NAME        "prsite"
#define ATP_PROC_PRSITE_DIR             ATP_PROC_DIR"/prsite"

#define ATP_PROC_PRSITE_RAND_FILE       "rand"
#if defined(CONFIG_FORCE_APP)
#define PROC_PRSITE_MACSWITCH_FILE      "macswitch"
#endif

#define ATP_PROC_PRSITE_DATA_RAND           4
#if defined(CONFIG_FORCE_APP)
#define ATP_PROC_PRSITE_DATA_MACSWITCH      5
#endif

#define PRSITE_BUFF_LEN_MAX             64

static struct proc_dir_entry *g_AtpProcPrsiteDirEntry = NULL;

static struct proc_dir_entry *g_AtpProcPrsiteRandEntry = NULL;
#if defined(CONFIG_FORCE_APP)
static struct proc_dir_entry *g_AtpProcPrsiteMacSwitchEntry = NULL;
extern int gMacSwitch;
#endif

extern struct prsite_url_info g_stPrsiteUrlInfo;

#define ATP_PROC_DEBUG(msg, ...)    printk(KERN_DEBUG " [%s] [%d] [%s] "msg"\r\n", __FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define ATP_PROC_ERROR(msg, ...)    printk(KERN_ERR " [%s] [%d] [%s] "msg"\r\n", __FILE__, __LINE__,__FUNCTION__, ##__VA_ARGS__)
#define ATP_PROC_SAFE_PTR( ptr)     ( (ptr) ? (ptr) : "NULL")
#define ATP_PROC_FILE_VALUE_LENGTH             8  
#define ATP_PROC_DIR_NAME               "app_proc"
#define ATP_PROC_DIR                    "/proc/"ATP_PROC_DIR_NAME
static struct proc_dir_entry *g_AtpProcDirEntry = NULL;
typedef int (*ProcFileRead)(struct file *fp, char *buffer, size_t len, loff_t *offset);
typedef int (*ProcFileWrite)(struct file *fp, const char *userBuf, size_t len, loff_t *off);

#ifdef CONFIG_PRSITE 

int atp_proc_create_file(struct proc_dir_entry *dir_entry,
                         char *file,
                         struct proc_dir_entry **file_entry,
                         struct file_operations *pfile_proc_fops)
{
    ATP_PROC_DEBUG("will create file: %s", ATP_PROC_SAFE_PTR(file));

    if (NULL == pfile_proc_fops)
    {        
        ATP_PROC_DEBUG("pfile_proc_fops is NULL");
        return -1;    
    }

    if (NULL != dir_entry)
    {        
        *file_entry = proc_create(file, 0666, dir_entry, pfile_proc_fops);
        if (NULL != *file_entry)
        {
            return 0;
        }
        else
        {
            ATP_PROC_DEBUG("create_proc_entry failed");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("dir_entry null");
        return -1;
    }
}
static ssize_t atp_proc_prsite_read(struct file *fp, char *buffer, size_t len, loff_t *offset, int datatype) 
{
    char       tempbuff[HTTP_URL_MAX] = {0};
    int        ret = 0;

    switch (datatype)
    {
        case ATP_PROC_PRSITE_DATA_RAND:
            ret = showrandall(buffer, len);
            ATP_PROC_ERROR(" atp_proc_prsite_read buffer Read is: %s\n", buffer);
            return ret;
#if defined(CONFIG_FORCE_APP)
        case ATP_PROC_PRSITE_DATA_MACSWITCH:
            snprintf(tempbuff, sizeof(tempbuff), "%d", gMacSwitch);
            ATP_PROC_ERROR(" atp_proc_prsite_read buffer Read is: %s\n", buffer);
            break;
#endif
        default:
            break;
    }

    ret = strlen(tempbuff) + 1;
    if (copy_to_user(buffer, tempbuff, ret)) 
    {
        ATP_PROC_ERROR("copy_to_user failed: %s\n",tempbuff);
        return -1;
    }

    return ret;
}

static ssize_t atp_proc_prsite_write(struct file *fp, const char *userBuf, size_t len, loff_t *off, int datatype) 
{    
    char temp_data[PRSITE_BUFF_LEN_MAX] = {0};
    int  ret = 0;

    if ((NULL == userBuf) || (len > (PRSITE_BUFF_LEN_MAX - 1)))
    {
        ATP_PROC_ERROR("buffer is NULL or buflen is too long");
        return -1;
    }

    if (0 != copy_from_user(temp_data, userBuf, len))
    {
        ATP_PROC_ERROR("copy_from_user failed");
        return -1;
    }
    temp_data[PRSITE_BUFF_LEN_MAX-1] = 0;
    ret = strlen(temp_data);

    switch (datatype)
    {
        case ATP_PROC_PRSITE_DATA_RAND:
            delrandbyid(temp_data);
            break;
#if defined(CONFIG_FORCE_APP)
        case ATP_PROC_PRSITE_DATA_MACSWITCH:
            gMacSwitch = simple_strtoul(temp_data, NULL, 10);
            ATP_PROC_ERROR("Read from user, mac switch: [%d]", gMacSwitch);
            break;
#endif
        default:
            break;
    }
    ATP_PROC_ERROR("atp_proc_prsite_write: %s,  g_stPrsiteUrlInfo.lEnable:%d\n", temp_data,  g_stPrsiteUrlInfo.lEnable);    
    return ret;
}

static ssize_t atp_proc_prsite_Rand_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_Rand_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_RAND);
    return ret;
}



static ssize_t atp_proc_prsite_Rand_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;
    
    ATP_PROC_DEBUG("atp_proc_prsite_Rand_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_RAND);
    return ret;
}

#if defined(CONFIG_FORCE_APP)
static ssize_t atp_proc_prsite_MacSwitch_read(struct file *fp, char *buffer, size_t len, loff_t *offset)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_MacSwitch_read \n");
    ret = atp_proc_prsite_read(fp, buffer, len, offset, ATP_PROC_PRSITE_DATA_MACSWITCH);
    return ret;
}

static ssize_t atp_proc_prsite_MacSwitch_write(struct file *fp, const char *userBuf, size_t len, loff_t *off)
{
    int ret = 0;

    ATP_PROC_DEBUG("atp_proc_prsite_MacSwitch_write \n");
    ret = atp_proc_prsite_write(fp, userBuf, len, off, ATP_PROC_PRSITE_DATA_MACSWITCH);
    return ret;
}

struct file_operations g_file_proc_fops_prsite_MacSwitch = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_MacSwitch_read,
                           .write = atp_proc_prsite_MacSwitch_write,                           
                           }; 
#endif

struct file_operations g_file_proc_fops_prsite_Rand = {
                           .owner = THIS_MODULE,
                           .read = atp_proc_prsite_Rand_read,
                           .write = atp_proc_prsite_Rand_write,                           
                           }; 




int atp_proc_create_prsite(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_PRSITE_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !!!\n", ATP_PROC_PRSITE_DIR);
        g_AtpProcPrsiteDirEntry = proc_mkdir(ATP_PROC_PRSITE_DIR_NAME, g_AtpProcDirEntry);
        if (NULL == g_AtpProcPrsiteDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed ");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_PRSITE_DIR);
        return 0;
    }


    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         ATP_PROC_PRSITE_RAND_FILE,
                         &g_AtpProcPrsiteRandEntry,
                         &g_file_proc_fops_prsite_Rand);

#if defined(CONFIG_FORCE_APP)
    atp_proc_create_file(g_AtpProcPrsiteDirEntry,
                         PROC_PRSITE_MACSWITCH_FILE,
                         &g_AtpProcPrsiteMacSwitchEntry,
                         &g_file_proc_fops_prsite_MacSwitch);
#endif

    return 0;
}
#endif

int atp_proc_init(void)
{
    int error = 0;
    struct path proc_path;

    error = kern_path(ATP_PROC_DIR, LOOKUP_FOLLOW, &proc_path);
    if (error)
    {
        ATP_PROC_DEBUG("%s NOT exist !\n", ATP_PROC_DIR);
        g_AtpProcDirEntry = proc_mkdir(ATP_PROC_DIR_NAME, NULL);
        if (NULL == g_AtpProcDirEntry)
        {
            ATP_PROC_ERROR("proc_mkdir failed ");
            return -1;
        }
    }
    else
    {
        ATP_PROC_DEBUG("%s already exist !\n", ATP_PROC_DIR);
    }
#ifdef CONFIG_PRSITE
    atp_proc_create_prsite();
#endif
    return 0;
}


void atp_proc_deinit(void)
{
#ifdef CONFIG_PRSITE
    
      
    remove_proc_entry(ATP_PROC_PRSITE_RAND_FILE, g_AtpProcPrsiteRandEntry);    
    
    remove_proc_entry(ATP_PROC_PRSITE_DIR_NAME, g_AtpProcPrsiteDirEntry);
#endif

    remove_proc_entry(ATP_PROC_DIR_NAME, g_AtpProcDirEntry);
}


module_init(atp_proc_init)
module_exit(atp_proc_deinit)
MODULE_LICENSE("GPL");