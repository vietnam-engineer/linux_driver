/******************************************************************************
 * @file      uart.c
 * @brief     Triển khai các APIs chung được sử dụng bởi nhiều UART device driver.
 * @author    vietnam-engineer
 *****************************************************************************/

#include <linux/of_device.h>

#include "uartdev_core.h"

u32 uart_get_send_timeout(struct uart *this)
{
	return jiffies_to_msecs(this->send_timeout);
}
EXPORT_SYMBOL(uart_get_send_timeout);

s32 uart_set_send_timeout(struct uart *this, u32 ms)
{
	unsigned long ticks = msecs_to_jiffies(ms);
	if (ticks == MAX_JIFFY_OFFSET) {
		dev_err(&this->serdev->dev, "%s invalid timeout (%u)\n", __func__, ms);
		return -EINVAL;
	}

	this->send_timeout = ticks;
	return 0;
}
EXPORT_SYMBOL(uart_set_send_timeout);

u32 uart_get_recv_timeout(struct uart *this)
{
	return jiffies_to_msecs(this->recv_timeout);
}
EXPORT_SYMBOL(uart_get_recv_timeout);

s32 uart_set_recv_timeout(struct uart *this, u32 ms)
{
	unsigned long ticks = msecs_to_jiffies(ms);
	if (ticks == MAX_JIFFY_OFFSET) {
		dev_err(&this->serdev->dev, "%s invalid timeout (%u)\n", __func__, ms);
		return -EINVAL;
	}

	this->recv_timeout = ticks;
	return 0;
}
EXPORT_SYMBOL(uart_set_recv_timeout);

/**
 * @brief Lấy các UART port settings từ device tree
 * @param this [in,out] đối tượng của lớp uart. Các thuộc tính của nó sẽ được cập nhật
 * @param dev [in] thông tin về thiết bị
 */
static void uart_parse_dt(struct uart *this, const struct device *dev)
{
	const struct device_node *np = dev->of_node;

	if (of_property_read_u32(np, "baudrate", &this->baudrate)) {
		dev_warn(dev, "use default baudrate %u\n", this->baudrate);
	}

	if (of_property_match_string(np, "parity", "none") >= 0) {
		this->parity = SERDEV_PARITY_NONE;
	} else if (of_property_match_string(np, "parity", "even") >= 0) {
		this->parity = SERDEV_PARITY_EVEN;
	} else if (of_property_match_string(np, "parity", "odd") >= 0) {
		this->parity = SERDEV_PARITY_ODD;
	} else {
		dev_warn(dev, "use default parity %u\n", this->parity);
	}

	this->flow_control = of_property_read_bool(np, "flow-control");
}

/**
 * @brief API này cho phép một UART device driver đăng ký
 * các serial operations với Linux kernel
 * @param this Bản đăng ký do UART device driver cung cấp
 * @param dev Thông tin mô tả thiết bị UART.
 * @return 0 nếu thành công. !0 nếu thất bại.
 * @note UART device driver cần phải triển khai các serdev operations.
 * @note API này nên được gọi từ hàm tạo của UART device driver.
 */
s32 uart_register(struct uart *this, struct device *dev)
{
	s32 err;

	this->serdev = to_serdev_device(dev);
	init_completion(&this->response_ready);

	/* Lấy thông tin từ device tree */
	uart_parse_dt(this, dev);

	/* cấu hình UART port của hệ nhúng */
	err = devm_serdev_device_open(dev, this->serdev);
	if (err) {
		dev_err(dev, "open serial device, error %d\n", err);
		return err;
	}
	serdev_device_set_baudrate(this->serdev, this->baudrate);
	serdev_device_set_flow_control(this->serdev, this->flow_control);
	serdev_device_set_parity(this->serdev, this->parity);

	/* đăng ký các callback để tiếp nhận các bản tin từ UART port */
	serdev_device_set_client_ops(this->serdev, this->serdev_ops);

	return 0;
}
EXPORT_SYMBOL(uart_register);

/**
 * @brief API này cho phép một UART device driver hủy đăng ký
 * các serial operations với Linux kernel
 * @param this đối tượng uart
 * @note Do devm_serdev_device_open được sử dụng trước đó, nên
 * Linux kernel sẽ tự động giải phóng UART port khi gỡ bỏ
 * UART device driver. Vì thế, không cần triển khai gì trong hàm này.
 * @note API này nên được gọi từ hàm hủy của UART device driver.
 */
void uart_unregister(struct uart *this)
{
}
EXPORT_SYMBOL(uart_unregister);
