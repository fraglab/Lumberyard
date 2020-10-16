// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#ifdef USE_ASYNC_RENDER
#include <AzCore/EBus/EBus.h>
struct IRenderNode;
namespace Fraglab
{
    class AsyncRenderWorldNodeRemove
        : public AZ::EBusTraits
    {
    public:
        virtual void DeleteRenderNode(const IRenderNode* pTickNode) = 0;
        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const bool EnableEventQueue = true;
    };

    using AsyncRenderWorldNodeRemoveRequestBus = AZ::EBus<AsyncRenderWorldNodeRemove>;
}
#endif