#pragma once
#include "Core/Base.h"

// std
#include <functional>
#include <string>
#include <sstream>



namespace SCPY
{
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved, WindowDrop,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    };

    enum EventCategory
    {
        EventNone					= 0,
        EventCategoryWindow			= BIT(0),
        EventCategoryInput			= BIT(2),
        EventCategoryKeyboard		= BIT(3),
        EventCategoryMouse			= BIT(4),
        EventCategoryMouseButton	= BIT(5)
    };

    #define EVENT_CLASS_METHODS(type)															\
                static EventType GetStaticType() { return EventType::type; }					\
                virtual EventType GetEventType() const override { return GetStaticType(); }		\
                virtual const char* GetName() const override { return #type; }

    #define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

    class Event
    {
        public:
            template<typename T>
            using EventFn = std::function<bool(T&)>;

        public:
            virtual ~Event() = default;

            bool Handled = false;

            virtual EventType GetEventType() const { return EventType::None; }
            virtual const char* GetName() const { return "EventNone"; }
            virtual int GetCategoryFlags() const { return EventNone; }
            virtual std::string ToString() const { return GetName(); }

            inline bool IsInCategory(EventCategory p_Category) const
            {
                return GetCategoryFlags() & p_Category;
            }

            template<typename T>
            inline bool Dispatch(EventFn<T> p_Func)
            {
                if (GetEventType() == T::GetStaticType())
                {
                    Handled = p_Func(static_cast<T&>(*this));
                    return true;
                }
                return false;
            }

            inline operator std::string() const
            {
                return ToString();
            }

            inline bool operator==(const EventType& p_Type) const
            {
                return GetEventType() == p_Type;
            }

            friend std::ostream& operator<<(std::ostream& p_OS, const Event& p_Event)
            {
                p_OS << p_Event.ToString();
                return p_OS;
            }

            friend std::ostream& operator<<(std::ostream& p_OS, Event& p_Event)
            {
                p_OS << p_Event.ToString();
                return p_OS;
            }
    };

} // namespace SCPY