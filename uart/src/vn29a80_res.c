/******************************************************************************
 * @file      vn29a80_res.c
 * @brief     Triển khai phần tử vn29a80_res của counter_vn29a80.ko
 * @author    vietnam-engineer
 *****************************************************************************/

#include <asm/unaligned.h>

#include "vn29a80.h"

/*                Định dạng bản tin phản hồi
 *+------+---+------------+---+---------------------+---+---+
 *|"$RES"|':'|ID (2 bytes)|':'|Các tham số (n bytes)|':'|'@'|
 *+------+---+------------+---+---------------------+---+---+
 *|<-------------------- n + 10 bytes --------------------->|
 */

#define RES_START_FRAME "$RES"
#define RES_SEPARATOR	':'
#define RES_END_FRAME	"@"

#define RES_HEADER_SIZE		 (sizeof(RES_START_FRAME) - 1)
#define RES_SEPARATOR_SIZE	 1
#define RES_ID_SIZE		 2
#define RES_PARAM_SIZE_GET_COUNT 8
#define RES_PARAM_SIZE_SET_COUNT 0
#define RES_FOOTER_SIZE		 (sizeof(RES_END_FRAME) - 1)
#define RES_MIN_SIZE		 10

#define OFS_ID	  (RES_HEADER_SIZE + RES_SEPARATOR_SIZE)
#define OFS_PARAM (OFS_ID + RES_ID_SIZE + RES_SEPARATOR_SIZE)

void vn29a80_res_clear_raw(vn29a80_res *this)
{
	mutex_lock(&this->lock);
	this->msg.len = 0;
	mutex_unlock(&this->lock);
}

size_t vn29a80_res_set_raw(vn29a80_res *this, const u8 *ser_buf, size_t sz)
{
	size_t len = min_t(size_t, sz, VN29A80_MSG_LEN - this->msg.len);
	mutex_lock(&this->lock);
	memcpy(this->msg.buf + this->msg.len, ser_buf, len);
	this->msg.len += len;
	mutex_unlock(&this->lock);
	return len;
}

/**
 * @brief Phân tích bản tin phản hồi từ vn29a80
 * @param this Đối tượng của lớp vn29a80_res
 * @return true nếu bản tin được phân tích thành công.
 * @note Thuật toán phân tích bản tin dưới đây không đủ mạnh để xử lý các
 * tình huống phức tạp.
 */
bool vn29a80_res_parse_raw(vn29a80_res *this)
{
	s32 err;
	u32 hdr_idx; /* vị trí của phần đầu đề trong this->msg.buf */
	u8 msg_id; /* định danh của bản tin */
	u32 param_size; /* kích thước phần tham số của bản tin */
	u32 footer_idx; /* vị trí của phần kết thúc trong this->msg.buf */
	u8 bytes[4]; /* lưu các giá trị tạm thời khi phân tích dữ liệu */
	u64 now_ns = ktime_get(); /* hiện tại, tính theo CLOCK_MONOTONIC */

	mutex_lock(&this->lock);

	/* Tìm vị trí của phần đầu đề  */
	for (hdr_idx = 0; this->msg.len >= hdr_idx + RES_MIN_SIZE; ++hdr_idx) {
		if (!strncmp(this->msg.buf + hdr_idx, RES_START_FRAME, RES_HEADER_SIZE)) {
			break;
		}
	}

	if (this->msg.len < hdr_idx + RES_MIN_SIZE) {
		mutex_unlock(&this->lock);
		return false;
	}

	/* Xác định số bytes của phần tham số */
	err = hex2bin(&msg_id, this->msg.buf + hdr_idx + OFS_ID, sizeof(msg_id));
	if (err) {
		dev_err(this->dev, "%s can't convert to binary\n", __func__);
		this->msg.len = 0;
		mutex_unlock(&this->lock);
		return false;
	} else if (msg_id == CMD_ID_GET_COUNTER) {
		param_size = RES_PARAM_SIZE_GET_COUNT;
	} else if (msg_id == CMD_ID_SET_COUNTER) {
		param_size = RES_PARAM_SIZE_SET_COUNT;
	}

	if (this->msg.len < hdr_idx + RES_MIN_SIZE + param_size) {
		mutex_unlock(&this->lock);
		return false;
	}

	/* Tìm vị trí của phần kết thúc */
	footer_idx = hdr_idx + RES_MIN_SIZE + param_size - RES_FOOTER_SIZE;
	if (strncmp(this->msg.buf + footer_idx, RES_END_FRAME, RES_FOOTER_SIZE)) {
		dev_err(this->dev, "%s found unexpected EOF\n", __func__);
		this->msg.len = 0;
		mutex_unlock(&this->lock);
		return false;
	}

	this->data.res_count += 1;

	/* Lấy dữ liệu */
	err = hex2bin(bytes, this->msg.buf + hdr_idx + OFS_PARAM, param_size);
	if (err == 0) {
		this->data.counter = get_unaligned_be32(bytes);
		dev_info(this->dev, "%s counter = %u\n", __func__, this->data.counter);
	}

	/* Sinh các dữ liệu của driver */
	this->data.sys_uptime_ns = now_ns;
	this->data.unix_time_ns = ktime_mono_to_real(now_ns);

	mutex_unlock(&this->lock);

	return true;
}

struct vn29a80_msg vn29a80_res_get_raw(vn29a80_res *this)
{
	struct vn29a80_msg raw = {};
	mutex_lock(&this->lock);
	raw = this->msg;
	mutex_unlock(&this->lock);
	return raw;
}
struct vn29a80_data vn29a80_res_get_data(vn29a80_res *this)
{
	struct vn29a80_data data = {};
	mutex_lock(&this->lock);
	data = this->data;
	mutex_unlock(&this->lock);
	return data;
}

/**
 * @brief Hàm tạo của lớp vn29a80_res.
 * @param this Đối tượng của lớp vn29a80_res
 * @param dev Thông tin thiết bị
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_res_setup(vn29a80_res *this, struct device *dev)
{
	this->msg.len = 0;
	this->dev = dev;
	mutex_init(&this->lock);

	return 0;
}

/**
 * @brief Hàm hủy của lớp vn29a80_res.
 * @param this Đối tượng của lớp vn29a80_res
 */
void vn29a80_res_cleanup(vn29a80_res *this)
{
	mutex_destroy(&this->lock);
}