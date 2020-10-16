// Copyright 2020 Fraglab Ltd. All rights reserved.
#pragma once

#include <AzCore/EBus/EBus.h>
#include <I3DEngine.h>
#include <IEntityRenderState.h>

struct IRenderNode;

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class UnsupportedRendererNodesManager
            : public AZ::EBusTraits
        {
        public:
            virtual void RenderUnsupportedNodes(const SRenderingPassInfo& passInfo) = 0;
            virtual void AddToRenderList(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo, IRenderNode * pEnt) = 0;

            // EBusTraits overrides
            static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
            static const bool EnableEventQueue = true;
        };

        using UnsupportedRendererNodesManagerBus = AZ::EBus<UnsupportedRendererNodesManager>;
    }
}