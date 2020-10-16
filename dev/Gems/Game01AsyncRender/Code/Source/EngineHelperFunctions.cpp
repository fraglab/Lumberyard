// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "EngineHelperFunctions.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        namespace EngineHelpers
        {
            void DisableDeferredAudio()
            {
                auto pConsole = gEnv->pConsole;
                if (pConsole)
                {
                    auto pAudioUpdateOptim = pConsole->GetCVar("sys_deferAudioUpdateOptim");
                    if (pAudioUpdateOptim)
                    {
                        pAudioUpdateOptim->Set(0);
                    }
                }
            }
        }
    }
}