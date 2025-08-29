/******************************************************************************
 * @file      uartdev_core.h
 * @brief     Khai báo các phần tử của uartdev_core kernel module (uartdev_core.ko)
 * @author    vietnam-engineer
 *****************************************************************************/

#ifndef __UARTDEV_CORE_H_
#define __UARTDEV_CORE_H_

#include <linux/device.h>

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
