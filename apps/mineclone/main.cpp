#include <mineclonelib/log.h>

int main()
{
	LOG_TRACE(Default, "Hello, world!");
	LOG_DEBUG(Default, "Hello, world!");
	LOG_INFO(Default, "Hello, world!");
	LOG_WARN(Default, "Hello, world!");
	LOG_ERROR(Default, "Hello, world!");
	LOG_CRITICAL(Default, "Hello, world!");
	return 0;
}
