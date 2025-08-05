#include "ImGuiLayer.h"
#include "Application.h"
#include "MaterialDesignIcons.h"

// lib
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>



namespace SCPY
{
    #include "Embedded/Fonts/MaterialDesign.inl"
    #include "Embedded/Fonts/RobotoBold.inl"
    #include "Embedded/Fonts/RobotoMedium.inl"
    #include "Embedded/Fonts/RobotoRegular.inl"

    ImGuiLayer::ImGuiLayer()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.IniFilename = "Layout.ini";
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

        m_FontSize *= Application::Get().GetWindow()->GetDPIScale();

        AddFonts();

        GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow()->GetNative());

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460 core");
    }

    ImGuiLayer::~ImGuiLayer()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        auto& window = Application::Get().GetWindow();
        io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();

            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::OnEvent(Event& p_Event)
    {
        if (!m_BlockEvents)
        {
            ImGuiIO& io      = ImGui::GetIO();
            p_Event.Handled |= p_Event.IsInCategory(EventCategoryMouse)     & io.WantCaptureMouse;
            p_Event.Handled |= p_Event.IsInCategory(EventCategoryKeyboard)  & io.WantCaptureKeyboard;
            p_Event.Handled |= p_Event.IsInCategory(EventCategoryKeyboard)  & io.WantTextInput;
        }
    }

    void ImGuiLayer::OnUpdate(float p_Dt)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = p_Dt;
    }

    ImGuiContext* ImGuiLayer::GetCurrentContext()
    {
        return ImGui::GetCurrentContext();
    }

    uint32_t ImGuiLayer::GetActiveWidgetID() const
    {
        ASSERT(GImGui);

		return GImGui->ActiveId;
    }

    void ImGuiLayer::AddFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::StyleColorsDark();

        io.FontGlobalScale = 1.0f;

        ImFontConfig icons_config;
        icons_config.MergeMode = false;
        icons_config.PixelSnapH = true;
        icons_config.OversampleH = icons_config.OversampleV = 1;
        icons_config.GlyphMinAdvanceX = 4.0f;
        icons_config.SizePixels = 12.0f;

        //static const ImWchar ranges[] = {
        //	0x0020,
        //	0x00FF,
        //	0x0400,
        //	0x044F,
        //	0,
        //};
        static const ImWchar ranges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x0100, 0x017F, // Latin Extended-A
            0x0180, 0x024F, // Latin Extended-B
            0x0300, 0x036F, // Combining Diacritical Marks
            0x0400, 0x04FF, // Cyrillic
            0x0100, 0x017F,
            0
        };

        io.Fonts->AddFontFromMemoryCompressedTTF(RobotoRegular_compressed_data, RobotoRegular_compressed_size, m_FontSize, &icons_config, ranges);
        
        {
            static const ImWchar icons_ranges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
            ImFontConfig icons_config;
            // merge in icons from Font Awesome
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            icons_config.GlyphOffset.y = 1.0f;
            icons_config.OversampleH = icons_config.OversampleV = 1;
            icons_config.GlyphMinAdvanceX = 4.0f;
            icons_config.SizePixels = 12.0f;

            io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, m_FontSize, &icons_config, icons_ranges);
        }

        io.Fonts->AddFontFromMemoryCompressedTTF(RobotoBold_compressed_data, RobotoBold_compressed_size, m_FontSize + 2.0f, &icons_config, ranges);

        io.Fonts->AddFontFromMemoryCompressedTTF(RobotoRegular_compressed_data, RobotoRegular_compressed_size, m_FontSize * 0.8f, &icons_config, ranges);
    
        io.Fonts->TexGlyphPadding = 1;
        for (int n = 0; n < io.Fonts->Sources.Size; n++)
        {
            ImFontConfig* font_config = (ImFontConfig*)&io.Fonts->Sources[n];
            font_config->RasterizerMultiply = 1.0f;
        }
    }

} // namespace SCPY
