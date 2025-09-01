/******************************************************************************
 * @file      vn29a80_uif.c
 * @brief     Triển khai phần tử vn29a80_uif của counter_vn29a80.ko
 * @author    vietnam-engineer
 *****************************************************************************/

#include "vn29a80.h"

/**
 * @brief Tạo và gửi bản tin yêu cầu đến vn29a80
 * @param this Đối tượng của lớp vn29a80_uif
 * @param cmd Mô tả lệnh của user space application
 * @return Nếu một bản tin được tạo ra, gửi đi thành công và nhận được phản hồi
 * thì hàm này sẽ trả về 0. Ngược lại, trả về một số âm.
 * @note Thread gọi hàm này sẽ bị tạm dừng hoạt động (đi ngủ) trong lúc chờ phản hồi.
 * @note Hàm này sẽ đảm bảo chắc chắc không có yêu cầu nào được gửi đi khi giao dịch
 * hiện tại vẫn chưa hoàn tất. -EBUSY sẽ được trả về nếu hàm này cũng đang được gọi
 * trong một thread khác.
 */
static s32 vn29a80_uif_create_and_send_cmd(vn29a80_uif *this, struct vn29a80_cmd *cmd)
{
	vn29a80_drv *drv = container_of(this, vn29a80_drv, uif);
	vn29a80_req *req = &drv->req;
	vn29a80_res *res = &drv->res;
	vn29a80_uart *uart = &drv->uart;
	struct vn29a80_msg msg;
	s32 err = 0;

	/* chỉ có một thread được phép giao dịch với thiết bị tại một thời điểm */
	if (atomic_cmpxchg(&this->in_transaction, 0, 1) != 0) {
		dev_err(this->dev, "%s busy\n", __func__);
		return -EBUSY;
	}

	/* tạo yêu cầu rồi gửi tới thiết bị và chờ phản hồi */
	err = vn29a80_req_create(req, cmd);
	if (0 == err) {
		msg = vn29a80_req_get(req);
		vn29a80_res_clear_raw(res);
		err = vn29a80_uart_send_req(uart, msg.buf, msg.len);
	}

	/* cho phép thread khác được phép giao dịch với thiết bị */
	atomic_set(&this->in_transaction, 0);

	return err;
}

/**
 * @brief Ghi trực tiếp một chuỗi ký tự vào vn29a80
 * @param dev Thông tin về thiết bị liên kết với `req_raw` attribute
 * @param attr Thông tin về sysfs attribute, bao gồm tên và quyền truy cập.
 * @param buf bản tin yêu cầu được viết vào sysfs file
 * @param count kích thước của bản tin (tính theo bytes)
 * @return Số byte đã gửi tới vn29a80 nếu thành công. Nếu có lỗi, trả về một số âm.
 * @note Thread gọi hàm này sẽ bị tạm dừng hoạt động (đi ngủ) trong lúc chờ phản hồi.
 * @note Hàm này sẽ đảm bảo chắc chắc không có yêu cầu nào được gửi đi khi giao dịch
 * hiện tại vẫn chưa hoàn tất. -EBUSY sẽ được trả về nếu hàm này cũng đang được gọi
 * trong một thread khác.
 */
static ssize_t req_raw_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_uart *uart = &drv->uart;
	vn29a80_res *res = &drv->res;
	s32 err = 0;

	if (atomic_cmpxchg(&uif->in_transaction, 0, 1) != 0) {
		dev_err(uif->dev, "%s busy\n", __func__);
		return -EBUSY;
	}

	vn29a80_res_clear_raw(res);
	err = vn29a80_uart_send_req(uart, buf, count);

	atomic_set(&uif->in_transaction, 0);

	if (err) {
		return err;
	}

	return count;
}
static ssize_t req_raw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_req *req = &drv->req;
	struct vn29a80_msg raw = vn29a80_req_get(req);
	s32 offset, i;

	for (i = 0, offset = 0; i < raw.len; ++i) {
		offset += snprintf(buf + offset, PAGE_SIZE - offset, "%c", raw.buf[i]);
	}

	return offset;
}
static DEVICE_ATTR_RW(req_raw);

static ssize_t res_raw_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_res *res = &drv->res;
	struct vn29a80_msg raw = vn29a80_res_get_raw(res);
	s32 offset, i;

	for (i = 0, offset = 0; i < raw.len; ++i) {
		offset += snprintf(buf + offset, PAGE_SIZE - offset, "%c", raw.buf[i]);
	}

	return offset;
}
static DEVICE_ATTR_RO(res_raw);

static ssize_t data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_res *res = &drv->res;
	struct vn29a80_data data = vn29a80_res_get_data(res);
	s32 offset = 0;

	offset +=
		snprintf(buf + offset, PAGE_SIZE - offset, "counter: %u\n", data.counter);
	offset += snprintf(buf + offset, PAGE_SIZE - offset,
			   "unix_time_ns: %llu, sys_uptime_ns: %llu\n", data.unix_time_ns,
			   data.sys_uptime_ns);
	offset += snprintf(buf + offset, PAGE_SIZE - offset, "res_count: %u\n",
			   data.res_count);
	return offset;
}
static DEVICE_ATTR_RO(data);

static ssize_t timeout_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_uart *uart = &drv->uart;
	s32 offset = 0;

	offset += snprintf(buf + offset, PAGE_SIZE - offset, "sending timeout: %u ms\n",
			   uart_get_send_timeout(uart));
	offset += snprintf(buf + offset, PAGE_SIZE - offset, "response timeout: %u ms\n",
			   uart_get_recv_timeout(uart));
	return offset;
}

static ssize_t timeout_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	vn29a80_uif *uif = dev_get_drvdata(dev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);
	vn29a80_uart *uart = &drv->uart;
	u32 send_timeout, recv_timeout;
	s32 err = 0;

	err = sscanf(buf, "%u:%u", &send_timeout, &recv_timeout);
	if (err != 2) {
		dev_err(dev, "%s get params, error %d\n", __func__, err);
		return -ENOEXEC;
	}

	err = uart_set_send_timeout(uart, send_timeout);
	if (err)
		return err;

	err = uart_set_recv_timeout(uart, recv_timeout);
	if (err)
		return err;

	return count;
}
static DEVICE_ATTR_RW(timeout);

static struct attribute *vn29a80_uif_attrs[] = {
	&dev_attr_req_raw.attr, /* /sys/class/uartdev/counter1/req_raw */
	&dev_attr_res_raw.attr, /* /sys/class/uartdev/counter1/res_raw */
	&dev_attr_data.attr, /* /sys/class/uartdev/counter1/data */
	&dev_attr_timeout.attr, /* /sys/class/uartdev/counter1/timeout */
	NULL,
};

static struct attribute_group vn29a80_uif_attrs_group = {
	.attrs = vn29a80_uif_attrs,
};

static s32 vn29a80_uif_cdev_open(struct inode *inode, struct file *file)
{
	vn29a80_uif *uif = container_of(inode->i_cdev, vn29a80_uif, cdev);
	vn29a80_drv *drv = container_of(uif, vn29a80_drv, uif);

	file->private_data = drv;

	return 0;
}

static s32 vn29a80_uif_cdev_close(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t vn29a80_uif_cdev_read(struct file *file, char __user *ubuf, size_t len,
				     loff_t *off)
{
	vn29a80_drv *drv = file->private_data;
	vn29a80_uif *uif = &drv->uif;
	vn29a80_res *res = &drv->res;
	struct vn29a80_cmd cmd;
	struct vn29a80_data data;
	s32 err;

	if (len < sizeof(data)) {
		dev_err(uif->dev, "%s ubuf too small\n", __func__);
		return -ENOMEM;
	}

	cmd.id = CMD_ID_GET_COUNTER;
	err = vn29a80_uif_create_and_send_cmd(uif, &cmd);
	if (err == 0) {
		data = vn29a80_res_get_data(res);
		if (copy_to_user(ubuf, &data, sizeof(data))) {
			err = -EFAULT;
		}
	} else {
		dev_err(uif->dev, "%s error %d\n", __func__, err);
	}

	return err;
}

static ssize_t vn29a80_uif_cdev_write(struct file *file, const char __user *ubuf,
				      size_t len, loff_t *off)
{
	vn29a80_drv *drv = file->private_data;
	vn29a80_uif *uif = &drv->uif;
	struct vn29a80_cmd cmd;
	s32 err;

	if (len < sizeof(cmd)) {
		dev_err(uif->dev, "%s ubuf too small\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(&cmd, ubuf, sizeof(cmd)) == 0) {
		err = vn29a80_uif_create_and_send_cmd(uif, &cmd);
	} else {
		dev_err(uif->dev, "%s error %d\n", __func__, err);
		err = -EFAULT;
	}

	return err;
}

/**
 * @brief IOCTL entry point của char device file
 * @param file char device file đã được mở
 * @param code mã IOCTL
 * @param arg tham số IOCTL
 * @return 0 nếu thành công. !0 nếu xảy ra lỗi
 */
static long vn29a80_uif_cdev_ioctl(struct file *file, u32 code, unsigned long arg)
{
	vn29a80_drv *drv = file->private_data;
	vn29a80_uif *uif = &drv->uif;
	vn29a80_uart *uart = &drv->uart;
	long ret = 0;
	s32 err;

	switch (code) {
	case DEV_SET_SEND_TIMEOUT: {
		ret = uart_set_send_timeout(uart, arg);
	} break;

	case DEV_GET_SEND_TIMEOUT: {
		u32 timeout = uart_get_send_timeout(uart);
		err = copy_to_user((void __user *)arg, &timeout, sizeof(timeout));
		if (err) {
			dev_err(uif->dev, "DEV_GET_SEND_TIMEOUT, error %d\n", err);
			ret = -EFAULT;
		}
	} break;

	case DEV_SET_RECV_TIMEOUT: {
		ret = uart_set_recv_timeout(uart, arg);
	} break;

	case DEV_GET_RECV_TIMEOUT: {
		u32 timeout = uart_get_recv_timeout(uart);
		err = copy_to_user((void __user *)arg, &timeout, sizeof(timeout));
		if (err) {
			dev_err(uif->dev, "DEV_GET_RECV_TIMEOUT, error %d\n", err);
			ret = -EFAULT;
		}
	} break;

	default: {
		dev_err(uif->dev, "unknown IOCTL code %u\n", code);
		ret = -ENOEXEC;
	} break;
	}

	return ret;
}

static struct file_operations vn29a80_uif_cdev_fops = {
	.owner = THIS_MODULE,
	.open = vn29a80_uif_cdev_open,
	.release = vn29a80_uif_cdev_close,
	.read = vn29a80_uif_cdev_read,
	.write = vn29a80_uif_cdev_write,
	.unlocked_ioctl = vn29a80_uif_cdev_ioctl,
};

/**
 * @brief Hàm tạo của lớp vn29a80_uif.
 * @param this Đối tượng của lớp vn29a80_uif
 * @param dev Thông tin thiết bị
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_uif_setup(vn29a80_uif *this, struct device *dev)
{
	this->cdev_fops = &vn29a80_uif_cdev_fops;
	this->attrs_group = &vn29a80_uif_attrs_group;
	return uif_register(this, dev);
}

/**
 * @brief Hàm hủy của lớp vn29a80_uif.
 * @param this Đối tượng của lớp vn29a80_uif
 */
void vn29a80_uif_cleanup(vn29a80_uif *this)
{
	uif_unregister(this);
}
