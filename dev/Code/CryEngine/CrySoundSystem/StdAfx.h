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
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#pragma once

//#define RWI_NAME_TAG "RayWorldIntersection(Audio)"
//#define PWI_NAME_TAG "PrimitiveWorldIntersection(Audio)"

#include <platform.h>
#include <AzCore/Debug/Trace.h>
#include <ProjectDefines.h>
#include <StlUtils.h>

#if !defined(_RELEASE)
    // Enable logging for non-Release builds.
    #define ENABLE_AUDIO_LOGGING
    // Production code is enabled in non-Release builds.
    #define INCLUDE_AUDIO_PRODUCTION_CODE
#endif // _RELEASE


#if defined(AZ_PLATFORM_WINDOWS)
    #include <windows.h>

#endif


// iOS
///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(AZ_PLATFORM_APPLE_IOS)
    #undef INCLUDE_AUDIO_PRODUCTION_CODE
#endif // iOS

// AppleTV
///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(AZ_PLATFORM_APPLE_TV)
    #undef INCLUDE_AUDIO_PRODUCTION_CODE
#endif


#include <AudioAllocators.h>
#include <AudioLogger.h>
#include <AudioInternalInterfaces.h>

namespace Audio
{
    extern CAudioLogger g_audioLogger;
} // namespace Audio
