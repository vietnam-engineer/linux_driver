/******************************************************************************
 * @file      vn29a80_req.c
 * @brief     Triển khai phần tử vn29a80_req của counter_vn29a80.ko
 * @author    vietnam-engineer
 *****************************************************************************/

#include <linux/string.h>
#include <linux/kernel.h>

#include "vn29a80.h"

/*                Định dạng bản tin yêu cầu
 *+------+---+------------+---+---------------------+---+---+
 *|"$REQ"|':'|ID (2 bytes)|':'|Các tham số (n bytes)|':'|'@'|
 *+------+---+------------+---+---------------------+---+---+
 *|<-------------------- n + 10 bytes --------------------->|
 */

#define REQ_START_FRAME "$REQ"
#define REQ_SEPARATOR	':'
#define REQ_END_FRAME	"@"

#define REQ_HEADER_SIZE		 (sizeof(REQ_START_FRAME) - 1)
#define REQ_SEPARATOR_SIZE	 1
#define REQ_ID_SIZE		 2
#define REQ_PARAM_SIZE_GET_COUNT 0
#define REQ_PARAM_SIZE_SET_COUNT 8
#define REQ_FOOTER_SIZE		 (sizeof(REQ_END_FRAME) - 1)

struct vn29a80_msg vn29a80_req_get(vn29a80_req *this)
{
	struct vn29a80_msg msg;
	mutex_lock(&this->lock);
	msg = this->msg;
	mutex_unlock(&this->lock);
	return msg;
}

/**
 * @brief Tạo ra một bản tin yêu cầu để gửi tới bộ đếm vn29a80
 * @param this Đối tượng của lớp vn29a80_req
 * @param cmd Lệnh nhận được từ ứng dụng trên user space
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_req_create(vn29a80_req *this, const struct vn29a80_cmd *cmd)
{
	s32 ret = 0;
	u8 *ptr;

	/* Kiểm tra đầu vào */
	if (!cmd) {
		dev_err(this->dev, "%s NULL param\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&this->lock);

	/* Khởi tạo bộ đệm */
	this->msg.len = 0;

	/* Thêm đầu đề */
	ptr = this->msg.buf + this->msg.len;
	ret = strscpy(ptr, REQ_START_FRAME, VN29A80_MSG_LEN - this->msg.len);
	if (ret == REQ_HEADER_SIZE) {
		this->msg.len += REQ_HEADER_SIZE;
	} else {
		dev_err(this->dev, "%s failed to copy SOF: ret = %d\n", __func__, ret);
		mutex_unlock(&this->lock);
		return -EINVAL;
	}

	/* Thêm dấu ngăn cách */
	ptr = this->msg.buf + this->msg.len;
	*ptr = REQ_SEPARATOR;
	this->msg.len += REQ_SEPARATOR_SIZE;

	/* Thêm ID */
	ptr = this->msg.buf + this->msg.len;
	ptr = hex_byte_pack_upper(ptr, cmd->id);
	this->msg.len += REQ_ID_SIZE;

	/* Thêm dấu ngăn cách */
	ptr = this->msg.buf + this->msg.len;
	*ptr = REQ_SEPARATOR;
	this->msg.len += REQ_SEPARATOR_SIZE;

	/* Thêm các tham số */
	ptr = this->msg.buf + this->msg.len;
	switch (cmd->id) {
	case CMD_ID_GET_COUNTER:
		break;
	case CMD_ID_SET_COUNTER:
		ret = snprintf(ptr, VN29A80_MSG_LEN - this->msg.len, "%08X",
			       cmd->param_cnt);
		if (ret > 0 && ret < VN29A80_MSG_LEN - this->msg.len) {
			this->msg.len += ret;
		} else {
			dev_err(this->dev, "%s failed to copy param: ret = %d\n",
				__func__, ret);
			mutex_unlock(&this->lock);
			return -EINVAL;
		}
		break;
	default:
		dev_err(this->dev, "%s invalid ID %u\n", __func__, cmd->id);
		mutex_unlock(&this->lock);
		return -EINVAL;
	}

	/* Thêm dấu ngăn cách */
	ptr = this->msg.buf + this->msg.len;
	*ptr = REQ_SEPARATOR;
	this->msg.len += REQ_SEPARATOR_SIZE;

	/* Thêm kết thúc */
	ptr = this->msg.buf + this->msg.len;
	ret = strscpy(ptr, REQ_END_FRAME, VN29A80_MSG_LEN - this->msg.len);
	if (ret == REQ_FOOTER_SIZE) {
		this->msg.len += ret;
	} else {
		dev_err(this->dev, "%s failed to copy EOF: ret = %d\n", __func__, ret);
		mutex_unlock(&this->lock);
		return -EINVAL;
	}

	dev_info(this->dev, "%s %s (%u bytes)\n", __func__, this->msg.buf, this->msg.len);

	mutex_unlock(&this->lock);

	return 0;
}

/**
 * @brief Hàm tạo của lớp vn29a80_req.
 * @param this Đối tượng của lớp vn29a80_req
 * @param dev Thông tin thiết bị
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_req_setup(vn29a80_req *this, struct device *dev)
{
	this->msg.len = 0;
	this->dev = dev;
	mutex_init(&this->lock);

	return 0;
}

/**
 * @brief Hàm hủy của lớp vn29a80_req.
 * @param this Đối tượng của lớp vn29a80_req
 */
void vn29a80_req_cleanup(vn29a80_req *this)
{
	mutex_destroy(&this->lock);
}
