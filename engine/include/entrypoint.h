#pragma once

::Utils::UniquePointer<::Application> CreateApplication();

int main()
{
    ::Utils::UniquePointer<::Application> app = CreateApplication();
    app->Run();
    app.reset();
    return 0;
}

