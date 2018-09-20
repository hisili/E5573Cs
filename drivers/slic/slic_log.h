#ifndef __SLIC_LOG_H__
#define __SLIC_LOG_H__

#define slic_debug(fmt,arg...) \
	printk("<slic debug> "fmt"\n",##arg)
#define slic_error(fmt,arg...) \
	printk("<slic error> "fmt"\n",##arg)

#endif /* __SLIC_LOG_H__ */
