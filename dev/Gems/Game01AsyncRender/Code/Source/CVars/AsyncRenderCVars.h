// Copyright 2020 Fraglab Ltd. All rights reserved.

#pragma once

#include <AzCore/Component/Component.h>
#include <ISystem.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class Game01AsyncRenderCVars;
        extern Game01AsyncRenderCVars* g_pGame01AsyncRenderCVars;

        class Game01AsyncRenderCVars
            : public AZ::Component
            , private CrySystemEventBus::Handler
        {
        public:
            AZ_COMPONENT(Game01AsyncRenderCVars, "{6E60C0F3-6BBB-4499-A749-9EDA99EAC1FB}");

            static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
            static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        private:

            static void Reflect(AZ::ReflectContext * pContext);

            // AZ::Component interface implementation
            virtual void Activate() override;
            virtual void Deactivate() override;

            virtual void OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams) override;
            void OnCrySystemShutdown(ISystem& system) override;
            void RegisterCVars();
            void UnregisterCVars();

            static void CmdLoad(IConsoleCmdArgs* pArgs);
            static void CmdDisable(IConsoleCmdArgs* pArgs);
        public:
            int asyncRender_enable { 0 };
        };
    }
}
