#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>

#include "vn29a80_driver_tester.hpp"

using namespace std::chrono_literals;

Vn29a80DrvTester::Vn29a80DrvTester()
{
}

void Vn29a80DrvTester::SetUp()
{
	if (system("lsmod | grep -q uartdev_core") == 0) {
		return;
	}

	expect_cmd_successful("ls /tmp/uartdev_core.ko > /dev/null 2>&1");
	expect_cmd_successful("insmod /tmp/uartdev_core.ko");
	expect_cmd_successful("lsmod | grep -q uartdev_core");

	expect_cmd_successful("ls /tmp/counter_vn29a80.ko > /dev/null 2>&1");
	expect_cmd_successful("insmod /tmp/counter_vn29a80.ko");
	expect_cmd_successful("lsmod | grep -q counter_vn29a80");

	std::this_thread::sleep_for(1s);
}

void Vn29a80DrvTester::TearDown()
{
	expect_cmd_successful("rmmod counter_vn29a80");
	expect_cmd_successful("rmmod uartdev_core");
}

void Vn29a80DrvTester::expect_cmd_successful(const char *cmd)
{
	EXPECT_EQ(0, system(cmd)) << "cmd = " << cmd;
}

void Vn29a80DrvTester::expect_cmd_failed(const char *cmd)
{
	EXPECT_NE(0, system(cmd)) << "cmd = " << cmd;
}

void Vn29a80DrvTester::test_exist_file(const std::string &file_name)
{
	EXPECT_EQ(0, access(file_name.c_str(), F_OK)) << file_name;
}

void Vn29a80DrvTester::test_read_only_file(const std::string &file_name)
{
	test_exist_file(file_name);

	std::ifstream ifs(file_name);
	ASSERT_TRUE(ifs.is_open());
	ifs.close();

	std::ofstream ofs(file_name);
	ASSERT_FALSE(ofs.is_open());
	ofs.close();
}

void Vn29a80DrvTester::test_read_write_file(const std::string &file_name)
{
	test_exist_file(file_name);

	std::ifstream ifs(file_name);
	ASSERT_TRUE(ifs.is_open());
	ifs.close();

	std::ofstream ofs(file_name);
	ASSERT_TRUE(ofs.is_open());
	ofs.close();
}
