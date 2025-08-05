#include "Window.h"
#include "Base.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Events/WindowEvent.h"

// lib
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>



namespace SCPY
{
    namespace
	{
		static void GLFWErrorCallback(int p_Error, const char* p_Description)
		{
			LOG("GLFW Error ({0}): {1}", p_Error, p_Description);
		}

		static uint8_t s_GLFWWindowCount = 0;

	} // namespace

    Window::Window(const WindowSpecification &p_Spec)
    {
        Init(p_Spec);
    }

    Window::~Window()
    {
        Shutdown();
    }

    void Window::SwapBuffer()
    {
        glfwSwapBuffers(m_Window);
    }

    void Window::OnUpdate()
    {
        glfwPollEvents();
    }

    void Window::Maximize()
    {
        glfwMaximizeWindow(m_Window);
    }

    void Window::Minimize()
    {
        glfwIconifyWindow(m_Window);
    }

    void Window::Restore()
    {
        glfwRestoreWindow(m_Window);
    }

    void Window::SetPosition(int p_X, int p_Y)
    {
        glfwSetWindowPos(m_Window, p_X, p_Y);
    }

    void Window::SetVsync(bool p_Value)
    {
        m_Data.Vsync = p_Value;
        glfwSwapInterval(p_Value ? 1 : 0);
    }

    void Window::SetTitle(const std::string &p_Title)
    {
        glfwSetWindowTitle(m_Window, p_Title.c_str());
    }

    void Window::Resize(uint32_t p_Width, uint32_t p_Height)
    {
        if (m_Data.Mode != WindowMode::Windowed)
            return;

        glfwSetWindowSize(m_Window, (int)p_Width, (int)p_Height);
    }

    void Window::ChangeMode(WindowMode p_Mode, bool p_Maximize)
    {
        if (m_Data.Mode == p_Mode && m_Data.Maximise == p_Maximize)
            return;

        int posX = 0, posY = 0;
        glfwGetWindowPos(m_Window, &posX, &posY);

        glfwHideWindow(m_Window);

        int width            = int(m_Data.Width);
        int height           = int(m_Data.Height);
        GLFWmonitor* monitor = nullptr;

        if (p_Mode == WindowMode::Fullscreen)
            monitor = glfwGetPrimaryMonitor();

        const GLFWvidmode* mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

        if (p_Maximize)
        {
            width   = mode->width;
            height  = mode->height;
        }

        glfwSetWindowMonitor(m_Window, monitor, posX, posY, width, height, GLFW_DONT_CARE);

        glfwSetWindowAttrib(m_Window, GLFW_DECORATED, p_Mode != WindowMode::Borderless);

        if (p_Mode != WindowMode::Fullscreen && p_Maximize)
            Maximize();

        glfwShowWindow(m_Window);

        m_Data.Width  = width;
        m_Data.Height = height;
        m_Data.Mode   = p_Mode;
    }

    bool Window::IsMaximized() const
    {
        return glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED) == GLFW_TRUE;
    }

    bool Window::IsMinimized() const
    {
        return glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) == GLFW_TRUE;
    }

    Vec2 Window::GetPosition() const
    {
        int px, py;
		glfwGetWindowPos(m_Window, &px, &py);

		return Vec2(px, py);
    }

    void Window::Init(const WindowSpecification &p_Spec)
    {
        #define SET_EVENT(event)       \
        if (data.EventCallback)        \
            data.EventCallback(event); \

        m_Data.Title     = p_Spec.Title;
        m_Data.Mode      = p_Spec.Mode;
        m_Data.Resizable = p_Spec.Resizable;
        m_Data.Maximise  = p_Spec.Maximize;
        m_Data.Vsync     = p_Spec.Vsync;

        LOG("Creating window {0} ({1}, {2}) Vsync: {3}", p_Spec.Title, p_Spec.Width, p_Spec.Height, p_Spec.Vsync ? "true" : "false");

        if (s_GLFWWindowCount == 0)
        {
            int status = glfwInit();
            ASSERT(status, "Could not initialize GLFW!");
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            float xscale, yscale;
            glfwGetMonitorContentScale(monitor, &xscale, &yscale);
            m_Data.DPIScale = xscale;
        }

        {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            int width = int(p_Spec.Width);
            int height = int(p_Spec.Height);
            GLFWmonitor* monitor = nullptr;

            if (p_Spec.Mode == WindowMode::Fullscreen)
                monitor = glfwGetPrimaryMonitor();

            glfwWindowHint(GLFW_DECORATED, p_Spec.Mode != WindowMode::Borderless);

            if (p_Spec.Maximize)
            {
                auto mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());
                width = mode->width;
                height = mode->height;
            }

            if (p_Spec.Mode == WindowMode::Windowed)
                glfwWindowHint(GLFW_MAXIMIZED, p_Spec.Maximize);

            glfwWindowHint(GLFW_RESIZABLE, p_Spec.Resizable);

            m_Window = glfwCreateWindow((int)width, (int)height, p_Spec.Title.c_str(), monitor, nullptr);
            ASSERT(m_Window, "Failed to create window!");

            m_Data.Width = width;
            m_Data.Height = height;

            if (p_Spec.Center && p_Spec.Mode == WindowMode::Windowed && !p_Spec.Maximize)
            {
                auto mode = glfwGetVideoMode(monitor ? monitor : glfwGetPrimaryMonitor());

                int posX = (mode->width - p_Spec.Width) / 2;
                int posY = (mode->height - p_Spec.Height) / 2;

                glfwSetWindowPos(m_Window, posX, posY);
            }

            glfwShowWindow(m_Window);
        }
        ++s_GLFWWindowCount;
        glfwMakeContextCurrent((GLFWwindow*)m_Window);

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		ASSERT(status, "Failed to initialize glad!");
        SetVsync(m_Data.Vsync);

        glfwSetWindowUserPointer(m_Window, &m_Data);

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* p_Window, int p_Width, int p_Height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
            data.Width = p_Width;
            data.Height = p_Height;

            WindowResizeEvent event(p_Width, p_Height);
            SET_EVENT(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* p_Window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
            WindowCloseEvent event;
            SET_EVENT(event);
        });

        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* p_Window, int p_Focused)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
            if (p_Focused)
            {
                WindowFocusEvent event;
                SET_EVENT(event);
            }
            else
            {
                WindowLostFocusEvent event;
                SET_EVENT(event);
            }
        });

        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* p_Window, int p_PosX, int p_PosY)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);
            WindowMovedEvent event(p_PosX, p_PosY);
            SET_EVENT(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* p_Window, int p_Key, int p_Scancode, int p_Action, int p_Mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

            switch (p_Action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(p_Key, 0);
                    SET_EVENT(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(p_Key);
                    SET_EVENT(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(p_Key, true);
                    SET_EVENT(event);
                    break;
                }
            }
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* p_Window, unsigned int p_Keycode)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

            KeyTypedEvent event(p_Keycode);
            SET_EVENT(event);
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* p_Window, int p_Button, int p_Action, int p_Mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

            switch (p_Action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(p_Button);
                    SET_EVENT(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(p_Button);
                    SET_EVENT(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* p_Window, double p_OffsetX, double p_OffsetY)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

            MouseScrolledEvent event((float)p_OffsetX, (float)p_OffsetY);
            SET_EVENT(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* p_Window, double p_X, double p_Y)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(p_Window);

            MouseMovedEvent event((float)p_X, (float)p_Y);
            SET_EVENT(event);
        });

        glfwSetDropCallback(m_Window, [](GLFWwindow* window, int pathCount, const char* paths[])
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            std::vector<std::string> filepaths(pathCount);
            for (int i = 0; i < pathCount; i++)
                filepaths[i] = paths[i];

            WindowDropEvent event(std::move(filepaths));
            data.EventCallback(event);
        });

        #undef SET_EVENT
    }

    void Window::Shutdown() noexcept
    {
        LOG("{} window shutdown", m_Data.Title);
        glfwDestroyWindow(m_Window);
        --s_GLFWWindowCount;

        if (s_GLFWWindowCount == 0)
            glfwTerminate();
    }

} // namespace SCPY
