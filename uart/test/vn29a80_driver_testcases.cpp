#include "vn29a80_driver_tester.hpp"

TEST_F(Vn29a80DrvTester, load_unload)
{
	std::string cmd = "ls " + SYSFS_UARTDEV + " > /dev/null 2>&1";
	expect_cmd_successful(cmd.c_str());
}

TEST_F(Vn29a80DrvTester, check_sysfs_files)
{
	test_read_write_file(std::string(SYSFS_REQ_RAW));
	test_read_only_file(std::string(SYSFS_RES_RAW));
	test_read_only_file(std::string(SYSFS_DATA));
	test_read_write_file(std::string(SYSFS_TIMEOUT));
}

TEST_F(Vn29a80DrvTester, check_device_file)
{
	test_exist_file(DEVICE_FILE);
}

/*
TEST_F(Vn29a80DrvTester, write_sysfs_req_raw)
{
}

TEST_F(Vn29a80DrvTester, read_sysfs_req_raw)
{
}

TEST_F(Vn29a80DrvTester, read_sysfs_res_raw)
{
}

TEST_F(Vn29a80DrvTester, read_sysfs_data)
{
}

TEST_F(Vn29a80DrvTester, write_sysfs_timeout)
{
}

TEST_F(Vn29a80DrvTester, read_sysfs_timeout)
{
}

TEST_F(Vn29a80DrvTester, write_sysfs_timeout)
{
}

TEST_F(Vn29a80DrvTester, open_close_device_file)
{
}

TEST_F(Vn29a80DrvTester, write_device_file)
{
}

TEST_F(Vn29a80DrvTester, read_device_file)
{
}

TEST_F(Vn29a80DrvTester, ioctl_device_file)
{
}
*/
