#include "profiler.h"

#include <GLFW/glfw3.h>

#include <map>

#include "utils/string.h"
#include "utils/time.h"

DEFINE_LOG_CATEGORY(Profiler, CONSOLE_LOGGER(info));

namespace Profiler
{
    namespace __detail
    {
        ScopeTree::ScopeTree(int reserved)
        {
            scopes.reserve(reserved);
            Scope& root = scopes.emplace_back();
            root.parent = root.child = root.prev = root.next = SCOPE_NONE;
        }

        ScopeTree::ChildIterator ScopeTree::AddScope(::Utils::StringID ID, ScopeID parent)
        {
            ScopeID scopeID = scopes.size();
            Scope& scope = scopes.emplace_back();
            scope.ID = ID;
            scope.parent = parent;
            scope.child = SCOPE_NONE;
            scope.prev = SCOPE_NONE;

            scope.next = scopes[parent].child;
            if(scope.next != SCOPE_NONE)
                scopes[scope.next].prev = scopeID;

            scopes[parent].child = scopeID;
            scopes[parent].children.emplace(ID, scopeID);

            return ChildIterator(this, scopeID);
        }

        ScopeTree::ChildIterator ScopeTree::FindOrAdd(ScopeID parent, ::Utils::StringID ID)
        {
            if(scopes[parent].children.find(ID) == scopes[parent].children.end())
                return AddScope(ID, parent);
            else return ChildIterator(this, scopes[parent].children[ID]);
        }

        void ScopeTree::Clear()
        {
            scopes.resize(1);
            Scope& root = scopes[0];
            root.total_time = {};
            root.parent = root.child = root.prev = root.next = SCOPE_NONE;
            root.children.clear();
        }

        ScopeTree::ChildIterator ScopeTree::FirstLeaf(ScopeID ID)
        {
            while(scopes[ID].child != SCOPE_NONE)
                ID = scopes[ID].child;
            return ChildIterator(this, ID);
        }

        ScopeTree::ConstChildIterator ScopeTree::FirstLeaf(ScopeID ID) const
        {
            while(scopes[ID].child != SCOPE_NONE)
                ID = scopes[ID].child;
            return ConstChildIterator(this, ID);
        }

        ScopeTree::ChildIterator ScopeTree::Root() noexcept
        {
            return ChildIterator(this, 0);
        }

        ScopeTree::ChildIterator ScopeTree::Parent(ScopeID child)
        {
            return ChildIterator(this, scopes[child].parent);
        }

        ScopeTree::ChildIterator ScopeTree::Begin(ScopeID parent)
        {
            return ChildIterator(this, scopes[parent].child);
        }

        ScopeTree::ChildIterator ScopeTree::End() noexcept
        {
            return ChildIterator(this, SCOPE_NONE);
        }

        ScopeTree::ConstChildIterator ScopeTree::CRoot() const noexcept
        {
            return ConstChildIterator(this, 0);
        }

        ScopeTree::ConstChildIterator ScopeTree::CParent(ScopeID child) const
        {
            return ConstChildIterator(this, scopes[child].parent);
        }

        ScopeTree::ConstChildIterator ScopeTree::CBegin(ScopeID parent) const
        {
            return ConstChildIterator(this, scopes[parent].child);
        }

        ScopeTree::ConstChildIterator ScopeTree::CEnd() const noexcept
        {
            return ConstChildIterator(this, SCOPE_NONE);
        }

        ScopeTree::TreeIterator ScopeTree::TRoot() noexcept
        {
            return TreeIterator(this, 0);
        }

        ScopeTree::TreeIterator ScopeTree::TBegin() noexcept
        {
            return TreeIterator(this, FirstLeaf(0));
        }

        ScopeTree::TreeIterator ScopeTree::TEnd() noexcept
        {
            return TreeIterator(this, SCOPE_NONE);
        }

        ScopeTree::ConstTreeIterator ScopeTree::CTRoot() const noexcept
        {
            return ConstTreeIterator(this, 0);
        }

        ScopeTree::ConstTreeIterator ScopeTree::CTBegin() const noexcept
        {
            return ConstTreeIterator(this, FirstLeaf(0));
        }

        ScopeTree::ConstTreeIterator ScopeTree::CTEnd() const noexcept
        {
            return ConstTreeIterator(this, SCOPE_NONE);
        }
    };

    thread_local ProfilerThread::ProfilerFrame ProfilerThread::profiler_frame;
    thread_local ProfilerThread::ScopeTree::ChildIterator ProfilerThread::stack_top;
    thread_local std::stack<time_point> ProfilerThread::start_times;
    thread_local time_point ProfilerThread::frame_start_time;

    std::queue<ProfilerThread::ProfilerFrame> ProfilerThread::frame_queue;
    std::vector<std::pair<::Utils::StringID, ProfilerThread::ProfilerFrameCallback>> ProfilerThread::callbacks;
    std::unordered_map<::Utils::StringID, std::size_t> ProfilerThread::callback_idx;
    std::mutex ProfilerThread::mutex;

    ProfilerThread::ProfilerThread()
    {
        exit = false;
        thread = std::move(std::thread([&]() { Run(); }));
    }

    ProfilerThread::~ProfilerThread()
    {
        exit = true;
        thread.join();
    }

    void ProfilerThread::AddCallback(::Utils::StringID ID, const ProfilerFrameCallback& callback)
    {
        std::lock_guard<std::mutex> guard(mutex);
        callback_idx[ID] = callbacks.size();
        callbacks.emplace_back(ID, callback);
    }

    void ProfilerThread::RemoveCallback(::Utils::StringID ID)
    {
        std::lock_guard<std::mutex> guard(mutex);
        std::size_t idx = callback_idx[ID];
        callback_idx.erase(ID);

        if(idx != callbacks.size() - 1)
        {
            callbacks[idx] = std::move(callbacks.back());
            callback_idx[callbacks[idx].first] = idx;
        }

        callbacks.pop_back();
    }

    void ProfilerThread::PushScope(::Utils::StringID ID)
    {
        stack_top = stack_top.FindOrAdd(ID);
        start_times.emplace(clock::now());
    }

    void ProfilerThread::PopScope()
    {
        stack_top->total_time += clock::now() - start_times.top();
        start_times.pop();
        stack_top = stack_top.Parent();
    }

    void ProfilerThread::Reset(::Utils::StringID ID)
    {
        profiler_frame = {};
        stack_top = profiler_frame.scope_tree.Root();
        stack_top->ID = ID;
        frame_start_time = clock::now();
    }

    void ProfilerThread::RecordFrame(::Utils::StringID ID)
    {
        if(stack_top)
        {
            stack_top->total_time += clock::now() - frame_start_time;

            std::lock_guard<std::mutex> guard(mutex);
            frame_queue.emplace(std::move(profiler_frame));
        }

        Reset(ID);
    }

    void ProfilerThread::Run()
    {
        while(!exit)
        {
            float profiler_thread_start = glfwGetTime();

            DispatchFrames();

            float profiler_thread_end = glfwGetTime();
            float profiler_thread_time = profiler_thread_end - profiler_thread_start;

            float thread_sleep = (profiler_thread_time < 1.0f ? 1.0f - profiler_thread_time : 0.0f);

            if(thread_sleep != 0.0f)
                std::this_thread::sleep_for(std::chrono::microseconds((long long) (thread_sleep * 1'000'000.0f)));
        }
    }

    void ProfilerThread::DispatchFrames()
    {
        std::lock_guard<std::mutex> guard(mutex);
        
        while(!frame_queue.empty())
        {
            ProfilerFrame& frame = frame_queue.front();
            
            CalculateStats(frame);

            for(std::size_t i = 0; i < callbacks.size(); i++)
                callbacks[i].second(frame);
            frame_queue.pop();
        }
    }

    void ProfilerThread::CalculateStats(ProfilerFrame& frame)
    {
        ScopeTree& scope_tree = frame.scope_tree;
        auto it = scope_tree.TBegin();
        while(it != scope_tree.TRoot())
        {
            it.Parent()->own_time -= it->total_time;
            it->own_time += it->total_time;
            it++;
        }
        it->own_time += it->total_time;
    }
};

namespace GUI
{
    namespace Profiler
    {
        ProfilerWindow::ProfilerWindow() :
            Window("Profiler")
        {
            ::Profiler::ProfilerThread::AddCallback(::Utils::StringIDOf<"Profiler Window">(), [&](const ::Profiler::ProfilerThread::ProfilerFrame& frame)
            {
                const ::Profiler::ProfilerThread::ScopeTree& scope_tree = frame.scope_tree;

                ::std::unordered_map<::Utils::StringID, ::Utils::Duration> scopes;
                ::Utils::Duration total_time(0);
                for(auto it = scope_tree.CTBegin(); it != scope_tree.CTEnd(); it++)
                {
                    scopes[it->ID] += it->own_time;
                    total_time += it->own_time;
                }
                
                ::std::lock_guard<::std::mutex> guard(mutex);

                ThreadData& thread_data = thread_datas[frame.scope_tree.CTRoot()->ID];
                auto& plots = thread_data.plots;

                float start = 0.0f;
                float time = ::std::chrono::duration_cast<::std::chrono::nanoseconds>(::Utils::LocalTime()).count() / 1'000'000'000.0f;
                for(const auto&[ID, duration] : scopes)
                {
                    if(plots.find(ID) == plots.end())
                    {
                        ShadedPlot& plot = plots.emplace(ID, ShadedPlot()).first->second;
                        plot.name = ID.ToString();
                        plot.data_points.Reset(divisions);
                    }
                    
                    float current = duration.count() * 1.0f / total_time.count();
                    plots[ID].data_points.Push(ShadedPlotDataPoint { time, start, start + current });
                    start += current;
                }
            });
        }

        ProfilerWindow::~ProfilerWindow()
        {
            ::Profiler::ProfilerThread::RemoveCallback(::Utils::StringIDOf<"Profiler Window">());
        }

        void ProfilerWindow::Draw()
        {
            float time = ::std::chrono::duration_cast<::std::chrono::nanoseconds>(::Utils::LocalTime()).count() / 1'000'000'000.0f;

            if(Begin())
            {
                ::std::lock_guard<::std::mutex> guard(mutex);

                for(const auto&[ID, thread_data] : thread_datas)
                {
                    ::std::string thread_name = ID.ToString();
                    const auto& plots = thread_data.plots;

                    if(Plot::BeginPlot(thread_name.c_str(), ImVec2(-1, -1)))
                    {
                        Plot::SetupAxis(ImAxis_X1, "Time (s)");
                        Plot::SetupAxis(ImAxis_Y1, "Runtime (%)");
                        Plot::SetupAxisLimits(ImAxis_X1, time - history, time, ImGuiCond_Always);
                        Plot::SetupFinish();

                        {
                            for(const auto&[_, shaded_plot] : plots)
                            {
                                const float* x  = (const float*) &shaded_plot.data_points[0] + offsetof(ShadedPlotDataPoint, x);
                                const float* y1 = (const float*) &shaded_plot.data_points[1] + offsetof(ShadedPlotDataPoint, y1);
                                const float* y2 = (const float*) &shaded_plot.data_points[2] + offsetof(ShadedPlotDataPoint, y2);
                                Plot::PlotShaded(shaded_plot.name.c_str(), x, y1, y2, shaded_plot.data_points.Size(), 0, shaded_plot.data_points.Offset(), sizeof(ShadedPlotDataPoint));
                            }
                        }

                        Plot::EndPlot();
                    }
                }
                End();
            }
        }
    };
};

