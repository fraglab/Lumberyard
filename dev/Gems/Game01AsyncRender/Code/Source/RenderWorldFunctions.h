// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <AzCore/std/parallel/semaphore.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class CRenderWolrdFunctions
        {
        public:
            void DoAfterAsyncRenderingCompleted(bool bCallRender);
            void BeginAsyncRendering(bool bSequentialExecutionOrder);
            void WaitForAsyncRenderingCompleted();
        private:
            AZStd::semaphore m_renderingCompletedSync { 1U, 1U };
            CCamera m_viewCamera;
        };
    }
}