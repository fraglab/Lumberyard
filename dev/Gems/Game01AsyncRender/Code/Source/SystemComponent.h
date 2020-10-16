// Copyright 2019 FragLab Ltd. All rights reserved.

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/set.h>
#include <AzGameFramework/FragLab/AsyncRender/AsyncRenderWorldRequestBus.h>
#include <AzGameFramework/FragLab/Replay/ReplayBus.h>
#include <IAsyncRenderWorldNodeRemove.h>
#include "RenderNodesManager.h"
#include "RenderWorldFunctions.h"

#ifdef IMGUI_ENABLED
    #include <imgui/imgui.h>
    #include <ImGuiBus.h>
#endif // #ifdef IMGUI_ENABLED

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class SystemComponent
            : public AZ::Component
            , public ISystemEventListener
            , protected CrySystemEventBus::Handler
            , private AZ::TickBus::Handler
            , private AsyncRenderWorldRequestBus::Handler
            , private AsyncRenderWorldSynchronizationRequestBus::Handler
            , private AsyncRenderWorldNodeRemoveRequestBus::Handler
            , public Replay::ReplayEventBus::Handler
#ifdef IMGUI_ENABLED
            , public ImGui::ImGuiUpdateListenerBus::Handler
#endif     // IMGUI_ENABLED
        {
        public:
            AZ_COMPONENT(SystemComponent, "{899D8833-16CF-40CE-A956-1BDC15ED9822}");

            SystemComponent() = default;
            virtual ~SystemComponent() override = default;

            static void Reflect(AZ::ReflectContext* context);

#ifdef IMGUI_ENABLED
            void OnImGuiMainMenuUpdate() override;
#endif // IMGUI_ENABLED

        protected:
            SystemComponent(const SystemComponent&) = delete;

            ////////////////////////////////////////////////////////////////////////
            // AZ::Component interface implementation
            void Activate() override;
            void Deactivate() override;
            ////////////////////////////////////////////////////////////////////////
            // CrySystemEventOrderedBus
            void OnCrySystemInitialized(ISystem&, const SSystemInitParams&) override;
            void OnCrySystemShutdown(ISystem& system) override;
            // Multiplayer::ReplayEventBus
            void ReplayStart() override;

        private:
            // AZ::TickBus::Handler
            void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;
            int GetTickOrder() override;

            // AsyncRenderWorldRequestBus
            bool IsEnabled() const override;

            // AsyncRenderWorldSynchronizationRequestBus
            void WaitRenderingCompleted() override;
            void DoAfterAsyncRenderingCompleted() override;
            void ProcessAsyncRenderNodes() override;
            void CleanRemovedRenderNodes() override;
            void LoadConfiguration() override;
            bool ChangeRenderNodeRegisteredState(IRenderNode* pRenderNode, bool bRegister) override;
            void LevelUnloadStart() override;
            // AsyncRenderWorldNodeRemoveRequestBus
            void DeleteRenderNode(const IRenderNode* pTickNode) override;

            // ISystemEventListener
            void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;

            bool m_enabled = false;
            bool m_bLevelLoading = true;
            bool m_bShouldWaitForAsyncRendering = false;

            CRenderWolrdFunctions m_renderWorldFunctions;
            CRenderNodesManager m_nodesManager {m_bLevelLoading};

            bool m_sequentialExecutionOrder = false;
        };
    }
}
