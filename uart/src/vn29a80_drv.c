/******************************************************************************
 * @file      vn29a80_drv.c
 * @brief     Khởi tạo vn29a80 driver (counter_vn29a80.ko)
 * @author    vietnam-engineer
 *****************************************************************************/
#include <linux/module.h>
#include <linux/of_device.h>

#include "vn29a80.h"

#define VN29A80_DRV_NAME "counter_vn29a80" /* tên driver */

/* Hàm này sẽ được gọi khi kernel thấy vn29a80 xuất hiện */
static int vn29a80_drv_setup(struct serdev_device *serdev)
{
	struct device *dev = &serdev->dev;
	vn29a80_drv *drv;
	s32 err = 0;

	dev_info(dev, "%s start\n", __func__);

	/* Tạo ra một đối tượng của lớp vn29a80_drv */
	drv = devm_kzalloc(dev, sizeof(*drv), GFP_KERNEL);
	if (!drv) {
		dev_err(dev, "allocate vn29a80_drv error\n");
		return -ENOMEM;
	}

	/* Liên kết đối tượng vn29a80_drv với serdev_device để sử dụng sau này */
	dev_set_drvdata(dev, drv);

	dev_info(dev, "%s finished\n", __func__);

	return err;
}

/* Hàm này sẽ được gọi khi driver bị dỡ bỏ khỏi kernel space */
static void vn29a80_drv_cleanup(struct serdev_device *serdev)
{
	struct device *dev = &serdev->dev;

	dev_info(dev, "%s start\n", __func__);

	dev_info(dev, "%s finished\n", __func__);
}

/* Liệt kê các thiết bị mà driver này có thể điều khiển */
static const struct of_device_id vn29a80_of_match[] = {
	{ .compatible = "hcm,vn29a80" },
	{},
};

/* Làm đơn đăng ký xin được điều khiển các thiết bị */
static struct serdev_device_driver vn29a80_driver = {
	.driver = {
		.name = VN29A80_DRV_NAME,
		.of_match_table = vn29a80_of_match,
	},
	.probe = vn29a80_drv_setup,
	.remove = vn29a80_drv_cleanup,
};

/* Gửi đơn đăng ký tới Linux kernel */
module_serdev_device_driver(vn29a80_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("vietnam-engineer");
MODULE_DESCRIPTION("UART device driver for a circular counter");
MODULE_VERSION("0.0.0");
