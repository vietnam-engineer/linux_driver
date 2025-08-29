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

    protected:
	static inline const std::string SYSFS_UARTDEV{ "/sys/class/uartdev" };
};

#endif
