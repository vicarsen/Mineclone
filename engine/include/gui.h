#pragma once

#include <imgui.h>
#include <implot.h>

#include "utils/array.h"
#include "utils/string.h"
#include "utils/memory.h"

namespace GUI
{
    using namespace ImGui;
    
    namespace __detail
    {
        struct WindowContext
        {
            ::Utils::String name;
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
        ::Utils::SharedPointer<WindowType> Push(Args&&... args)
        {
            ::Utils::SharedPointer<Window> ptr = ::Utils::MakeShared<WindowType>(::std::forward<Args>(args)...);
            windows.Push(ptr);
            return ::Utils::StaticPointerCast<WindowType>(ptr);
        }

    private:
        ::Utils::Array<::Utils::SharedPointer<Window>> windows;
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

