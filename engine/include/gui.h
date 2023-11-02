#pragma once

#include <imgui.h>
#include <implot.h>

#include <string>
#include <vector>
#include <memory>

namespace GUI
{
    using namespace ImGui;
    
    namespace __detail
    {
        struct WindowContext
        {
            std::string name;
            bool opened;
        };
    };

    class Window
    {
    public:
        typedef __detail::WindowContext WindowContext;

        Window(const char* name);
        virtual ~Window() = default;

        virtual void Draw() = 0;

        inline void Open() noexcept { context.opened = true; }
        inline void Close() noexcept { context.opened = false; }
        inline void Toggle() noexcept { context.opened = !context.opened; }
        
        inline bool IsOpened() const noexcept { return context.opened; }

    protected:
        bool Begin(ImGuiWindowFlags flags = 0);
        void End();

    protected:
        WindowContext context;
    };

    class DemoWindow : public Window
    {
    public:
        DemoWindow();
        virtual ~DemoWindow() = default;

        virtual void Draw() override final;
    };

    class WindowManager
    {
    public:
        WindowManager() = default;
        ~WindowManager() = default;

        void Draw();

        template<typename WindowType, typename... Args>
        ::std::shared_ptr<WindowType> emplace(Args&&... args)
        {
            std::shared_ptr<Window> ptr = std::make_shared<WindowType>(std::forward<Args>(args)...);
            windows.emplace_back(ptr);
            return ::std::static_pointer_cast<WindowType>(ptr);
        }

    private:
        std::vector<std::shared_ptr<Window>> windows;
    };

    namespace Plot
    {
        using namespace ImPlot;

        class PlotDemoWindow : public ::GUI::Window
        {
        public:
            PlotDemoWindow();
            virtual ~PlotDemoWindow() = default;

            virtual void Draw() override final;
        };
    };
};

