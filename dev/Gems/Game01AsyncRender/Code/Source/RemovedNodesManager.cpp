// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "RemovedNodesManager.h"
#include <AzCore/Jobs/Algorithms.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        bool CRemovedNodesManager::IsEmpty() const
        {
            return m_removedNodes[0].empty() && m_removedNodes[1].empty();
        }

        bool CRemovedNodesManager::Contains(IRenderNode* pRenderNode) const
        {
            return m_removedNodes[m_fillArrayId].find(pRenderNode) != m_removedNodes[m_fillArrayId].end();
        }

        void CRemovedNodesManager::CleanRemovedRenderNodes()
        {
            m_removedNodes[m_processArrayId].clear();
            AZStd::swap(m_processArrayId, m_fillArrayId);
        }

        void CRemovedNodesManager::EraseNode(IRenderNode* pRenderNode)
        {
            for (int i = 0; i < DoubleBufferSize; ++i)
            {
                auto itRemoveNode = m_removedNodes[i].find(pRenderNode);
                if (itRemoveNode != m_removedNodes[i].end())
                {
                    m_removedNodes[i].erase(itRemoveNode);
                }
            }
        }

        void CRemovedNodesManager::Insert(IRenderNode* pRenderNode)
        {
            m_removedNodes[m_fillArrayId].insert(pRenderNode);
        }

        void CRemovedNodesManager::DoForEachInFillArray(TRenderNodeFunction renderNodeFunction)
        {
            DoForEachInArray(renderNodeFunction, m_fillArrayId);
        }

        void CRemovedNodesManager::DoForEachInProcessArray(TRenderNodeFunction renderNodeFunction)
        {
            DoForEachInArray(renderNodeFunction, m_processArrayId);
        }

        void CRemovedNodesManager::DoForEachInArray(TRenderNodeFunction renderNodeFunction, int index)
        {
            AZStd::for_each(m_removedNodes[index].begin(), m_removedNodes[index].end(), renderNodeFunction);
        }
    }
}
