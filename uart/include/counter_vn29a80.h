/******************************************************************************
 * @file      counter_vn29a80.h
 * @brief     Các định nghĩa dùng chung bởi cả kernel driver and user application
 * @author    vietnam-engineer
 *****************************************************************************/

#ifndef __COUNTER_VN29A80_H_
#define __COUNTER_VN29A80_H_

/* Các định nghĩa cho lệnh */

enum cmd_id {
	CMD_ID_GET_COUNTER = 0x01,
	CMD_ID_SET_COUNTER = 0x02,
};

struct vn29a80_cmd {
	__u8 id;
	__u32 param_cnt;
};

#endif /* __COUNTER_VN29A80_H_ */
