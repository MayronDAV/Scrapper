#pragma once
#include "Event.h"
#include "Core/Definitions.h"



namespace SCPY
{
    class MouseMovedEvent : public Event
    {
        public:
            MouseMovedEvent(float p_X, float p_Y)
                : m_MouseX(p_X), m_MouseY(p_Y) {}

            inline float GetX() const { return m_MouseX; }
            inline float GetY() const { return m_MouseY; }
            inline Vec2 GetPos() const { return Vec2(m_MouseX, m_MouseY); }

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
                return ss.str();
            }

            EVENT_CLASS_METHODS(MouseMoved)
            EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

        private:
            float m_MouseX;
            float m_MouseY;
    };

    class MouseScrolledEvent : public Event
    {
        public:
            MouseScrolledEvent(float p_XOffset, float p_YOffset)
                : m_XOffset(p_XOffset), m_YOffset(p_YOffset) {}

            inline float GetXOffset() const { return m_XOffset; }
            inline float GetYOffset() const { return m_YOffset; }
            inline Vec2 GetOffset() const { return Vec2(m_XOffset, m_YOffset); }

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "MouseScrolledEvent: " << GetXOffset() << ", " << GetYOffset();
                return ss.str();
            }

            EVENT_CLASS_METHODS(MouseScrolled)
            EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

        private:
            float m_XOffset;
            float m_YOffset;
    };

    class MouseButtonEvent : public Event
    {
        public:
            inline int GetMouseButton() const { return m_Button; }

            EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

        protected:
            explicit MouseButtonEvent(int p_Button)
                : m_Button(p_Button) {}

            int m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
        public:
            explicit MouseButtonPressedEvent(int p_Button)
                : MouseButtonEvent(p_Button) {}

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "MouseButtonPressedEvent: " << m_Button;
                return ss.str();
            }

            EVENT_CLASS_METHODS(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
        public:
            explicit MouseButtonReleasedEvent(int p_Button)
                : MouseButtonEvent(p_Button) {}

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "MouseButtonReleasedEvent: " << m_Button;
                return ss.str();
            }

            EVENT_CLASS_METHODS(MouseButtonReleased)
    };

} // namespace SCPY
