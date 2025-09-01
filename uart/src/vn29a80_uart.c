/******************************************************************************
 * @file      vn29a80_uart.c
 * @brief     Triển khai phần tử vn29a80_uart của counter_vn29a80.ko
 * @author    vietnam-engineer
 *****************************************************************************/

#include "vn29a80.h"

#define VN29A80_BAUD_RATE 38400

#define VN29A80_SEND_TIMEOUT msecs_to_jiffies(50)
#define VN29A80_RECV_TIMEOUT msecs_to_jiffies(50)

/**
 * @brief Gửi yêu cầu tới bộ đếm vn29a80
 * @param this Đối tượng của lớp vn29a80_uart
 * @param buf Bộ đệm chứa bản tin yêu cầu
 * @param len Kích thước của bản tin yêu cầu (tính theo bytes)
 * @return Nếu bản tin yêu cầu được gửi thành công, và
 * bản tin phản hồi được nhận từ vn29a80 trước hạn chót,
 * hàm này sẽ trả về 0. Nếu không thì trả về một số âm.
 * @note Hàm này sẽ chạy trong luồng được tạo ra bởi user space.
 * Sau khi gửi bản tin yêu cầu, luồng thực thi sẽ bị tạm dừng
 * cho đến khi nhận được bản tin phản hồi, hoặc quá hạn nhận.
 */
s32 vn29a80_uart_send_req(vn29a80_uart *this, const u8 *buf, u32 len)
{
	struct serdev_device *serdev = this->serdev;
	struct device *dev = &serdev->dev;
	s32 tmp;

	tmp = serdev_device_write(serdev, buf, len, this->send_timeout);
	if (tmp < (s32)len) {
		dev_err(dev, "%s write, error %d\n", __func__, tmp);
		return (tmp < 0) ? tmp : -EIO;
	}

	reinit_completion(&this->response_ready);
	tmp = wait_for_completion_interruptible_timeout(&this->response_ready,
							this->recv_timeout);
	if (tmp <= 0) {
		dev_err(dev, "%s wait, error %d\n", __func__, tmp);
		return (tmp < 0) ? tmp : -ETIMEDOUT;
	}

	return 0;
}

/**
 * @brief Nhận bản tin phản hồi từ bộ đếm vn29a80
 * @param serdev Một đối tượng của lớp serdev_device được truyền bởi TTY driver subsystem
 * @param buf Bộ đệm chứa bản tin phản hồi
 * @param size Số byte trong bộ đệm đang chờ được đẩy vào bộ đệm thô của vn29a80_res
 * @return Số byte đã được đẩy vào bộ đệm thô của vn29a80_res
 * @note Hàm này sẽ chạy trong luồng được tạo bởi TTY driver subsystem.
 * @note Nếu toàn bộ bản tin phản hồi đã được nhận, hàm này sẽ đánh thức luồng gửi yêu cầu.
 */
static s32 vn29a80_uart_recv_res(struct serdev_device *serdev, const u8 *buf, size_t size)
{
	struct device *dev = &serdev->dev;
	vn29a80_drv *drv = dev_get_drvdata(dev);
	vn29a80_res *res = &drv->res;
	vn29a80_uart *uart = &drv->uart;

	size = vn29a80_res_set_raw(res, buf, size);

	if (vn29a80_res_parse_raw(res)) {
		complete(&uart->response_ready);
	}

	return (s32)(size);
}

/* Đơn đăng ký sử dụng cổng UART */
static struct serdev_device_ops vn29a80_uart_ops = {
	.receive_buf = vn29a80_uart_recv_res,
	.write_wakeup = serdev_device_write_wakeup,
};

/**
 * @brief Hàm tạo của lớp vn29a80_uart.
 * @param this Đối tượng của lớp vn29a80_uart
 * @param dev Thông tin thiết bị
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_uart_setup(vn29a80_uart *this, struct device *dev)
{
	this->baudrate = VN29A80_BAUD_RATE;
	this->parity = SERDEV_PARITY_EVEN;
	this->serdev_ops = &vn29a80_uart_ops;
	this->send_timeout = VN29A80_SEND_TIMEOUT;
	this->recv_timeout = VN29A80_RECV_TIMEOUT;
	return uart_register(this, dev);
}

/**
 * @brief Hàm hủy của lớp vn29a80_uart.
 * @param this Đối tượng của lớp vn29a80_uart
 */
void vn29a80_uart_cleanup(vn29a80_uart *this)
{
}