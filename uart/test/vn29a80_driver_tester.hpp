#ifndef __VN29A80_DRIVER_TESTER_HPP_
#define __VN29A80_DRIVER_TESTER_HPP_

#include <string>

#include "gtest/gtest.h"

class Vn29a80DrvTester : public ::testing::Test {
    public:
	Vn29a80DrvTester();
	~Vn29a80DrvTester() = default;

	void SetUp() override;
	void TearDown() override;

    protected:
	void expect_cmd_successful(const char *cmd);
	void expect_cmd_failed(const char *cmd);

	void test_exist_file(const std::string &file_name);
	void test_read_only_file(const std::string &file_name);
	void test_read_write_file(const std::string &file_name);

    protected:
	static inline const std::string SYSFS_UARTDEV{ "/sys/class/uartdev" };
	static inline const std::string SYSFS_REQ_RAW{
		"/sys/class/uartdev/counter1/req_raw"
	};
	static inline const std::string SYSFS_RES_RAW{
		"/sys/class/uartdev/counter1/res_raw"
	};
	static inline const std::string SYSFS_DATA{ "/sys/class/uartdev/counter1/data" };
	static inline const std::string SYSFS_TIMEOUT{
		"/sys/class/uartdev/counter1/timeout"
	};
	static inline const std::string DEVICE_FILE{ "/dev/counter1" };
};

#endif
