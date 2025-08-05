#pragma once
#include "Events/Event.h"
#include "Window.h"
#include "ImGuiLayer.h"

// std
#include <memory>


struct GLFWwindow;

namespace SCPY
{
    class Application
    {
        public:
            Application();
            ~Application();

            void Run();
            void OnEvent(Event& p_Event);

            double GetDeltaTime() { return m_DeltaTime; }
            std::shared_ptr<Window>& GetWindow() { return m_Window; }

            static Application& Get() { return *s_Instance; }

        private:
            void Init();

        private:
            static inline Application* s_Instance = nullptr;

            std::shared_ptr<Window> m_Window;
            std::shared_ptr<ImGuiLayer> m_ImGuiLayer;
            int m_Width = 800;
            int  m_Height = 600;

            double m_LastTime = 0;
            double m_DeltaTime = 0;

            bool m_Running = true;
    };

} // namespace SCPY