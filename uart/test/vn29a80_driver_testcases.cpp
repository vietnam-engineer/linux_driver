#include "vn29a80_driver_tester.hpp"

TEST_F(Vn29a80DrvTester, load_unload)
{
	std::string cmd = "ls " + SYSFS_UARTDEV + " > /dev/null 2>&1";
	expect_cmd_successful(cmd.c_str());
}
