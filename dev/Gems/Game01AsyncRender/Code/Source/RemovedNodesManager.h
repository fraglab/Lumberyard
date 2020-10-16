// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <AzCore/std/containers/set.h>

struct IRenderNode;

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class CRemovedNodesManager
        {
        public:
            bool IsEmpty() const;
            bool Contains(IRenderNode* pRenderNode) const;
            void CleanRemovedRenderNodes();
            void EraseNode(IRenderNode* pRenderNode);
            void Insert(IRenderNode* pRenderNode);
            using TRenderNodeFunction = AZStd::function<void(IRenderNode*)>;
            void DoForEachInFillArray(TRenderNodeFunction renderNodeFunction);
            void DoForEachInProcessArray(TRenderNodeFunction renderNodeFunction);
        private:

            void DoForEachInArray(TRenderNodeFunction renderNodeFunction, int index);
            // List of nodes to be removed in the end of a frame
            static const int DoubleBufferSize { 2 };
            AZStd::set<IRenderNode*> m_removedNodes[DoubleBufferSize];
            int m_fillArrayId = 0;
            int m_processArrayId = 1;
        };
    }
}
