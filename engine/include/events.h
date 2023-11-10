#pragma once

#include "utils/type.h"
#include "utils/tuple.h"
#include "utils/array.h"
#include "utils/hash.h"
#include "utils/thread.h"
#include "utils/memory.h"

namespace Events
{
    template<typename EventType>
    class EventHandlerBase
    {
    public:
        virtual void Handle(const EventType& type) = 0;
    };

    template<typename... EventTypes>
    class EventHandler : public EventHandlerBase<EventTypes>...
    {
    public:
        template<typename EventType>
        struct HandlesType
        {
            static constexpr bool value = (::std::is_same_v<EventType, EventTypes> || ...);
        };

        template<typename EventType>
        static constexpr bool handles_type_v = HandlesType<EventType>::value;

        typedef ::Utils::Pack<EventTypes...> EventTypesPack;
        typedef EventHandler<EventTypes...> HandlerType;
    };

    class EventDispatcher
    {
    public:
        EventDispatcher()
        {
        }

        ~EventDispatcher()
        {
        }

        template<typename EventType>
        void Dispatch(const EventType& event)
        {
            Events<EventType>().events.Push(event);
        }

        template<typename EventType, typename... Args>
        void Dispatch(Args&&... args)
        {
            Events<EventType>().events.Push(std::forward<Args>(args)...);
        }

        template<typename EventType>
        void RegisterHandler(const ::Utils::SharedPointer<EventHandlerBase<EventType>>& handler)
        {
            Handlers<EventType>().handlers.Push(handler);
        }

        template<typename... EventTypes>
        void __RegisterHandler(::Utils::Pack<EventTypes...> pack, const ::Utils::SharedPointer<EventHandler<EventTypes...>>& handler)
        {
            (RegisterHandler<EventTypes>(handler), ...);
        }

        template<typename EventHandlerType>
        void RegisterHandler(const ::Utils::SharedPointer<EventHandlerType>& handler)
        {
            __RegisterHandler(typename EventHandlerType::EventTypesPack{}, ::Utils::StaticPointerCast<typename EventHandlerType::HandlerType>(handler));
        }

        void ProcessEvents()
        {
            for(auto&[type, event_arr] : events)
            {
                auto it = handlers.find(type);
                if(it != handlers.end())
                    it->second->Handle(event_arr);
            }
            
            events.clear();
        }

    private:
        class EventArrayBase
        {
        public:
            EventArrayBase() = default;
            virtual ~EventArrayBase() = default;
        };

        template<typename EventType>
        class EventArray : public EventArrayBase
        {
        public:
            EventArray() = default;
            virtual ~EventArray() = default;
        
        public:
            ::Utils::Array<EventType> events;
        };

        class EventHandlerArrayBase
        {
        public:
            EventHandlerArrayBase() = default;
            virtual ~EventHandlerArrayBase() = default;

            virtual void Handle(const ::Utils::SharedPointer<EventArrayBase>& event_arr) = 0;
        };

        template<typename EventType>
        class EventHandlerArray : public EventHandlerArrayBase
        {
        public:
            EventHandlerArray() = default;
            virtual ~EventHandlerArray() = default;

            virtual void Handle(const ::Utils::SharedPointer<EventArrayBase>& _event_arr) override final
            {
                const auto& event_arr = ::Utils::StaticPointerCast<EventArray<EventType>>(_event_arr);
                for(::Utils::USize i = 0; i < event_arr->events.Size(); i++)
                {
                    for(::Utils::USize j = 0; j < handlers.Size(); )
                        if(auto handler = handlers[j].lock())
                        {
                            handler->Handle(event_arr->events[i]);
                            j++;
                        }
                        else
                        {
                            handlers[j] = handlers.Back();
                            handlers.Pop();
                        }
                }
            }

        public:
            ::Utils::Array<::Utils::WeakPointer<EventHandlerBase<EventType>>> handlers;
        };

    private:
        template<typename EventType>
        inline EventArray<EventType>& Events()
        {
            auto& ptr = events[typeid(EventType).hash_code()];
            if(ptr == nullptr)
                ptr = ::Utils::MakeShared<EventArray<EventType>>();
        
            return *::Utils::StaticPointerCast<EventArray<EventType>>(ptr);
        }

        template<typename EventType>
        inline EventHandlerArray<EventType>& Handlers()
        {
            auto& ptr = handlers[typeid(EventType).hash_code()];
            if(ptr == nullptr)
                ptr = ::Utils::MakeShared<EventHandlerArray<EventType>>();
            
            return *::Utils::StaticPointerCast<EventHandlerArray<EventType>>(ptr);
        }

    private:
        ::Utils::HashMap<::Utils::USize, ::Utils::SharedPointer<EventArrayBase>> events;
        ::Utils::HashMap<::Utils::USize, ::Utils::SharedPointer<EventHandlerArrayBase>> handlers;
    };

    class EventDispatcherMT
    {
    public:
        template<typename EventType>
        inline void Dispatch(const EventType& event)
        {
            ::Utils::LockGuard<> guard(mutex);
            dispatcher.Dispatch<EventType>(event);
        }

        template<typename EventType, typename... Args>
        inline void Dispatch(Args&&... args)
        {
            ::Utils::LockGuard<> guard(mutex);
            dispatcher.Dispatch<EventType, Args...>(::std::forward<Args>(args)...);
        }

        template<typename EventType>
        inline void RegisterHandler(const ::Utils::SharedPointer<EventHandlerBase<EventType>>& handler)
        {
            ::Utils::LockGuard<> guard(mutex);
            dispatcher.RegisterHandler<EventType>(handler);
        }
        
        template<typename EventHandlerType>
        void RegisterHandler(const ::Utils::SharedPointer<EventHandlerType>& handler)
        {
            ::Utils::LockGuard<> guard(mutex);
            dispatcher.RegisterHandler<EventHandlerType>(handler);
        }

        inline void ProcessEvents()
        {
            ::Utils::LockGuard<> guard(mutex);
            dispatcher.ProcessEvents();
        }

    private:
        EventDispatcher dispatcher;
        ::Utils::Mutex mutex;
    };
};

