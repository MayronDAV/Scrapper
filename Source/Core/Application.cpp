#include "Application.h"
#include "Base.h"
#include "Window.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"
#include "MaterialDesignIcons.h"

// lib
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>



namespace SCPY
{
    namespace
    {
        void BeginDockspace(std::string p_ID, std::string p_Dockspace, bool p_MenuBar, ImGuiDockNodeFlags p_DockFlags = 0)
        {
            static bool opt_fullscreen = true;
            static bool opt_padding = false;
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton;

            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
            if (p_MenuBar)
                window_flags |= ImGuiWindowFlags_MenuBar;

            if (opt_fullscreen)
            {
                const ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->WorkPos);
                ImGui::SetNextWindowSize(viewport->WorkSize);
                ImGui::SetNextWindowViewport(viewport->ID);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }
            else
            {
                dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
            }

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 1.0f });
            if (!opt_padding)
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin(p_Dockspace.c_str(), nullptr, window_flags);
            if (!opt_padding)
                ImGui::PopStyleVar();
            ImGui::PopStyleColor(); // windowBg

            if (opt_fullscreen)
                ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID(p_ID.c_str());
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags | p_DockFlags);
            }
        }

        void EndDockspace()
        {
            ImGui::End();
        }

    } // namespace

    Application::Application()
    {
        s_Instance = this;

        WindowSpecification spec = {};
        m_Window = std::make_shared<Window>(spec);

        m_Window->SetEventCallback(
        [&] (Event& p_Event) 
        {
            OnEvent(p_Event);
        });

        m_ImGuiLayer = std::make_shared<ImGuiLayer>();
    }

    Application::~Application()
    {
        s_Instance = nullptr;
    }

    void Application::Run()
    {
        while (m_Running)
        {
            double currentTime  = glfwGetTime();
            m_DeltaTime         = currentTime - m_LastTime;
            m_LastTime          = currentTime;

            m_ImGuiLayer->OnUpdate((float)m_DeltaTime);
            
            if (!m_Window->IsMinimized())
            {
                m_ImGuiLayer->Begin();
                {
                    BeginDockspace("MyDockspace", "Dockspace", false);

                    ImGui::Begin("Teste " ICON_MDI_ALERT);
                    ImGui::Text("Testando...");
                    ImGui::End();

                    EndDockspace();
                }
                m_ImGuiLayer->End();

                m_Window->SwapBuffer();
            }

            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event &p_Event)
    {
        p_Event.Dispatch<WindowCloseEvent>(
        [&](WindowCloseEvent& p_Event)
        {
            m_Running = false;
            return true;
        });

        m_ImGuiLayer->OnEvent(p_Event);
    }
} // namespace SCPY
