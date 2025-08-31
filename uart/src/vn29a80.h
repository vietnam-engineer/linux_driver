/******************************************************************************
 * @file      vn29a80.h
 * @brief     Khai báo các phần tử của vn29a80 driver (counter_vn29a80.ko)
 * @author    vietnam-engineer
 *****************************************************************************/

#ifndef __VN29A80_H_
#define __VN29A80_H_

#include "uartdev_core.h"
#include "counter_vn29a80.h"

/**************************** class vn29a80_req ********************************/
#define VN29A80_MSG_LEN 64

struct vn29a80_msg {
	u8 buf[VN29A80_MSG_LEN];
	u32 len;
};

typedef struct {
	struct vn29a80_msg msg;
	struct device *dev;
} vn29a80_req;
struct vn29a80_msg vn29a80_req_get(vn29a80_req *this);
s32 vn29a80_req_create(vn29a80_req *this, const struct vn29a80_cmd *cmd);

s32 vn29a80_req_setup(vn29a80_req *this, struct device *dev);
void vn29a80_req_cleanup(vn29a80_req *this);

/**************************** class vn29a80_drv ********************************/
typedef struct {
	vn29a80_req req;
} vn29a80_drv;

#endif /* __VN29A80_H_ */
