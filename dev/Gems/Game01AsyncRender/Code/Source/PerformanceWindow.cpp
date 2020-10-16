// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "PerformanceWindow.h"

#ifdef IMGUI_ENABLED
    #include <imgui/imgui.h>
#endif // #ifdef IMGUI_ENABLED

namespace Fraglab
{
    namespace Game01AsyncRender
    {

        CPerformanceWindow& CPerformanceWindow::GetInstance()
        {
            static CPerformanceWindow s_singleton;
            return s_singleton;
        }

        void DrawFrameWorkloadBar(float value)
        {
            // Interpolate the color of the bar depending on the load.
            const float fvalue = AZStd::clamp(value, 0.0f, 1.0f);
            static const AZ::Vector3 lowHSV(96.0f / 255.0f, 100.0f / 100.0f, 100.0f / 100.0f);
            static const AZ::Vector3 highHSV(0.0f / 255.0f, 68.0f / 100.0f, 100.0f / 100.0f);
            const AZ::Vector3 colorHSV = lowHSV + (highHSV - lowHSV) * fvalue;

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, (ImVec4)ImColor::HSV(colorHSV.GetX(), colorHSV.GetY(), colorHSV.GetZ()));
            ImGui::ProgressBar(fvalue, ImVec2(0.0f, 0.0f));
            ImGui::PopStyleColor(1);
        }

        void CPerformanceWindow::Show()
        {
#ifdef IMGUI_ENABLED
            if (!m_bEnabled)
            {
                return;
            }
            if (ImGui::Begin("AsyncRender Performance", &m_bEnabled, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
            {
                m_frameTime = GetSmoothedValue(m_frameTime, gEnv->pTimer->GetRealFrameTime() * 1000.0f);
                ImGui::Text("Frame %3.1f", m_frameTime);

                ImGui::ProgressBar(m_renderWorldTime / m_frameTime, ImVec2(0.0f, 0.0f));
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("RenderWorld %3.1f", m_renderWorldTime);

                DrawFrameWorkloadBar(m_mainWaitsTime / m_frameTime);
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Main waits AsyncRender %3.1f", m_mainWaitsTime);

                DrawFrameWorkloadBar(m_unsupportedNodesTime / m_frameTime);
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Unsupported nodes %3.1f", m_unsupportedNodesTime);

                DrawFrameWorkloadBar(m_copyTime / m_frameTime);
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                ImGui::Text("Copy data %3.1f", m_copyTime);

                ImGui::Checkbox("Smooth", &m_bSmooth);
            }
            ImGui::End();
#endif // #ifdef IMGUI_ENABLED
        }

        float CPerformanceWindow::GetSmoothedValue(float prevValue, float newValue)
        {
            if (m_bSmooth)
            {
                const float m = 0.1f;
                return (1.0f - m) * prevValue + m * newValue;
            }
            else
            {
                return newValue;
            }
        }

        void CPerformanceWindow::ToogleEnabled()
        {
            m_bEnabled = !m_bEnabled;
        }

        void CPerformanceWindow::StartCopyTimer()
        {
            m_copyTimer.Stamp();
        }

        void CPerformanceWindow::StopCopyTimer()
        {
            m_copyTime = GetSmoothedValue(m_copyTime, m_copyTimer.GetDeltaTimeInSeconds() * 1000.0f);
        }

        void CPerformanceWindow::StartUnsupportedNodesTimer()
        {
            m_unsupportedNodesTimer.Stamp();
        }
        void CPerformanceWindow::StopUnsupportedNodesTimer()
        {
            m_unsupportedNodesTime = GetSmoothedValue(m_unsupportedNodesTime, m_unsupportedNodesTimer.GetDeltaTimeInSeconds() * 1000.0f);
        }

        void CPerformanceWindow::StartRenderWorldTimer()
        {
            m_renderWorldTimer.Stamp();
        }

        void CPerformanceWindow::StopRenderWorldTimer()
        {
            m_renderWorldTime = GetSmoothedValue(m_renderWorldTime, m_renderWorldTimer.GetDeltaTimeInSeconds() * 1000.0f);
        }

        void CPerformanceWindow::StartMainWaitsTimer()
        {
            m_mainWaitsTimer.Stamp();
        }

        void CPerformanceWindow::StopMainWaitsTimer()
        {
            m_mainWaitsTime = GetSmoothedValue(m_mainWaitsTime, m_mainWaitsTimer.GetDeltaTimeInSeconds() * 1000.0f);
        }
    }
}
