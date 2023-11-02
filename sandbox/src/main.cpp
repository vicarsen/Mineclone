#include "engine.h"

class SandboxApplication : public ::Application
{
};

::std::unique_ptr<::Application> CreateApplication()
{
    return ::std::make_unique<::SandboxApplication>();
}

