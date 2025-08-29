/******************************************************************************
 * @file      drv.c
 * @brief     Khởi tạo uartdev_core kernel module (uartdev_core.ko)
 * @author    vietnam-engineer
 *****************************************************************************/

#include <linux/module.h>

#define CLASS_NAME  "uartdev"

static int __init drv_init(void)
{
	pr_info("%s: %s done\n", CLASS_NAME, __func__);

	return 0;
}

static void __exit drv_exit(void)
{
	pr_info("%s: %s done\n", CLASS_NAME, __func__);
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vietnam-engineer");
MODULE_DESCRIPTION("core module for UART device driver");
MODULE_VERSION("0.0.0");
