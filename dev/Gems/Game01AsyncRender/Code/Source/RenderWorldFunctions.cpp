// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "RenderWorldFunctions.h"
#include <AzCore/Jobs/Job.h>
#include <AzCore/Jobs/LegacyJobExecutor.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>
#include <RenderBus.h>
#include <IGameFramework.h>
#include <IRenderer.h>
#include <IParticles.h>
#include "PerformanceWindow.h"
#include "RenderNodesManagerBus.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        static void CallWrappedRenderWorld(const CCamera& viewCamera, AZStd::semaphore& renderingCompletedSync);
        static void CallPreRender();
        static void CallRenderWorld(const CCamera& viewCamera);
        static void CallRenderWorldEnd(const CCamera& viewCamera);

        class AsyncRenderJob
            : public AZ::Job
        {
        public:
            AZ_CLASS_ALLOCATOR(AsyncRenderJob, AZ::ThreadPoolAllocator, 0);

            AsyncRenderJob(const CCamera& viewCamera, AZStd::semaphore& renderingCompletedSync, bool isAutoDelete = true, AZ::JobContext* context = nullptr)
                : AZ::Job(isAutoDelete, context),
                m_viewCamera(viewCamera),
                m_renderingCompletedSync(renderingCompletedSync)
            {}

            virtual void Process() { CallWrappedRenderWorld(m_viewCamera, m_renderingCompletedSync); }

        private:
            const CCamera& m_viewCamera;
            AZStd::semaphore& m_renderingCompletedSync;
        };

        void CallPreRender()
        {
            gEnv->p3DEngine->CallOnPreAsyncRender();
            IGameFramework* pGameFramework = gEnv->pGame->GetIGameFramework();
            if (pGameFramework)
            {
                pGameFramework->CallOnPreRenderListeners();
            }
        }

        void CallWrappedRenderWorld(const CCamera& viewCamera, AZStd::semaphore& renderingCompletedSync)
        {
            auto oldMainId = gEnv->mMainThreadId;
            gEnv->mMainThreadId = GetCurrentThreadId();
            CallRenderWorld(viewCamera);
            gEnv->mMainThreadId = oldMainId;
            renderingCompletedSync.release();
        }

        void CallRenderWorld(const CCamera& viewCamera)
        {
            CPerformanceWindow::GetInstance().StartRenderWorldTimer();
            gEnv->p3DEngine->PrepareOcclusion(viewCamera);
            // Also broadcast for anyone else that needs to draw global debug to do so now
            AzFramework::DebugDisplayEventBus::Broadcast(&AzFramework::DebugDisplayEvents::DrawGlobalDebugInfo);
            gEnv->pSystem->Render();
            CPerformanceWindow::GetInstance().StopRenderWorldTimer();
        }

        void CallRenderWorldEnd(const CCamera& viewCamera)
        {
            if (!IsEquivalent(viewCamera.GetPosition(), Vec3(0, 0, 0), VEC_EPSILON) || // never pass undefined camera to p3DEngine->RenderWorld()
                gEnv->IsDedicated() || (gEnv->pRenderer && gEnv->pRenderer->IsPost3DRendererEnabled()))
            {
                gEnv->p3DEngine->GetParticleManager()->Update();
                const auto nRenderFlags = SHDF_ALLOW_WATER | SHDF_ALLOWPOSTPROCESS | SHDF_ALLOWHDR | SHDF_ZPASS | SHDF_ALLOW_AO;
                auto passInfo = SRenderingPassInfo::CreateGeneralPassRenderingInfo(viewCamera);
                EBUS_EVENT(UnsupportedRendererNodesManagerBus, RenderUnsupportedNodes, passInfo);
                gEnv->p3DEngine->RenderSceneEnd(nRenderFlags, passInfo);
                gEnv->p3DEngine->UpdatePostRender(passInfo);
                gEnv->p3DEngine->EndOcclusion();
            }
        }

        void CRenderWolrdFunctions::DoAfterAsyncRenderingCompleted(bool bCallRender)
        {
            if (bCallRender)
            {
                CallPreRender();
                m_viewCamera = gEnv->pSystem->GetViewCamera();
                CallRenderWorld(m_viewCamera);
                CallRenderWorldEnd(m_viewCamera);
            }

            EBUS_EVENT(AZ::RenderNotificationsBus, OnScene3DEnd);
            gEnv->pRenderer->EF_RenderTextMessages();
            gEnv->p3DEngine->WorldStreamUpdate();
            gEnv->pRenderer->SwitchToNativeResolutionBackbuffer();
        }

        void CRenderWolrdFunctions::BeginAsyncRendering(bool bSequentialExecutionOrder)
        {
            CallPreRender();
            m_viewCamera = gEnv->pSystem->GetViewCamera();

            if (bSequentialExecutionOrder)
            {
                CallWrappedRenderWorld(m_viewCamera, m_renderingCompletedSync);
            }
            else
            {
                AsyncRenderJob* job = aznew AsyncRenderJob(m_viewCamera, m_renderingCompletedSync);
                job->Start();
            }
        }

        void CRenderWolrdFunctions::WaitForAsyncRenderingCompleted()
        {
            AZ_PROFILE_SCOPE(AZ::Debug::ProfileCategory::ThreeDEngine, "AsyncRenderWorld_LeadTime");
            CPerformanceWindow::GetInstance().StartMainWaitsTimer();
            m_renderingCompletedSync.acquire();
            CallRenderWorldEnd(m_viewCamera);
            CPerformanceWindow::GetInstance().StopMainWaitsTimer();
        }
    }
}
