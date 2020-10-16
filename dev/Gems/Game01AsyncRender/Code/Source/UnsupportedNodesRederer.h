// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <IEntityRenderState.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        struct IUnsupportedNodesRenderer
        {
            virtual void AddToRenderList(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo, IRenderNode * pEnt) = 0;
        };
    }
}
