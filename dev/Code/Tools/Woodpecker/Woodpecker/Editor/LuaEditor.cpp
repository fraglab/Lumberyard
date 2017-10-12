/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "stdafx.h"
#include "LuaEditor.h"
#include <Woodpecker/LuaIDEApplication.h>

#if defined(EXTERNAL_CRASH_REPORTING)
#include <CrashHandler.h>
#endif

// Editor.cpp : Defines the entry point for the application.

int _tmain(int argc, _TCHAR* argv[])
{
    (void)argc;
    (void)argv;
    // here we free the console (and close the console window) in release.

    int exitCode = 0;

    {
        if (!AZ::AllocatorInstance<AZ::OSAllocator>::IsReady())
        {
            AZ::AllocatorInstance<AZ::OSAllocator>::Create();
        }

        AZStd::unique_ptr<AZ::IO::LocalFileIO> fileIO = AZStd::unique_ptr<AZ::IO::LocalFileIO>(aznew AZ::IO::LocalFileIO());
        AZ::IO::FileIOBase::SetInstance(fileIO.get());

        LUAEditor::Application app;

        char procPath[256];
        char procName[256];
        DWORD ret = GetModuleFileNameA(NULL, procPath, 256);
        if (ret > 0)
        {
            ::_splitpath_s(procPath, 0, 0, 0, 0, procName, 256, 0, 0);
        }

        LegacyFramework::ApplicationDesc desc(procName);
        desc.m_applicationModule = NULL;
        desc.m_enableProjectManager = false;

#if defined(EXTERNAL_CRASH_REPORTING)
        InitCrashHandler("LuaEditor", {});
#endif

        exitCode = app.Run(desc);
        // this call will block until someone tells the core app to shut down via a bus message.
        // the bus message is usually sent (in gui mode) in response to pressing the quit button or something.
        // in an app that does not require GUI to be manufactured or use GUI windows, you should still call RUN
        // but make a component which does your processing, in response to RestoreState(). in the  CoreMessages bus.
        // RestoreState will always be called right before the main message pump activates.
        // and will then block until someone calls:
        /*EBUS_EVENT(UIFramework::FrameworkMessages::Bus, UserWantsToQuit); */
        // so ideally to make a file processor or something, simply call app.Initialize(.... but with false as the gui mode ... )
        // and make at least one component which starts processing in response to CoreMessages::RestoreState(), and then sends UserWantsToQuit() once it has done its processing.
        // calling UserWantsToQuit will simply queue the quit, so its safe to call from any thread.
        // your components can query EBUS_EVENT_RESULT(res, LegacyFramework::FrameworkApplicationMessages::IsRunningInGUIMode) to determine
        // if its in GUI mode or not.
    }

    return exitCode;
}
