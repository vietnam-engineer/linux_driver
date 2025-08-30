/******************************************************************************
 * @file      drv.c
 * @brief     Khởi tạo uartdev_core kernel module (uartdev_core.ko)
 * @author    vietnam-engineer
 *****************************************************************************/

#include <linux/module.h>
#include <linux/slab.h>

#include "uartdev_core.h"

#define CLASS_NAME  "uartdev"
#define START_NUM   0
#define MAX_NUM_DEV 32
#define END_NUM	    (START_NUM + MAX_NUM_DEV - 1)

static struct drv *drv;

struct class *drv_get_dev_class(void)
{
	return drv->class;
}

dev_t drv_get_dev_num(void)
{
	return drv->devt;
}

s32 drv_alloc_minor()
{
	return ida_alloc_range(&drv->ida, START_NUM, END_NUM, GFP_KERNEL);
}

void drv_remove_minor(u32 num)
{
	ida_free(&drv->ida, num);
}

static int __init drv_init(void)
{
	int err;
	char *class_name = CLASS_NAME;

	/**
	 * Tạo ra một đối tượng của lớp drv.
	 */
	drv = kzalloc(sizeof(struct drv), GFP_KERNEL);
	if (!drv) {
		pr_err("%s: allocate mem error\n", class_name);
		return -ENOMEM;
	}

	/**
	 * Khởi tạo biến thành viên .class, đại diện cho một nhóm các thiết bị.
	 * Nếu thành công, thư mục /sys/class/uartdev sẽ được tạo ra
	 */
	drv->class = class_create(THIS_MODULE, class_name);
	if (IS_ERR(drv->class)) {
		err = PTR_ERR(drv->class);
		pr_err("%s: create class, error %d\n", class_name, err);
		goto err_class_create;
	}

	/**
	 * Khởi tạo biến thành viên .devt, một device number.
	 * Nếu thành công, Linux kernel sẽ cấp phát một major number cùng với
	 * 64 minor number, bắt đầu từ 0.
	 */
	err = alloc_chrdev_region(&drv->devt, START_NUM, MAX_NUM_DEV, class_name);
	if (err) {
		pr_err("%s: alloc dev numbers, error %d\n", class_name, err);
		goto err_alloc_chrdev;
	}

	/**
	 * Khởi tạo biến thành viên .ida, một ID Allocator.
	 * Sau này, các device driver sẽ gọi API drv_alloc_minor() để xin cấp một
	 * minor number trong khoảng từ 0 đến 63 nếu cần, và gọi API drv_remove_minor()
	 * để trả lại minor number đã được cấp phát trước đó nếu không cần nữa.
	 */
	ida_init(&drv->ida);

	pr_info("%s: %s done\n", CLASS_NAME, __func__);

	return 0;

err_alloc_chrdev:
	class_destroy(drv->class);

err_class_create:
	kfree(drv);

	return err;
}

static void __exit drv_exit(void)
{
	ida_destroy(&drv->ida);
	unregister_chrdev_region(drv->devt, MAX_NUM_DEV);
	class_destroy(drv->class);
	kfree(drv);
	pr_info("%s: %s done\n", CLASS_NAME, __func__);
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vietnam-engineer");
MODULE_DESCRIPTION("core module for UART device driver");
MODULE_VERSION("0.3.0");
