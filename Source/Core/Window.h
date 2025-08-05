#pragma once
#include "Definitions.h"
#include "Events/Event.h"

// std
#include <functional>



struct GLFWwindow;

namespace SCPY
{
    class Window
    {
        public:
            using EventCallbackFn = std::function<void(Event&)>;

        public:
            Window(const WindowSpecification& p_Spec = {});
            ~Window();

            void SwapBuffer();
            void OnUpdate();

            void Maximize();
            void Minimize();
            void Restore();

            float GetDPIScale() const { return m_Data.DPIScale; }

            void SetEventCallback(EventCallbackFn p_Callback) { m_Data.EventCallback = p_Callback; }
            void SetPosition(int p_X, int p_Y);
            void SetVsync(bool p_Value);
            void SetTitle(const std::string& p_Title);

            void Resize(uint32_t p_Width, uint32_t p_Height);
            void ChangeMode(WindowMode p_Mode, bool p_Maximize = true);

            bool IsMaximized() const;
            bool IsMinimized() const;

            Vec2 GetPosition() const;

            uint32_t GetWidth() const { return m_Data.Width; }
            uint32_t GetHeight() const { return m_Data.Height; }
            void* GetNative() { return m_Window; }
            bool IsVsync() const { return m_Data.Vsync; }

        private:
            void Init(const WindowSpecification& p_Spec);
            void Shutdown() noexcept;

        private:
            GLFWwindow* m_Window              = nullptr;

            struct WindowData
            {
                std::string Title             = "Koten";
                uint32_t Width                = 800;
                uint32_t Height               = 600;
                WindowMode Mode               = WindowMode::Windowed;
                bool Resizable                = true;
                bool Maximise                 = false;
                bool Vsync                    = false;
                float DPIScale                = 1.0f;

                EventCallbackFn EventCallback = nullptr;
            } m_Data;
    };

} // namespace SCPY