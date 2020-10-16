// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "RenderNodesManager.h"

#include "MockAsyncRenderNode.h"
#include "PerformanceWindow.h"
#include <AzCore/Jobs/Algorithms.h>
#ifdef IMGUI_ENABLED
    #include <imgui/imgui.h>
#endif // #ifdef IMGUI_ENABLED

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        CRenderNodesManager::CRenderNodesManager(bool bLevelLoading)
            : m_bLevelLoading(bLevelLoading)
        {
            UnsupportedRendererNodesManagerBus::Handler::BusConnect();
            m_synchronizationArray.reserve(32768);
        }

        CRenderNodesManager::~CRenderNodesManager()
        {
            UnsupportedRendererNodesManagerBus::Handler::BusDisconnect();
        }

        void CRenderNodesManager::ProcessAsyncRenderNodes()
        {
            AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::ThreeDEngine);

            CPerformanceWindow::GetInstance().StartCopyTimer();

            if (m_bParallelCopy)
            {
                AZ::parallel_for_each(m_synchronizationArray.begin(), m_synchronizationArray.end(), [](auto& tickAndFillPair)
                {
                    if (tickAndFillPair.second.flag == ENodeAction::Update)
                    {
                        HandleNodeUpdate(tickAndFillPair);
                    }
                });
            }
            else
            {
                for (auto& tickAndFillPair : m_synchronizationArray)
                {
                    if (tickAndFillPair.second.flag == ENodeAction::Update)
                    {
                        HandleNodeUpdate(tickAndFillPair);
                    }
                }
            }

            for (auto& tickAndFillPair : m_synchronizationArray)
            {
                HandleNode(tickAndFillPair);
            }

            CPerformanceWindow::GetInstance().StopCopyTimer();

            m_removedNodes.DoForEachInFillArray([this](IRenderNode* pNode)
            {
                if (m_bUnregisterNodesOnDelete || m_unregisterNodes.find(pNode) != m_unregisterNodes.end())
                {
                    m_registratorHelper.UnregisterInEngine(pNode);
                }
            });
        }

        void CRenderNodesManager::CleanRemovedRenderNodes()
        {
            AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::ThreeDEngine);

            m_removedNodes.DoForEachInProcessArray([this](IRenderNode* pNode) { HandleNodeRemove(pNode); });

            m_removedNodes.CleanRemovedRenderNodes();
            m_unregisterNodes.clear();
        }

        inline bool operator<(const CRenderNodesManager::TSynchPair& a, const CRenderNodesManager::TSynchPair& b)
        {
            return static_cast<const void*>(a.first) < static_cast<const void*>(b.first);
        }

        CRenderNodesManager::TFindSynchronizedNodeResult CRenderNodesManager::FindSynchronizedRenderNode(const IRenderNode* pTickNode)
        {
            if (m_synchronizationArray.empty())
            {
                return TFindSynchronizedNodeResult { false, m_synchronizationArray.end() };
            }
            SFillNode empty;
            TSynchPair searchEl(const_cast<IRenderNode*>(pTickNode), empty);
            auto it = AZStd::lower_bound(m_synchronizationArray.begin(), m_synchronizationArray.end(), searchEl);
            return TFindSynchronizedNodeResult { (it != m_synchronizationArray.end()) && (it->first == (const void*)pTickNode), it };
        }

        void CRenderNodesManager::DeleteRenderNode(const IRenderNode* pTickNode)
        {
            if (m_bHandlingNodeRemove)
            {
                return;
            }

            if (m_bActionsLogging)
            {
                AZ_TracePrintf("AsyncRender", "Delete RenderNode: %p", static_cast<const void*>(pTickNode));
            }

            auto findResult = FindSynchronizedRenderNode(pTickNode);
            auto& it = findResult.second;
            if (findResult.first)
            {
                DeleteClonedNode(it);
                m_synchronizationArray.erase(it);
            }
            else
            {
                auto ownedNodesIt = m_ownedNodes.find(const_cast<IRenderNode*>(pTickNode));
                if (ownedNodesIt != m_ownedNodes.end())
                {
                    auto findResult = FindSynchronizedRenderNode(ownedNodesIt->second);
                    if (findResult.first)
                    {
                        auto& itPairToRemove = findResult.second;
                        AZ_Assert(m_bLevelLoading || itPairToRemove->second.flag == ENodeAction::OutOfAction, "(AsyncRenderWorld) Unexpected delete of IRenderNode");
                        m_synchronizationArray.erase(itPairToRemove);
                    }

                    m_removedNodes.EraseNode(ownedNodesIt->first);
                    m_ownedNodes.erase(ownedNodesIt);
                }
            }
        }

        void CRenderNodesManager::DeleteClonedNode(TSynchronizedNodesArrIter& synchronyzedNodeIt)
        {
            if (m_bActionsLogging)
            {
                AZ_TracePrintf("AsyncRender", "RenderNode: %p Deleted", static_cast<const void*>(synchronyzedNodeIt->first));
            }

            IRenderNode* pClonedNode = synchronyzedNodeIt->second.pNode;

            if (pClonedNode)
            {
                AZ_Assert(pClonedNode->GetRenderNodeType(), "(AsyncRenderWorld) Crashes here if pClonedNode is invalid.");
                if (!m_bLevelLoading)
                {
                    m_ownedNodes.erase(pClonedNode);
                }

                if (m_bLevelLoading)
                {
                    if (m_bUnregisterNodesOnDelete || m_unregisterNodes.find(pClonedNode) != m_unregisterNodes.end())
                    {
                        m_registratorHelper.UnregisterInEngine(pClonedNode);
                    }
                    HandleNodeRemove(pClonedNode);
                }
                else
                {
                    m_removedNodes.Insert(pClonedNode);
                    if (synchronyzedNodeIt->second.bRegistered)
                    {
                        m_unregisterNodes.insert(pClonedNode);
                    }
                }
            }
        }

        bool CRenderNodesManager::ChangeRenderNodeRegisteredState(IRenderNode* pRenderNode, bool bRegister)
        {
            const bool bSkipOctreeModification = true;
            const bool bDoOctreeModification = false;

            if (m_bHandlingNodeRemove)
            {
                return bDoOctreeModification;
            }

            if (!pRenderNode || (!pRenderNode->SupportsAsyncRender() && !m_bRegisterUnsupportedNodes))
            {
                return bSkipOctreeModification;
            }

            bool bForwardToEngine = bRegister ? m_registratorHelper.ShouldRegister() : m_registratorHelper.ShouldUnregister();

            if (bForwardToEngine)
            {
                return bDoOctreeModification;
            }

            if (m_removedNodes.Contains(pRenderNode))
            {
                return bSkipOctreeModification; // skip octree modification if node removed in current frame
            }

            // Check if rendernode is actually copy of other rendernode
            auto ownedNodesIt = m_ownedNodes.find(pRenderNode);
            if (ownedNodesIt != m_ownedNodes.end())
            {
                pRenderNode = ownedNodesIt->second;
            }

            if (bRegister)
            {
                RegisterForRender(pRenderNode);
            }
            else
            {
                UnregisterForRender(pRenderNode);
            }

            return bSkipOctreeModification;
        }

        void CRenderNodesManager::RegisterForRender(IRenderNode* pTickNode)
        {
            AZ_Assert(!!pTickNode && pTickNode->GetName(), "(AsyncRenderWorld) pTickNode is null");
            auto findResult = FindSynchronizedRenderNode(pTickNode);
            auto it = findResult.second;
            if (findResult.first)
            {
                AZ_Assert(m_bDisableAsserts || it->second.flag == ENodeAction::Register || it->second.flag == ENodeAction::Update ||
                          it->second.flag == ENodeAction::Unregister || it->second.flag == ENodeAction::OutOfAction,
                          "(AsyncRenderWorld) HandleNodeRegister: %p, Op: %d", static_cast<const void*>(it->first), int(it->second.flag));
                it->second.flag = ENodeAction::Register;
            }
            else
            {
                TSynchPair newNode { pTickNode, SFillNode { nullptr, ENodeAction::Register, false }};
                it = m_synchronizationArray.insert(it, newNode);
            }

            if (m_bLevelLoading)
            {
                HandleNode(*it);
            }
        }

        void CRenderNodesManager::UnregisterForRender(IRenderNode* pTickNode)
        {
            auto findResult = FindSynchronizedRenderNode(pTickNode);
            if (findResult.first)
            {
                auto& it = findResult.second;
                AZ_Assert(m_bDisableAsserts || it->second.flag == ENodeAction::Register || it->second.flag == ENodeAction::Update ||
                          it->second.flag == ENodeAction::Unregister || it->second.flag == ENodeAction::OutOfAction,
                          "(AsyncRenderWorld) HandleNodeUnregister: %p, Op: %d", static_cast<const void*>(it->first), int(it->second.flag));
                it->second.flag = ENodeAction::Unregister;

                if (m_bLevelLoading)
                {
                    HandleNode(*it);
                }
            }
        }

        void CRenderNodesManager::HandleNode(TSynchPair& pair)
        {
            if (m_bActionsLogging && pair.second.flag != ENodeAction::Update)
            {
                AZ_TracePrintf("AsyncRender", "RenderNode: %p, Op: %d", static_cast<const void*>(pair.first), int(pair.second.flag));
            }

            switch (pair.second.flag)
            {
            case ENodeAction::Register:
                HandleNodeRegister(pair);
                break;
            case ENodeAction::Unregister:
                HandleNodeUnregister(pair);
                break;
            default:
                break;
            }
        }

        void CRenderNodesManager::HandleNodeRegister(TSynchPair& pair)
        {
            auto& pClonedNode = pair.second.pNode;
            if (!pClonedNode)
            {
                if (m_bActionsLogging)
                {
                    AZ_TracePrintf("AsyncRender", "RenderNode: %p, Name: %s Cloned", static_cast<const void*>(pair.first), pair.first->GetName());
                }

                if (m_bRegisterUnsupportedNodes && !pair.first->SupportsAsyncRender())
                {
                    RegisterUnsupportedNode(pair);
                }
                else
                {
                    pClonedNode = pair.first->Clone();
                    m_ownedNodes.insert(AZStd::make_pair(pClonedNode, pair.first));
                }
            }
            else
            {
                HandleNodeUpdate(pair);
            }
            m_registratorHelper.RegisterInEngine(pClonedNode);
            pair.second.flag = ENodeAction::Update;
            pair.second.bRegistered = true;
        }

        void CRenderNodesManager::RegisterUnsupportedNode(TSynchPair& pair)
        {
            auto& pClonedNode = pair.second.pNode;
            pClonedNode = aznew CMockAsyncRenderNode(pair.first);
            m_ownedNodes.insert(AZStd::make_pair(pClonedNode, pair.first));
        }

        void CRenderNodesManager::HandleNodeUnregister(TSynchPair& pair)
        {
            if (pair.second.bRegistered)
            {
                m_registratorHelper.UnregisterInEngine(pair.second.pNode);
            }

            pair.second.flag = ENodeAction::OutOfAction; // We could try to set it to delete in next frame
            pair.second.bRegistered = false;
        }

        void CRenderNodesManager::HandleNodeRemove(IRenderNode* pNode)
        {
            if (!pNode)
            {
                return;
            }

            m_bHandlingNodeRemove = true;
            delete pNode;
            m_ownedNodes.erase(pNode);
            m_bHandlingNodeRemove = false;
        }

        void CRenderNodesManager::HandleNodeUpdate(TSynchPair& pair)
        {
            pair.second.pNode->CopyUpdatedData(*pair.first);
        }

        void CRenderNodesManager::CheckNodesRemovedAfterLevelUnload() const
        {
            AZ_Assert(m_bDisableAsserts || m_synchronizationArray.empty() && m_removedNodes.IsEmpty() && m_ownedNodes.empty(),
                      "(AsyncRenderWorld) RenderNodes not removed after level unload :  %d", m_synchronizationArray.size());
        }

        void CRenderNodesManager::SetLevelLoading(bool bLevelLoading)
        {
            m_bLevelLoading = bLevelLoading;
            ClearUnsupportedNodesRenderList();
        }

        void CRenderNodesManager::AddToRenderList(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo, IRenderNode* pEnt)
        {
            if (!m_bUseAddToRenderList)
            {
                auto ownedNodesIt = m_ownedNodes.find(pEnt);
                if (ownedNodesIt != m_ownedNodes.end())
                {
                    ownedNodesIt->second->Render(inRenderParams, passInfo);
                }
            }
            else
            {
                TRenderParamList::iterator itRenderParams = m_renderParamList.insert(inRenderParams).first;
                itRenderParams->pFoliage = nullptr;
                itRenderParams->pInstance = nullptr;
                itRenderParams->pMatrix = nullptr;
                itRenderParams->m_pVisArea = nullptr;
                itRenderParams->ppRNTmpData = nullptr;
                TPassInfoList::iterator itPassInfo = m_passInfoList.insert(passInfo).first;
                //itPassInfo->m_pCamera = nullptr;
                //itPassInfo->m_pRenderView = nullptr;
                m_renderList.push_back(AZStd::make_tuple(pEnt, *itRenderParams, *itPassInfo));
                AZ_Assert(AZStd::get<2>(*m_renderList.begin()).ThreadID() == passInfo.ThreadID(), "(AsyncRenderWorld) ThreadId doesn't match other values in list.");
            }
        }

        bool CRenderNodesManager::ShouldRenderUnsupportedNode(TFindSynchronizedNodeResult& findResult) const
        {
            const auto& it = findResult.second;
            const auto& flag = it->second.flag;
            return findResult.first && it->first
                    && (flag == ENodeAction::Update || flag == ENodeAction::Register);
        }

        NO_INLINE void CallRenderNodeRender(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo, IRenderNode* pEnt)
        {
            pEnt->Render(inRenderParams, passInfo);
        }

        void CRenderNodesManager::RenderUnsupportedNodes(const SRenderingPassInfo& passInfo)
        {
            if (m_bLevelLoading)
            {
                return;
            }

            AZ_PROFILE_FUNCTION(AZ::Debug::ProfileCategory::ThreeDEngine);
            CPerformanceWindow::GetInstance().StartUnsupportedNodesTimer();

            for (auto& renderData : m_renderList)
            {
                auto ownedNodesIt = m_ownedNodes.find(AZStd::get<0>(renderData));
                if (ownedNodesIt == m_ownedNodes.end())
                {
                    continue;
                }
                IRenderNode* pRenderNode = ownedNodesIt->second;
                auto findResult = FindSynchronizedRenderNode(pRenderNode);
                if (ShouldRenderUnsupportedNode(findResult))
                {
                    auto& itRenderParams = AZStd::get<1>(renderData);
                    auto& itPassInfo = AZStd::get<2>(renderData);
                    AZ_Assert(itPassInfo.ThreadID() == passInfo.ThreadID(), "(AsyncRenderWorld) ThreadId doesn't match.");
                    CallRenderNodeRender(itRenderParams, itPassInfo, pRenderNode);
                    auto& it = findResult.second;
                    static_cast<CMockAsyncRenderNode*>(it->second.pNode)->UpdateStreamableComponents(pRenderNode, itPassInfo);
                }
            }

            m_renderList.clear();
            m_bUseAddToRenderList = false;

            CPerformanceWindow::GetInstance().StopUnsupportedNodesTimer();
        }

        void CRenderNodesManager::StartAddingUnsupportedNodesToRenderList()
        {
            m_bUseAddToRenderList = true;
        }

        void CRenderNodesManager::ClearUnsupportedNodesRenderList()
        {
            m_renderList.clear();
            m_renderParamList.clear();
            m_passInfoList.clear();
        }

#ifdef IMGUI_ENABLED
        void CRenderNodesManager::AddImGuiMenu()
        {
            ImGui::Checkbox("Enable detailed operations logging", &m_bActionsLogging);
            ImGui::Checkbox("Asserts disabled", &m_bDisableAsserts);
            ImGui::Checkbox("Unregister deleted nodes", &m_bUnregisterNodesOnDelete);
            ImGui::Checkbox("Register unsupported nodes", &m_bRegisterUnsupportedNodes);
            ImGui::Checkbox("Parallel copy", &m_bParallelCopy);
        }
#endif  // IMGUI_ENABLED
    }
}
