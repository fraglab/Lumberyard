// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

struct IRenderNode;

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class RenderNodesRegistratorHelper
        {
        public:
            void RegisterInEngine(IRenderNode* pNode);
            void UnregisterInEngine(IRenderNode* pNode);
            bool ShouldRegister();
            bool ShouldUnregister();
        private:
            void RegisterImpl(IRenderNode* pNode);
            void UnregisterImpl(IRenderNode* pNode);

            bool m_bRegisterInEngine { false };
            bool m_bUnregisterInEngine { false };
        };
    }
}
