#include <cvar.h>
#include <log.h>

mc::cvar<int> test("test", "A test CVar", 5);

int main()
{
  LOG_ASSERT(test.value == 5, Default, "CVar direct access failure");
  LOG_ASSERT(mc::cvars<int>::get_instance()->get("test") == 5, Default, "CVar indirect access failure");
  LOG_ASSERT(mc::cvars<int>::get_instance()->get_cvar("test")->value == 5, Default, "CVar pointer indirect access failure");
  return 0;
}

