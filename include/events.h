#pragma once

#include <unordered_map>
#include <vector>
#include <memory>

#include "std_ext.h"

namespace Events
{
    template<typename EventType>
    class EventHandler
    {
    public:
        virtual void Handle(const EventType& type) = 0;
    };

    template<typename EventType>
    using EventHandlerPointer = std::weak_ptr<EventHandler<EventType>>;

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
            Events<EventType>().events.push_back(event);
        }

        template<typename EventType, typename... Args>
        void Dispatch(Args&&... args)
        {
            Events<EventType>().events.emplace_back(std::forward<Args>(args)...);
        }

        template<typename EventType>
        void RegisterHandler(const EventHandlerPointer<EventType>& handler)
        {
            Handlers<EventType>().handlers.emplace_back(handler);
        }

        template<typename EventType>
        void ProcessEvents()
        {
            EventArray<EventType>& events = Events<EventType>();
            EventHandlerArray<EventType>& event_handlers = Handlers<EventType>();

            for(int i = 0; i < event_handlers.handlers.size(); )
                if(auto handler = event_handlers.handlers[i].lock())
                {
                    for(int j = 0; j < events.events.size(); j++)
                        handler->Handle(events.events[j]);
                    i++;
                }
                else
                {
                    event_handlers.handlers[i] = event_handlers.handlers.back();
                    event_handlers.handlers.pop_back();
                }

            events.events.clear();
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
            std::vector<EventType> events;
        };

        class EventHandlerArrayBase
        {
        public:
            EventHandlerArrayBase() = default;
            virtual ~EventHandlerArrayBase() = default;
        };

        template<typename EventType>
        class EventHandlerArray : public EventHandlerArrayBase
        {
        public:
            EventHandlerArray() = default;
            virtual ~EventHandlerArray() = default;

        public:
            std::vector<EventHandlerPointer<EventType>> handlers;
        };

    private:
        template<typename EventType>
        inline EventArray<EventType>& Events()
        {
            auto& ptr = events[typeid(EventType).hash_code()];
            if(ptr == nullptr)
                ptr = std::make_shared<EventArray<EventType>>();
        
            return *std::static_pointer_cast<EventArray<EventType>>(ptr);
        }

        template<typename EventType>
        inline EventHandlerArray<EventType>& Handlers()
        {
            auto& ptr = handlers[typeid(EventType).hash_code()];
            if(ptr == nullptr)
                ptr = std::make_shared<EventHandlerArray<EventType>>();
            
            return *std::static_pointer_cast<EventHandlerArray<EventType>>(ptr);
        }

    private:
        std::unordered_map<std::size_t, std::shared_ptr<EventArrayBase>> events;
        std::unordered_map<std::size_t, std::shared_ptr<EventHandlerArrayBase>> handlers;
    };
};

