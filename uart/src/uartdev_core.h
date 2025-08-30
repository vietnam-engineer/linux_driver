/******************************************************************************
 * @file      uartdev_core.h
 * @brief     Khai báo các phần tử của uartdev_core kernel module (uartdev_core.ko)
 * @author    vietnam-engineer
 *****************************************************************************/

#ifndef __UARTDEV_CORE_H_
#define __UARTDEV_CORE_H_

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>

/**************************** class uif *******************************/
struct uif {
	/* giao diện char device file: sử dụng bởi userspace services/apps */

	u32 id;
	struct cdev cdev;
	struct device *dev;
	struct file_operations *cdev_fops; /* được cung cấp bởi UART device driver */
	atomic_t in_transaction;

	/* giao diện sysfs: sử dụng bởi kỹ sư phát triển device driver */

	struct attribute_group *attrs_group; /* được cung cấp bởi UART device driver */
};

s32 uif_register(struct uif *this, struct device *dev);
void uif_unregister(struct uif *this);


/**************************** class drv *******************************/

struct drv {
	struct class *class; /* trỏ tới một nhóm các thiết bị */
	dev_t devt; /* device number */
	struct ida ida; /* dùng cho mục đích cấp phát minor numbers */
};

struct class *drv_get_dev_class(void);
dev_t drv_get_dev_num(void);
s32 drv_alloc_minor(void);
void drv_remove_minor(u32 num);

#endif /* __UARTDEV_CORE_H_ */
