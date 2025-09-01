/******************************************************************************
 * @file      vn29a80_uif.c
 * @brief     Triển khai phần tử vn29a80_uif của counter_vn29a80.ko
 * @author    vietnam-engineer
 *****************************************************************************/

#include "vn29a80.h"

/**
 * @brief Hàm tạo của lớp vn29a80_uif.
 * @param this Đối tượng của lớp vn29a80_uif
 * @param dev Thông tin thiết bị
 * @return 0 nếu thành công. !0 nếu có lỗi.
 */
s32 vn29a80_uif_setup(vn29a80_uif *this, struct device *dev)
{
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
