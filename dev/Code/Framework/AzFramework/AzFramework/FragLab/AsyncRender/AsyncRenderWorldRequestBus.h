// Copyright 2020 FragLab Ltd. All rights reserved.
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/std/parallel/mutex.h>

struct IRenderNode;

namespace Fraglab
{
    /**
     * Used to control and configure RenderWorld as asynchronous job
     */
    class AsyncRenderWorldEvents
        : public AZ::EBusTraits
    {
    public:
        virtual bool IsEnabled() const = 0;

        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const bool EnableEventQueue = true;
        typedef AZStd::mutex MutexType;
    };

    using AsyncRenderWorldRequestBus = AZ::EBus<AsyncRenderWorldEvents>;

    class AsyncRenderWorldSynchronization
        : public AZ::EBusTraits
    {
    public:

        virtual void WaitRenderingCompleted() = 0;
        virtual void DoAfterAsyncRenderingCompleted() = 0;
        virtual void LoadConfiguration() = 0;
        virtual void ProcessAsyncRenderNodes() = 0;
        virtual void CleanRemovedRenderNodes() = 0;
        virtual bool ChangeRenderNodeRegisteredState(IRenderNode * pRenderNode, bool bRegister) = 0;
        virtual void DeleteRenderNode(const IRenderNode* pTickNode) = 0;
        virtual void LevelUnloadStart() = 0;


        // EBusTraits overrides
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static const bool EnableEventQueue = true;
    };

    using AsyncRenderWorldSynchronizationRequestBus = AZ::EBus<AsyncRenderWorldSynchronization>;
}

#ifdef USE_ASYNC_RENDER

    #define ASYNC_RENDER_VAR_NAME_HELPER(X,Y) X##Y
    #define ASYNC_RENDER_VAR_NAME(X,Y) ASYNC_RENDER_VAR_NAME_HELPER(X,Y)
    #define ASYNC_RENDER_ENABLED_BEGIN \
            bool ASYNC_RENDER_VAR_NAME(bAsyncRenderingEnabled, __LINE__) = false; \
            EBUS_EVENT_RESULT(ASYNC_RENDER_VAR_NAME(bAsyncRenderingEnabled, __LINE__), Fraglab::AsyncRenderWorldRequestBus, IsEnabled); \
            if (! ASYNC_RENDER_VAR_NAME(bAsyncRenderingEnabled, __LINE__) ) \
            {

    #define ASYNC_RENDER_ENABLED_END }

    #define ASYNC_RENDER_IF_ENABLED_BEGIN \
        bool bAsyncRenderingEnabled = false; \
        EBUS_EVENT_RESULT(bAsyncRenderingEnabled, Fraglab::AsyncRenderWorldRequestBus, IsEnabled); \
        if (bAsyncRenderingEnabled) \
        {

#else

#define ASYNC_RENDER_IF_ENABLED_BEGIN
#define ASYNC_RENDER_ENABLED_BEGIN
#define ASYNC_RENDER_ENABLED_END

#endif

