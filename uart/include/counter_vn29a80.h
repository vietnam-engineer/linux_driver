/******************************************************************************
 * @file      counter_vn29a80.h
 * @brief     Các định nghĩa dùng chung bởi cả kernel driver and user application
 * @author    vietnam-engineer
 *****************************************************************************/

#ifndef __COUNTER_VN29A80_H_
#define __COUNTER_VN29A80_H_

/* Định nghĩa các IOCTL của character device file */

#define IOCTL_TYPE_COUNTER 0x84

#define DEV_SET_SEND_TIMEOUT _IOW(IOCTL_TYPE_COUNTER, 0x02, __u32)
#define DEV_GET_SEND_TIMEOUT _IOR(IOCTL_TYPE_COUNTER, 0x03, __u32 *)
#define DEV_SET_RECV_TIMEOUT _IOW(IOCTL_TYPE_COUNTER, 0x04, __u32)
#define DEV_GET_RECV_TIMEOUT _IOR(IOCTL_TYPE_COUNTER, 0x05, __u32 *)

/* Các định nghĩa cho lệnh */

enum cmd_id {
	CMD_ID_GET_COUNTER = 0x01,
	CMD_ID_SET_COUNTER = 0x02,
};

struct vn29a80_cmd {
	__u8 id;
	__u32 param_cnt;
};

/* Các định nghĩa cho dữ liệu */

struct vn29a80_data {
	/* dữ liệu được lấy từ thiết bị */

	__u32 counter;

	/* dữ liệu được thêm bởi device driver */

	__u64 unix_time_ns;
	__u64 sys_uptime_ns;
	__u32 res_count;
};

#endif /* __COUNTER_VN29A80_H_ */
