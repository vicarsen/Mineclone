#include <mineclonelib/cvar.h>

static mc::cvar<const char *> app_name("Mineclone", "app/name",
				       "The name of the application");

static mc::cvar<const char *> engine_name("Mineclone Engine", "engine/name",
					  "The name of the engine");

int main()
{
	const char *app_name =
		mc::cvars<const char *>::get()->find("app/name")->get();

	const char *engine_name =
		mc::cvars<const char *>::get()->find("engine/name")->get();

	LOG_INFO(Default, "Starting app {} with engine {}", app_name,
		 engine_name);

	return 0;
}
