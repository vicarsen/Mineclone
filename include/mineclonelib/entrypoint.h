#include "mineclonelib/application.h"

std::unique_ptr<mc::application> create_app();

int main(int argc, const char **argv)
{
	std::unique_ptr<mc::application> app = create_app();
	app->run();
	app.reset();
	return 0;
};
