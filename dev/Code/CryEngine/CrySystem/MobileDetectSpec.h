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

#include "AzCore/std/containers/unordered_map.h"
#include "AzCore/std/typetraits/size_t_trait_def.h"

namespace MobileSysInspect
{
    // Uses hash as key to reduce memory with larger map
    extern AZStd::unordered_map<AZStd::size_t, AZStd::string> device_spec_map;
    extern const float LOW_SPEC_RAM;
    extern const float MEDIUM_SPEC_RAM;
    extern const float HIGH_SPEC_RAM;

    void LoadDeviceSpecMapping();
    bool GetAutoDetectedSpecName(AZStd::string &buffer);
    const float GetDeviceRamInGB();
    
    namespace Internal
    {
        void LoadDeviceSpecMapping_impl(const char* fileName);
    }
}
