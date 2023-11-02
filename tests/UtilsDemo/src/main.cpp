#include "engine.h"

class UtilsDemoApplication : public ::Application
{
public:
    virtual void Init() override
    {
    }

    virtual void Update() override
    {
    }

    virtual void Destroy() override
    {
    }
};

::std::unique_ptr<::Application> CreateApplication()
{
    return ::std::make_unique<::UtilsDemoApplication>();
}

