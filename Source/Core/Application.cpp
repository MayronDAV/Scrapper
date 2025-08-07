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

        void DrawItemOutline(float p_Rounding, float p_Thickness, bool p_DrawWhenInactive, ImColor p_Color, ImColor p_ColorHovered, ImColor p_ColorActived)
        {
            auto* drawList = ImGui::GetWindowDrawList();

            ImRect expandedRect = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            expandedRect.Min.x -= 1.0f;
            expandedRect.Min.y -= 1.0f;
            expandedRect.Max.x += 1.0f;
            expandedRect.Max.y += 1.0f;

            const ImRect rect = expandedRect;
            if (ImGui::IsItemHovered() && !ImGui::IsItemActive())
            {
                drawList->AddRect(rect.Min, rect.Max,
                    p_ColorHovered, p_Rounding, 0, p_Thickness);
            }
            if (ImGui::IsItemActive())
            {
                drawList->AddRect(rect.Min, rect.Max,
                    p_ColorActived, p_Rounding, 0, p_Thickness);
            }
            else if (!ImGui::IsItemHovered() && p_DrawWhenInactive)
            {
                drawList->AddRect(rect.Min, rect.Max,
                    p_Color, p_Rounding, 0, p_Thickness);
            }
        }

        bool InputText(const std::string& p_Label, std::string& p_Text, bool p_DrawLabel = true, float p_OutlineRouding = false, bool p_DrawOutlineWhenInactive = true, ImGuiInputTextFlags p_Flags = 0, ImColor p_OutlineColor = ImColor(80, 80, 80))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
            char buffer[256];
            memset(buffer, 0, 256);
            memcpy(buffer, p_Text.c_str(), p_Text.length());
            ImGui::PushID(p_Label.c_str());

            if (p_DrawLabel)
            {
                ImGui::Text(p_Label.c_str());
                ImGui::SameLine();
            }

            float availableWidth = ImGui::GetContentRegionAvail().x;
            bool updated = ImGui::InputTextEx("##SceneName", NULL, buffer, 256, ImVec2(availableWidth, 0), p_Flags | ImGuiInputTextFlags_AlwaysOverwrite, (ImGuiInputTextCallback)0, (void*)0);

            DrawItemOutline(p_OutlineRouding, 1.0f, p_DrawOutlineWhenInactive, p_OutlineColor, p_OutlineColor, p_OutlineColor);
            
            if (updated)
                p_Text = std::string(buffer);

            ImGui::PopID();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            return updated;
        }

        bool InviButton(const char* p_Label, const ImVec2& p_Size = {0, 0})
        {
            ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
            bool updated = ImGui::Button(p_Label, p_Size);
            ImGui::PopStyleColor();

            return updated;
        }

    } // namespace

    Application::Application()
    {
        s_Instance = this;

        WindowSpecification spec = {};
        spec.Vsync = false;
        m_Window = std::make_shared<Window>(spec);

        m_Window->SetEventCallback(
        [&] (Event& p_Event) 
        {
            OnEvent(p_Event);
        });

        m_ImGuiLayer = std::make_shared<ImGuiLayer>();

        m_Search = std::make_shared<Search>();
        m_Search->SetDocsPerPage(50);
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
                    BeginDockspace("MyDockspace", "Dockspace", false, ImGuiDockNodeFlags_NoTabBar);

                    OnImgui();

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

    void Application::OnImgui()
    {
        if (m_ShowCopyPopup)
        {
            m_CopyPopupTimer -= m_DeltaTime;
            if (m_CopyPopupTimer <= 0.0)
                m_ShowCopyPopup = false;
        }

        auto dockID = ImGui::GetID("MyDockspace");
		ImGui::SetNextWindowDockID(dockID, ImGuiCond_Always);
        ImGui::Begin("Viewport");
        {
            auto size = ImGui::GetContentRegionAvail();
            float lineHeight = GImGui->Font->LegacySize + GImGui->Style.FramePadding.y * 2.0f;

            std::string term = m_Search->GetTerm();
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EscapeClearsAll;
            if (InputText(ICON_MDI_SEARCH_WEB, term, true, 4.0f, true, flags))
            {
                m_Search->SearchTerm(term);
            }
         
            auto lawsuits = m_Search->GetLawsuits();
            if (!lawsuits.empty())
            {
                int total = m_Search->GetTotalResults();
                int current = (m_Search->GetCurrentPage() * m_Search->GetDocsPerPage()) + 1;
                ImGui::Text("Results %i of %i", current, total);

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
                ImGui::BeginChild("##Content", { size.x, size.y - (lineHeight * 3.4f) }, false);
                ImGui::PopStyleVar();
                {
                    for (auto& lawsuit : lawsuits)
                    {
                        ImGui::PushID(lawsuit.Case.c_str());
                        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                        ImGui::SetNextWindowSize({0, 512}, ImGuiCond_Once);
                        ImGui::BeginChild("Lawsuit", {0, 0}, ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);
                        ImGui::PopStyleVar();
                        {
                            if (InviButton(ICON_MDI_CONTENT_COPY "##1", {0, lineHeight}))
                            {
                                ImGui::SetClipboardText(lawsuit.Case.c_str());
                                m_ShowCopyPopup = true;
                                m_CopyPopupTimer = m_CopyPopupDuration;
                            }
                            ImGui::SameLine();
                            ImGui::Text("CASE");

                            ImGui::TextWrapped(lawsuit.Case.c_str());
                            ImGui::NewLine();

                            auto available = ImGui::GetContentRegionAvail().x;
                            ImGui::BeginChild("##JUDGEDATE", {available / 2.0f, 2.0f * lineHeight});
                            {
                                if (InviButton(ICON_MDI_CONTENT_COPY "##2", {0, lineHeight}))
                                {
                                    ImGui::SetClipboardText(lawsuit.JudgmentDate.c_str());
                                    m_ShowCopyPopup = true;
                                    m_CopyPopupTimer = m_CopyPopupDuration;
                                }
                                ImGui::SameLine();
                                ImGui::Text("JUDGMENT DATE");

                                ImGui::TextWrapped(lawsuit.JudgmentDate.c_str());
                            }
                            ImGui::EndChild();
                            ImGui::SameLine();
                            ImGui::BeginChild("##PUBDATE", {available / 2.0f, 2.0f * lineHeight});
                            {
                                if (InviButton(ICON_MDI_CONTENT_COPY "##3", {0, lineHeight}))
                                {
                                    ImGui::SetClipboardText(lawsuit.PubDate.c_str());
                                    m_ShowCopyPopup = true;
                                    m_CopyPopupTimer = m_CopyPopupDuration;
                                }
                                ImGui::SameLine();
                                ImGui::Text("PUBLICATION DATE");

                                ImGui::TextWrapped(lawsuit.PubDate.c_str());
                            }
                            ImGui::EndChild();

                            ImGui::NewLine();

                            if (InviButton(ICON_MDI_CONTENT_COPY "##4", {0, lineHeight}))
                            {
                                ImGui::SetClipboardText(lawsuit.Rapporteur.c_str());
                                m_ShowCopyPopup = true;
                                m_CopyPopupTimer = m_CopyPopupDuration;
                            }
                            ImGui::SameLine();
                            ImGui::Text("RAPPORTEUR");

                            ImGui::TextWrapped(lawsuit.Rapporteur.c_str());
                            ImGui::NewLine();

                            if (InviButton(ICON_MDI_CONTENT_COPY "##5", {0, lineHeight}))
                            {
                                ImGui::SetClipboardText(lawsuit.Headnote.c_str());
                                m_ShowCopyPopup = true;
                                m_CopyPopupTimer = m_CopyPopupDuration;
                            }
                            ImGui::SameLine();
                            ImGui::Text("HEADNOTE");

                            ImGui::TextWrapped(lawsuit.Headnote.c_str());
                            ImGui::NewLine();

                            if (InviButton(ICON_MDI_CONTENT_COPY "##6", {0, lineHeight}))
                            {
                                ImGui::SetClipboardText(lawsuit.Decision.c_str());
                                m_ShowCopyPopup = true;
                                m_CopyPopupTimer = m_CopyPopupDuration;
                            }
                            ImGui::SameLine();
                            ImGui::Text("DECISION");

                            ImGui::TextWrapped(lawsuit.Decision.c_str());
                        }
                        ImGui::EndChild();
                        ImGui::PopID();
                    }
                }
                ImGui::EndChild();

                if (InviButton(ICON_MDI_PAGE_FIRST, { 0, lineHeight }))
                    m_Search->FirstPage();         

                ImGui::SameLine();

                if (InviButton(ICON_MDI_PAGE_PREVIOUS, { 0, lineHeight }))
                    m_Search->PrevPage();

                ImGui::SameLine();

                auto page = m_Search->GetCurrentPage();
                ImGui::Text("%i", page);

                ImGui::SameLine();

                if (InviButton(ICON_MDI_PAGE_NEXT, { 0, lineHeight }))
                    m_Search->NextPage();

                ImGui::SameLine();

                if (InviButton(ICON_MDI_PAGE_LAST, { 0, lineHeight }))
                    m_Search->LastPage();

                ImGui::SameLine();

                if (ImGui::Button("Export " ICON_MDI_FILE_EXPORT, { 0, lineHeight }))
                    m_Search->ExportYML();
            }

            if (m_ShowCopyPopup)
            {
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                ImGui::SetNextWindowBgAlpha(0.8f);
                
                if (ImGui::BeginPopup("##CopyPopup", 
                    ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | 
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
                {
                    float alpha = (float)(m_CopyPopupTimer / m_CopyPopupDuration);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    
                    ImGui::Text(ICON_MDI_CHECK_CIRCLE " ");
                    ImGui::SameLine();
                    ImGui::Text("Copied to clipboard!");
                    
                    ImGui::PopStyleVar();
                    ImGui::EndPopup();
                }
                else
                {
                    ImGui::OpenPopup("##CopyPopup");
                }
            }
        }
        ImGui::End();
    }
} // namespace SCPY
