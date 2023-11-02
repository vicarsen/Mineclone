#pragma once

::std::unique_ptr<::Application> CreateApplication();

int main()
{
    ::std::unique_ptr<::Application> app = CreateApplication();
    app->Run();
    app.reset();
    return 0;
}

