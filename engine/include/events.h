#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

#include "std_ext.h"

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
        struct handles_type
        {
            static constexpr bool value = (std::is_same_v<EventType, EventTypes> || ...);
        };

        template<typename EventType>
        static constexpr bool handles_type_v = handles_type<EventType>::value;

        typedef std_ext::Pack<EventTypes...> EventTypesPack;
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
            std::lock_guard<std::mutex> guard(mutex);
            Events<EventType>().events.push_back(event);
        }

        template<typename EventType, typename... Args>
        void Dispatch(Args&&... args)
        {
            std::lock_guard<std::mutex> guard(mutex);
            Events<EventType>().events.emplace_back(std::forward<Args>(args)...);
        }

        template<typename EventType>
        void RegisterHandler(const std::shared_ptr<EventHandlerBase<EventType>>& handler)
        {
            std::lock_guard<std::mutex> guard(mutex);
            Handlers<EventType>().handlers.emplace_back(handler);
        }

        template<typename... EventTypes>
        void __RegisterHandler(std_ext::Pack<EventTypes...> pack, const std::shared_ptr<EventHandler<EventTypes...>>& handler)
        {
            (RegisterHandler<EventTypes>(handler), ...);
        }

        template<typename EventHandlerType>
        void RegisterHandler(const std::shared_ptr<EventHandlerType>& handler)
        {
            __RegisterHandler(typename EventHandlerType::EventTypesPack{}, std::static_pointer_cast<typename EventHandlerType::HandlerType>(handler));
        }

        /*template<typename EventType>
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
        }*/

        void ProcessEvents()
        {
            std::lock_guard<std::mutex> guard(mutex);

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
            std::vector<EventType> events;
        };

        class EventHandlerArrayBase
        {
        public:
            EventHandlerArrayBase() = default;
            virtual ~EventHandlerArrayBase() = default;

            virtual void Handle(const std::shared_ptr<EventArrayBase>& event_arr) = 0;
        };

        template<typename EventType>
        class EventHandlerArray : public EventHandlerArrayBase
        {
        public:
            EventHandlerArray() = default;
            virtual ~EventHandlerArray() = default;

            virtual void Handle(const std::shared_ptr<EventArrayBase>& _event_arr) override
            {
                const auto& event_arr = std::static_pointer_cast<EventArray<EventType>>(_event_arr);
                for(std::size_t i = 0; i < event_arr->events.size(); i++)
                {
                    for(std::size_t j = 0; j < handlers.size(); )
                        if(auto handler = handlers[j].lock())
                        {
                            handler->Handle(event_arr->events[i]);
                            j++;
                        }
                        else
                        {
                            handlers[j] = handlers.back();
                            handlers.pop_back();
                        }
                }
            }

        public:
            std::vector<std::weak_ptr<EventHandlerBase<EventType>>> handlers;
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
        std::mutex mutex;
    };
};

