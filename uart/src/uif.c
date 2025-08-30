/******************************************************************************
 * @file      uif.c
 * @brief     Triển khai các APIs chung được sử dụng bởi nhiều UART device driver.
 * @author    vietnam-engineer
 *****************************************************************************/

#include <linux/of_device.h>

#include "uartdev_core.h"

/**
 * @brief API này cho phép một UART device driver đăng ký
 * device file operations và sysfs files với Linux kernel
 * @param this Bản đăng ký do UART device driver cung cấp
 * @param dev Thông tin mô tả thiết bị UART.
 * @return 0 nếu thành công. !0 nếu thất bại.
 * @note UART device driver cần phải triển khai các
 * device file operations và các sysfs files.
 * @note Nếu thành công, một character device file và
 * các sysfs files sẽ được tạo ra.
 * @note API này nên được gọi từ hàm tạo của UART device driver.
 */
s32 uif_register(struct uif *this, struct device *dev)
{
	s32 err = 0;
	s32 minor = 0;
	dev_t devt;

	/* tên của thiết bị giống với tên của node tương ứng trong device tree */
	const char *dev_name = dev->of_node->name;

	/* Xin cấp phát một minor number từ kho 64 số */
	minor = drv_alloc_minor();
	if (minor < 0) {
		err = minor;
		dev_err(dev, "allocate a minor number, error %d\n", err);
		goto err_alloc_minor;
	} else {
		this->id = minor;
	}

	/* Khởi tạo cdev (character device) */
	devt = MKDEV(MAJOR(drv_get_dev_num()), minor);
	cdev_init(&this->cdev, this->cdev_fops);
	this->cdev.owner = this->cdev_fops->owner;

	/* Đăng ký cdev với Linux kernel */
	err = cdev_add(&this->cdev, devt, 1);
	if (err) {
		dev_err(dev, "add char device [%d-%d], error %d\n",
			MAJOR(drv_get_dev_num()), minor, err);
		goto err_cdev_add;
	}

	/* Tạo ra một device file trong thư mục /dev */
	this->dev = device_create(drv_get_dev_class(), dev, devt, this, dev_name);
	if (IS_ERR(this->dev)) {
		err = PTR_ERR(this->dev);
		dev_err(dev, "create /dev/%s, error %d\n", dev_name, err);
		goto err_device_create;
	}

	/* Tạo ra các sysfs files trong thư mục /sys/class/uartdev/${dev_name}/ */
	err = sysfs_create_group(&this->dev->kobj, this->attrs_group);
	if (err) {
		dev_err(dev, "create sysfs files, error %d\n", err);
		goto err_sysfs_create;
	}

	return 0;

err_sysfs_create:
	device_destroy(this->dev->class, this->dev->devt);

err_device_create:
	cdev_del(&this->cdev);

err_cdev_add:
	drv_remove_minor(this->id);

err_alloc_minor:
	return err;
}
EXPORT_SYMBOL(uif_register);

/**
 * @brief API này cho phép một UART device driver hủy đăng ký
 * device file operations và sysfs files với Linux kernel
 * @param this đối tượng uif
 * @note sysfs files và character device file sẽ bị xóa đi.
 * @note API này nên được gọi từ hàm hủy của UART device driver.
 */
void uif_unregister(struct uif *this)
{
	sysfs_remove_group(&this->dev->kobj, this->attrs_group);

	device_destroy(this->dev->class, this->dev->devt);
	cdev_del(&this->cdev);
	drv_remove_minor(this->id);
}
EXPORT_SYMBOL(uif_unregister);
