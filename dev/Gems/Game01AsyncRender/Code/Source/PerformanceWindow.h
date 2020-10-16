// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <AzCore/Debug/Timer.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class CPerformanceWindow
        {
        public:
            CPerformanceWindow(CPerformanceWindow&) = delete;
            void operator=(CPerformanceWindow&) = delete;
            static CPerformanceWindow& GetInstance();

            void Show();
            void ToogleEnabled();
            void StartCopyTimer();
            void StopCopyTimer();
            void StartUnsupportedNodesTimer();
            void StopUnsupportedNodesTimer();
            void StartRenderWorldTimer();
            void StopRenderWorldTimer();
            void StartMainWaitsTimer();
            void StopMainWaitsTimer();

        private:
            CPerformanceWindow() = default;
            float GetSmoothedValue(float prevValue, float newValue);

            bool m_bEnabled { false };
            bool m_bSmooth { true };
            AZ::Debug::Timer m_copyTimer;
            AZ::Debug::Timer m_unsupportedNodesTimer;
            AZ::Debug::Timer m_renderWorldTimer;
            AZ::Debug::Timer m_mainWaitsTimer;

            float m_copyTime { 0.0f };
            float m_unsupportedNodesTime { 0.0f };
            float m_renderWorldTime { 0.0f };
            float m_mainWaitsTime { 0.0f };
            float m_frameTime { 16.6f };
        };
    }
}
