#include "gui.h"

namespace GUI
{
    Window::Window(const char* name) :
        context { .name = std::string(name), .opened = false }
    {}

    bool Window::Begin(ImGuiWindowFlags flags)
    {
        if(context.opened)
        {
            ::GUI::Begin(context.name.c_str(), &context.opened, flags);
            return true;
        }

        return false;
    }

    void Window::End()
    {
        ::GUI::End();
    }

    DemoWindow::DemoWindow() :
        Window("Demo Window")
    {}

    void DemoWindow::Draw()
    {
        if(context.opened)
            ::GUI::ShowDemoWindow(&context.opened);
    }

    void WindowManager::Draw()
    {
        for(std::size_t i = 0; i < windows.size(); i++)
            windows[i]->Draw();
    }

    namespace Plot
    {
        PlotDemoWindow::PlotDemoWindow() :
            Window("Plot Demo Window")
        {
        }

        void PlotDemoWindow::Draw()
        {
            if(context.opened)
                ImPlot::ShowDemoWindow(&context.opened);
        }
    };
};

