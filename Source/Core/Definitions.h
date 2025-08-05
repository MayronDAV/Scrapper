#pragma once
#include <string>
#include <cstdint>


namespace SCPY
{
    enum class WindowMode : uint8_t
	{
		Windowed = 0,
		Fullscreen,
		Borderless
	};

    struct Vec2
    {    
        float x, y;

        Vec2(float p_X, float p_Y) : x(p_X), y(p_Y) {}
        Vec2(float p_Scalar) : x(p_Scalar), y(p_Scalar) {}
    };

    struct WindowSpecification
    {
        std::string Title    = "Scrapper";
        uint32_t Width       = 800;
        uint32_t Height      = 600;
        WindowMode Mode      = WindowMode::Windowed;
        bool Resizable       = true;
        bool Maximize        = false;
        bool Center          = true;
        bool Vsync           = false;
        std::string IconPath = "";
    };
} // namespace SCPY
