#pragma once
#include "Events/Event.h"


struct ImGuiContext;

namespace SCPY
{
    class ImGuiLayer
    {
        public:
            ImGuiLayer();
            ~ImGuiLayer();

            void BlockEvents(bool p_Block) { m_BlockEvents = p_Block; }

            void Begin();
            void End();

            void OnEvent(Event& p_Event);
            void OnUpdate(float p_Dt);

            ImGuiContext* GetCurrentContext();

            uint32_t GetActiveWidgetID() const;

        private:
            void AddFonts();

        private:
            float m_FontSize    = 14.0f;
            bool  m_BlockEvents = false;
    };

} // namespace SCPY