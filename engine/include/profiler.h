#pragma once

#include <chrono>

#include <queue>
#include <stack>
#include <functional>
#include <unordered_map>

#include "logger.h"
#include "mathematics.h"

#include "utils/array.h"
#include "utils/string.h"
#include "utils/thread.h"

DECLARE_LOG_CATEGORY(Profiler);

//#define PROFILE_ENABLED

#ifdef PROFILE_ENABLED
#define PROFILE_SCOPE(name) \
    static ::Utils::StringID __ID##name = ::Utils::StringIDOf<#name>(); \
    ::Profiler::ScopedTimer __ScopedTimer##name(__ID##name);

#define PROFILE_FUNCTION() \
    static ::Utils::StringID __ID##__FUNCTION__ = ::Utils::StringIDOf<__FUNCTION__>(); \
    ::Profiler::ScopedTimer __ScopedTimer##__FUNCTION__(__ID##__FUNCTION__);

#define PROFILE_THREAD(name) \
    static ::Utils::StringID __FrameID##name = ::Utils::StringIDOf<#name>(); \
    ::Profiler::ProfilerThread::RecordFrame(__FrameID##name);
#else
#define PROFILE_SCOPE(name)
#define PROFILE_FUNCTION()
#define PROFILE_THREAD(name)
#endif

namespace Profiler
{
    typedef std::chrono::high_resolution_clock clock;
    typedef clock::time_point time_point;
    typedef clock::duration duration;

    namespace __detail
    {
        typedef int ScopeID;

        static inline constexpr const ScopeID SCOPE_NONE = -1;
 
        struct Scope
        {
            ::Utils::StringID ID;
            duration total_time, own_time;
            ScopeID parent, child, prev, next;
            std::unordered_map<::Utils::StringID, ScopeID> children;
        };
         
        namespace Iterators
        {
            class ChildIterator;
            class ConstChildIterator;

            class TreeIterator;
            class ConstTreeIterator;
        };

        class ScopeTree
        {
        public:
            typedef Iterators::ChildIterator ChildIterator;
            typedef Iterators::ConstChildIterator ConstChildIterator;

            typedef Iterators::TreeIterator TreeIterator;
            typedef Iterators::ConstTreeIterator ConstTreeIterator;

            ScopeTree(int reserved = 128);
            ~ScopeTree() = default;

            ScopeTree(ScopeTree&& other) = default;
            ScopeTree(const ScopeTree& other) = default;

            ScopeTree& operator=(ScopeTree&& other) = default;
            ScopeTree& operator=(const ScopeTree& other) = default;

            ChildIterator AddScope(::Utils::StringID ID, ScopeID parent);
            ChildIterator FindOrAdd(ScopeID parent, ::Utils::StringID ID);

            void Clear();

            ChildIterator FirstLeaf(ScopeID ID);
            ConstChildIterator FirstLeaf(ScopeID ID) const;

            ChildIterator Root() noexcept;
            ChildIterator Parent(ScopeID child);
            ChildIterator Begin(ScopeID parent);
            ChildIterator End() noexcept;

            ConstChildIterator CRoot() const noexcept;
            ConstChildIterator CParent(ScopeID child) const;
            ConstChildIterator CBegin(ScopeID parent) const;
            ConstChildIterator CEnd() const noexcept;

            TreeIterator TRoot() noexcept;
            TreeIterator TBegin() noexcept;
            TreeIterator TEnd() noexcept;

            ConstTreeIterator CTRoot() const noexcept;
            ConstTreeIterator CTBegin() const noexcept;
            ConstTreeIterator CTEnd() const noexcept;

            inline Scope& operator[](ScopeID ID) { return scopes[ID]; }
            inline const Scope& operator[](ScopeID ID) const { return scopes[ID]; }

            inline std::size_t Size() const noexcept { return scopes.size(); }

        private:
            std::vector<Scope> scopes;

            friend class ::Profiler::__detail::Iterators::ChildIterator;
            friend class ::Profiler::__detail::Iterators::ConstChildIterator;

            friend class ::Profiler::__detail::Iterators::TreeIterator;
            friend class ::Profiler::__detail::Iterators::ConstTreeIterator;
        };

        namespace Iterators
        {
            class ChildIterator
            {
            public:
                inline ChildIterator(ScopeTree* tree = nullptr, ScopeID ID = SCOPE_NONE) noexcept : tree(tree), ID(ID) {}
                inline ChildIterator(const ChildIterator& other) noexcept = default;
                inline ChildIterator(ChildIterator&& other) noexcept = default;

                inline ~ChildIterator() noexcept = default;

                inline ChildIterator& operator=(ChildIterator&& other) noexcept = default;
                inline ChildIterator& operator=(const ChildIterator& other) noexcept = default;

                inline ChildIterator& operator++() { ID = tree->scopes[ID].next; return *this; }
                inline ChildIterator operator++(int) { ChildIterator copy(*this); ID = tree->scopes[ID].next; return copy; }
                inline ChildIterator& operator--() { ID = tree->scopes[ID].prev; return *this; }
                inline ChildIterator operator--(int) { ChildIterator copy(*this); ID = tree->scopes[ID].prev; return copy; }

                inline ChildIterator Parent() const noexcept { return tree->Parent(ID); }
                inline ChildIterator Begin() const noexcept { return tree->Begin(ID); }
                inline ChildIterator End() const noexcept { return tree->End(); }

                inline ChildIterator AddChild(::Utils::StringID ID) { return tree->AddScope(ID, this->ID); }
                inline ChildIterator FindOrAdd(::Utils::StringID ID) { return tree->FindOrAdd(this->ID, ID); }

                inline Scope& operator*() const { return tree->scopes[ID]; }
                inline Scope* operator->() const { return &tree->scopes[ID]; }

                inline bool operator==(const ChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }

                inline ScopeID GetScopeID() const noexcept { return ID; }
                inline operator ScopeID() const noexcept { return ID; }

                inline operator bool() const noexcept { return tree != nullptr && ID != SCOPE_NONE; }

                inline ScopeTree* GetScopeTree() const noexcept { return tree; }

            private:
                ScopeTree* tree;
                ScopeID ID;

                friend class ConstChildIterator;
                friend class TreeIterator;
                friend class ConstTreeIterator;
            };

            class ConstChildIterator
            {
            public:
                inline ConstChildIterator(const ScopeTree* tree = nullptr, ScopeID ID = SCOPE_NONE) noexcept : tree(tree), ID(ID) {}
                inline ConstChildIterator(const ConstChildIterator& other) noexcept = default;
                inline ConstChildIterator(ConstChildIterator&& other) noexcept = default;
                inline ConstChildIterator(const ChildIterator& other) noexcept : tree(other.tree), ID(other.ID) {}

                inline ~ConstChildIterator() noexcept = default;

                inline ConstChildIterator& operator=(const ConstChildIterator& other) noexcept = default;
                inline ConstChildIterator& operator=(ConstChildIterator&& other) noexcept = default;
                inline ConstChildIterator& operator=(const ChildIterator& other) noexcept { tree = other.tree; ID = other.ID; return *this; }

                inline ConstChildIterator& operator++() { ID = tree->scopes[ID].next; return *this; }
                inline ConstChildIterator operator++(int) { ConstChildIterator copy(*this); ID = tree->scopes[ID].next; return copy; }
                inline ConstChildIterator& operator--() { ID = tree->scopes[ID].prev; return *this; }
                inline ConstChildIterator operator--(int) { ConstChildIterator copy(*this); ID = tree->scopes[ID].prev; return copy; }

                inline ConstChildIterator Parent() const noexcept { return tree->CParent(ID); }
                inline ConstChildIterator Begin() const noexcept { return tree->CBegin(ID); }
                inline ConstChildIterator End() const noexcept { return tree->CEnd(); }

                inline const Scope& operator*() const { return tree->scopes[ID]; }
                inline const Scope* operator->() const { return &tree->scopes[ID]; }

                inline bool operator==(const ChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const ConstChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ConstChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }

                inline ScopeID GetScopeID() const noexcept { return ID; }
                inline operator ScopeID() const noexcept { return ID; }

                inline operator bool() const noexcept { return tree != nullptr && ID != SCOPE_NONE; }

                inline const ScopeTree* GetScopeTree() const noexcept { return tree; }

            private:
                const ScopeTree* tree;
                ScopeID ID;

                friend class TreeIterator;
                friend class ConstTreeIterator;
            };

            class TreeIterator
            {
            public:
                inline TreeIterator(ScopeTree* tree = nullptr, ScopeID ID = SCOPE_NONE) noexcept : tree(tree), ID(ID) {}
                inline TreeIterator(const TreeIterator& other) noexcept = default;
                inline TreeIterator(TreeIterator&& other) noexcept = default;
                
                inline ~TreeIterator() noexcept = default;

                inline TreeIterator& operator=(const TreeIterator& other) noexcept = default;
                inline TreeIterator& operator=(TreeIterator&& other) noexcept = default;
                
                inline TreeIterator& operator++() { ID = (tree->scopes[ID].next == SCOPE_NONE ? tree->scopes[ID].parent : tree->FirstLeaf(tree->scopes[ID].next)); return *this; }
                inline TreeIterator operator++(int) { TreeIterator copy(*this); ID = (tree->scopes[ID].next == SCOPE_NONE ? tree->scopes[ID].parent : tree->FirstLeaf(tree->scopes[ID].next)); return copy; }

                inline ChildIterator Parent() const noexcept { return tree->Parent(ID); }
                inline ChildIterator Begin() const noexcept { return tree->Begin(ID); }
                inline ChildIterator End() const noexcept { return tree->End(); }

                inline Scope& operator*() const { return tree->scopes[ID]; }
                inline Scope* operator->() const { return &tree->scopes[ID]; }

                inline bool operator==(const ChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const ConstChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ConstChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const TreeIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const TreeIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }

                inline ScopeID GetScopeID() const noexcept { return ID; }
                inline operator ScopeID() const noexcept { return ID; }

                inline operator bool() const noexcept { return tree != nullptr && ID != SCOPE_NONE; }

                inline ScopeTree* GetScopeTree() const noexcept { return tree; }

            private:
                ScopeTree* tree;
                ScopeID ID;

                friend class ConstTreeIterator;
            };

            class ConstTreeIterator
            {
            public:
                inline ConstTreeIterator(const ScopeTree* tree = nullptr, ScopeID ID = SCOPE_NONE) noexcept : tree(tree), ID(ID) {}
                inline ConstTreeIterator(const ConstTreeIterator& other) noexcept = default;
                inline ConstTreeIterator(ConstTreeIterator&& other) noexcept = default;
                inline ConstTreeIterator(const TreeIterator& other) noexcept : tree(other.tree), ID(other.ID) {}

                inline ~ConstTreeIterator() noexcept = default;

                inline ConstTreeIterator& operator=(const ConstTreeIterator& other) noexcept = default;
                inline ConstTreeIterator& operator=(ConstTreeIterator&& other) noexcept = default;
                inline ConstTreeIterator& operator=(const TreeIterator& other) noexcept { tree = other.tree; ID = other.ID; return *this; }

                inline ConstTreeIterator& operator++() { ID = (tree->scopes[ID].next == SCOPE_NONE ? tree->scopes[ID].parent : tree->FirstLeaf(tree->scopes[ID].next)); return *this; }
                inline ConstTreeIterator operator++(int) { ConstTreeIterator copy(*this); ID = (tree->scopes[ID].next == SCOPE_NONE ? tree->scopes[ID].parent : tree->FirstLeaf(tree->scopes[ID].next)); return copy; }

                inline ConstChildIterator Parent() const noexcept { return tree->CParent(ID); }
                inline ConstChildIterator Begin() const noexcept { return tree->CBegin(ID); }
                inline ConstChildIterator End() const noexcept { return tree->CEnd(); }

                inline const Scope& operator*() const { return tree->scopes[ID]; }
                inline const Scope* operator->() const { return &tree->scopes[ID]; }

                inline bool operator==(const ChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const ConstChildIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ConstChildIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const TreeIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const TreeIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }
                inline bool operator==(const ConstTreeIterator& other) const noexcept { return tree == other.tree && ID == other.ID; }
                inline bool operator!=(const ConstTreeIterator& other) const noexcept { return tree != other.tree || ID != other.ID; }

                inline ScopeID GetScopeID() const noexcept { return ID; }
                inline operator ScopeID() const noexcept { return ID; }

                inline operator bool() const noexcept { return tree != nullptr && ID != SCOPE_NONE; }

                inline const ScopeTree* GetScopeTree() const noexcept { return tree; }
                
            private:
                const ScopeTree* tree;
                ScopeID ID;
            };
        };

        struct ProfilerFrame
        {
            ScopeTree scope_tree;
        };
    };

    class ProfilerThread
    {
    public:
        typedef __detail::ScopeTree ScopeTree;
        typedef __detail::ProfilerFrame ProfilerFrame;
        typedef std::function<void(const ProfilerFrame&)> ProfilerFrameCallback;

        ProfilerThread();
        ProfilerThread(ProfilerThread&& other) = delete;
        ProfilerThread(const ProfilerThread& other) = delete;

        ~ProfilerThread();

        ProfilerThread& operator=(ProfilerThread&& other) = delete;
        ProfilerThread& operator=(const ProfilerThread& other) = delete;

        static void AddCallback(::Utils::StringID ID, const ProfilerFrameCallback& callback);
        static void RemoveCallback(::Utils::StringID ID);

        static void PushScope(::Utils::StringID ID);
        static void PopScope();

        static void RecordFrame(::Utils::StringID ID);

        inline static const ProfilerFrame& GetProfilerFrame() { return profiler_frame; }
        static void Reset(::Utils::StringID ID);

        inline static std::mutex& GetMutex() noexcept { return mutex; }

    private:
        void Run();

        void DispatchFrames();
        void CalculateStats(ProfilerFrame& frame);

    private:
        std::thread thread;
        std::atomic<bool> exit;

        thread_local static ProfilerFrame profiler_frame;
        thread_local static ScopeTree::ChildIterator stack_top;
        thread_local static std::stack<time_point> start_times;
        thread_local static time_point frame_start_time;

        static std::queue<ProfilerFrame> frame_queue;
        static std::vector<std::pair<::Utils::StringID, ProfilerFrameCallback>> callbacks;
        static std::unordered_map<::Utils::StringID, std::size_t> callback_idx;
        static std::mutex mutex;
    };

    class ScopedTimer
    {
    public:
        inline ScopedTimer(::Utils::StringID ID) { ProfilerThread::PushScope(ID); }
        inline ~ScopedTimer() { ProfilerThread::PopScope(); }
    };
};

namespace GUI
{
    namespace Profiler
    {
        struct ShadedPlotDataPoint
        {
            float x, y1, y2;
        };

        struct ShadedPlot
        {
            ::Utils::CircularArray<ShadedPlotDataPoint> data_points;
            ::std::string name;
        };

        struct ThreadData
        {
            ::std::unordered_map<::Utils::StringID, ShadedPlot> plots;
        };

        class ProfilerWindow : public ::GUI::Window
        {
        public:
            ProfilerWindow();
            virtual ~ProfilerWindow();

            virtual void Draw() override final;

        private:
            ::std::unordered_map<::Utils::StringID, ThreadData> thread_datas;
            ::std::mutex mutex;

            inline static const float history = 32.0f;
            inline static const ::Utils::USize divisions = 512;
        };
    };
};

