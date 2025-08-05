#pragma once
#include "Event.h"
#include "Core/Definitions.h"




namespace SCPY
{
    class WindowResizeEvent : public Event
    {
        public:
            WindowResizeEvent(uint32_t p_Width, uint32_t p_Height)
                : m_Width(p_Width), m_Height(p_Height) {}

            inline uint32_t GetWidth() const { return m_Width; }
            inline uint32_t GetHeight() const { return m_Height; }

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
                return ss.str();
            }

            EVENT_CLASS_METHODS(WindowResize)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

        private:
            uint32_t m_Width;
            uint32_t m_Height;
    };

    class WindowCloseEvent : public Event
    {
        public:
            WindowCloseEvent() = default;

            EVENT_CLASS_METHODS(WindowClose)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowFocusEvent : public Event
    {
        public:
            WindowFocusEvent() = default;

            EVENT_CLASS_METHODS(WindowFocus)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowLostFocusEvent : public Event
    {
        public:
            WindowLostFocusEvent() = default;

            EVENT_CLASS_METHODS(WindowLostFocus)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

    class WindowMovedEvent : public Event
    {
        public:
            WindowMovedEvent(int p_X, int p_Y) : m_PosX(p_X), m_PosY(p_Y) {}

            int GetX() const { return m_PosX; }
            int GetY() const { return m_PosY; }
            Vec2 GetPos() const { return Vec2((float)m_PosX, (float)m_PosY); }

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "WindowMovedEvent: " << m_PosX << ", " << m_PosY;
                return ss.str();
            }

            EVENT_CLASS_METHODS(WindowMoved)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

        private:
            int m_PosX;
            int m_PosY;
    };

    class WindowDropEvent : public Event
    {
        public:
            explicit WindowDropEvent(const std::vector<std::string>& p_Paths)
                : m_Paths(p_Paths) {}

            explicit WindowDropEvent(std::vector<std::string>&& p_Paths)
                : m_Paths(std::move(p_Paths)) {}

            const std::vector<std::string>& GetPaths() const { return m_Paths; }

            EVENT_CLASS_METHODS(WindowDrop)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

        private:
            std::vector<std::string> m_Paths;
    };

} // namespace SCPY
