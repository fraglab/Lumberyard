// Copyright 2019 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"
#include <SystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include "RenderWorldFunctions.h"
#include "EngineHelperFunctions.h"
#include "PerformanceWindow.h"
#include "CVars/AsyncRenderCVars.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        void SystemComponent::Reflect(AZ::ReflectContext* context)
        {
            if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serialize->Class<SystemComponent, AZ::Component>()
                ->Version(0);

                if (AZ::EditContext* pEditContext = serialize->GetEditContext())
                {
                    pEditContext->Class<SystemComponent>("Game01AsyncRender", "Execute RenderWorld in worker thread.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true);
                }
            }
        }

        void SystemComponent::Activate()
        {
            AsyncRenderWorldRequestBus::Handler::BusConnect();
            AsyncRenderWorldSynchronizationRequestBus::Handler::BusConnect();
            AsyncRenderWorldNodeRemoveRequestBus::Handler::BusConnect();
            CrySystemEventBus::Handler::BusConnect();
        }

        void SystemComponent::Deactivate()
        {
            AsyncRenderWorldRequestBus::Handler::BusDisconnect();
            AsyncRenderWorldSynchronizationRequestBus::Handler::BusDisconnect();
            AsyncRenderWorldNodeRemoveRequestBus::Handler::BusDisconnect();
            CrySystemEventBus::Handler::BusDisconnect();

            if (!m_enabled)
            {
                return;
            }

#if defined(IMGUI_ENABLED)
            ImGui::ImGuiUpdateListenerBus::Handler::BusDisconnect();
#endif
            AZ::TickBus::Handler::BusDisconnect();
            Replay::ReplayEventBus::Handler::BusDisconnect();
        }

#ifdef IMGUI_ENABLED
        void SystemComponent::OnImGuiMainMenuUpdate()
        {
            if (m_enabled && ImGui::BeginMenu("AsyncRender"))
            {
                ImGui::Checkbox("Sequential order", &m_sequentialExecutionOrder);
                if (ImGui::Button("Show Performance Window"))
                {
                    CPerformanceWindow::GetInstance().ToogleEnabled();
                }
                m_nodesManager.AddImGuiMenu();
                ImGui::EndMenu();
            }

            CPerformanceWindow::GetInstance().Show();
        }
#endif  // IMGUI_ENABLED

        void SystemComponent::OnTick(float deltaTime, AZ::ScriptTimePoint time)
        {
            if (!m_enabled || m_bLevelLoading)
            {
                return;
            }

            m_bShouldWaitForAsyncRendering = true;
            m_nodesManager.StartAddingUnsupportedNodesToRenderList();
            m_renderWorldFunctions.BeginAsyncRendering(m_sequentialExecutionOrder);
        }

        int SystemComponent::GetTickOrder()
        {
            return AZ::ComponentTickBus::TICK_INPUT + 1;
        }

        bool SystemComponent::IsEnabled() const
        {
            return m_enabled;
        }

        void SystemComponent::WaitRenderingCompleted()
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            if (m_enabled && m_bShouldWaitForAsyncRendering)
            {
                m_bShouldWaitForAsyncRendering = false;
                m_renderWorldFunctions.WaitForAsyncRenderingCompleted();
            }
        }

        void SystemComponent::DoAfterAsyncRenderingCompleted()
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            if (!m_enabled)
            {
                return;
            }

            m_renderWorldFunctions.DoAfterAsyncRenderingCompleted(m_bLevelLoading);
        }

        static bool isEnabledAsyncRender();

        void SystemComponent::LoadConfiguration()
        {
            m_enabled = isEnabledAsyncRender();

            if (m_enabled)
            {
#if defined(IMGUI_ENABLED)
                ImGui::ImGuiUpdateListenerBus::Handler::BusConnect();
#endif
                AZ::TickBus::Handler::BusConnect();
                Replay::ReplayEventBus::Handler::BusConnect();
                EngineHelpers::DisableDeferredAudio();
            }
        }

        bool isEnabledAsyncRender()
        {
            bool bSystemHasManyCores = true;
#if defined(AZ_PLATFORM_WINDOWS)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            DWORD minProcessors = 4;
            bSystemHasManyCores = sysinfo.dwNumberOfProcessors >= minProcessors;
#endif
            return !!g_pGame01AsyncRenderCVars->asyncRender_enable && !gEnv->IsEditor() && bSystemHasManyCores;
        }

        void SystemComponent::DeleteRenderNode(const IRenderNode* pTickNode)
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            if (!m_enabled)
            {
                return;
            }

            m_nodesManager.DeleteRenderNode(pTickNode);
        }

        bool SystemComponent::ChangeRenderNodeRegisteredState(IRenderNode* pRenderNode, bool bRegister)
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            const bool bDoOctreeModification = false;

            if (!m_enabled)
            {
                return bDoOctreeModification;
            }

            return m_nodesManager.ChangeRenderNodeRegisteredState(pRenderNode, bRegister);
        }

        void SystemComponent::ProcessAsyncRenderNodes()
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            if (!m_enabled)
            {
                return;
            }

            m_nodesManager.ProcessAsyncRenderNodes();
        }

        void SystemComponent::CleanRemovedRenderNodes()
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");

            if (!m_enabled)
            {
                return;
            }

            m_nodesManager.CleanRemovedRenderNodes();
        }

        void SystemComponent::OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams)
        {
            system.GetISystemEventDispatcher()->RegisterListener(this);
        }

        void SystemComponent::OnCrySystemShutdown(ISystem& system)
        {
            system.GetISystemEventDispatcher()->RemoveListener(this);
        }

        void SystemComponent::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
        {
            switch (event)
            {
            case ESYSTEM_EVENT_LEVEL_LOAD_END:
                m_bLevelLoading = false;
                m_nodesManager.SetLevelLoading(m_bLevelLoading);
                break;
            case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
                m_nodesManager.CheckNodesRemovedAfterLevelUnload();
                break;

            default:
                break;
            }
        }

        void SystemComponent::ReplayStart()
        {
            LevelUnloadStart();
        }

        void SystemComponent::LevelUnloadStart()
        {
            AZ_Assert(gEnv->mTickThreadId == GetCurrentThreadId(), "(AsyncRenderWorld) Thread access violation");
            m_bLevelLoading = true;
            m_nodesManager.SetLevelLoading(m_bLevelLoading);
        }
    }
}
