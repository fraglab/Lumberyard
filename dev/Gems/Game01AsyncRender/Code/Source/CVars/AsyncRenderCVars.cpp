// Copyright 2020 Fraglab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"
#include "AsyncRenderCVars.h"

#include <AzCore/Serialization/SerializeContext.h>
#include <AzFramework/API/ApplicationAPI.h>
#include <AzFramework/CommandLine/CommandLine.h>
#include <AzGameFramework/FragLab/AsyncRender/AsyncRenderWorldRequestBus.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        Game01AsyncRenderCVars* g_pGame01AsyncRenderCVars = nullptr;

        void Game01AsyncRenderCVars::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC("Game01AsyncRenderCVarsService"));
        }

        void Game01AsyncRenderCVars::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC("Game01AsyncRenderCVarsService"));
        }

        void Game01AsyncRenderCVars::Activate()
        {
            CrySystemEventBus::Handler::BusConnect();
        }

        void Game01AsyncRenderCVars::Deactivate()
        {
            CrySystemEventBus::Handler::BusDisconnect();
        }

        void Game01AsyncRenderCVars::OnCrySystemInitialized(ISystem& system, const SSystemInitParams& systemInitParams)
        {
            g_pGame01AsyncRenderCVars = this;
            RegisterCVars();
        }

        void Game01AsyncRenderCVars::OnCrySystemShutdown(ISystem& system)
        {
            UnregisterCVars();
        }

        void Game01AsyncRenderCVars::CmdLoad(IConsoleCmdArgs* pArgs)
        {
            g_pGame01AsyncRenderCVars->asyncRender_enable = 1;
            EBUS_EVENT(AsyncRenderWorldSynchronizationRequestBus, LoadConfiguration);
        }

        void Game01AsyncRenderCVars::CmdDisable(IConsoleCmdArgs* pArgs)
        {
            g_pGame01AsyncRenderCVars->asyncRender_enable = 0;
            EBUS_EVENT(AsyncRenderWorldSynchronizationRequestBus, LoadConfiguration);
        }

        void Game01AsyncRenderCVars::Reflect(AZ::ReflectContext * pContext)
        {
            if (AZ::SerializeContext * pSerializeContext = azrtti_cast<AZ::SerializeContext*>(pContext))
            {
                pSerializeContext->Class<Game01AsyncRenderCVars, AZ::Component>()->Version(0);
            }
        }

        void Game01AsyncRenderCVars::RegisterCVars()
        {
            if (gEnv && !gEnv->IsEditor())
            {
                REGISTER_COMMAND("asyncRender_load", CmdLoad, VF_DEV_ONLY, "Enable AsyncRender. Debug only.");
                REGISTER_COMMAND("asyncRender_forceDisable", CmdDisable, VF_NULL, "Disable AsyncRender");
                REGISTER_CVAR2("asyncRender_enable", &asyncRender_enable, 0,
                        VF_NULL, "Enables render world execution worker thread.");
            }
        }

        void Game01AsyncRenderCVars::UnregisterCVars()
        {
            UNREGISTER_COMMAND("asyncRender_load");
            UNREGISTER_COMMAND("asyncRender_forceDisable");
            UNREGISTER_CVAR("asyncRender_enable");
        }
    }
}