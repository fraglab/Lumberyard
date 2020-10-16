// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"
#include "RenderNodesRegistratorHelper.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        void RenderNodesRegistratorHelper::RegisterInEngine(IRenderNode* pNode)
        {
            AZ_Assert(!m_bRegisterInEngine, "(AsyncRenderWorld) RegisterInEngine Recursion");
            m_bRegisterInEngine = true;
            RegisterImpl(pNode);
            m_bRegisterInEngine = false;
        }

        void RenderNodesRegistratorHelper::UnregisterInEngine(IRenderNode* pNode)
        {
            AZ_Assert(!m_bUnregisterInEngine, "(AsyncRenderWorld) UnregisterInEngine Recursion");
            m_bUnregisterInEngine = true;
            UnregisterImpl(pNode);
            m_bUnregisterInEngine = false;
        }

        bool RenderNodesRegistratorHelper::ShouldRegister()
        {
            return m_bRegisterInEngine;
        }

        bool RenderNodesRegistratorHelper::ShouldUnregister()
        {
            return m_bUnregisterInEngine;
        }

        void RenderNodesRegistratorHelper::RegisterImpl(IRenderNode* pNode)
        {
            gEnv->p3DEngine->RegisterEntity(pNode);
        }

        void RenderNodesRegistratorHelper::UnregisterImpl(IRenderNode* pNode)
        {
            gEnv->p3DEngine->FreeRenderNodeState(pNode);
        }
    } // namespace Game01AsyncRender
}     // namespace Fraglab
