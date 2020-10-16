// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <AzCore/std/containers/set.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/Debug/Timer.h>
#include <AzCore/std/parallel/semaphore.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzGameFramework/FragLab/AsyncRender/AsyncRenderWorldRequestBus.h>
#include "RenderNodesRegistratorHelper.h"
#include "RemovedNodesManager.h"
#include "RenderNodesManagerBus.h"

struct IRenderNode;

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        struct SComparatorList
        {
            bool operator()(const SRenderingPassInfo& lhs, const SRenderingPassInfo& rhs)
            {
                return 0 < memcmp(&lhs, &rhs, sizeof(SRenderingPassInfo));
            }

            bool operator()(const SRendParams& lhs, const SRendParams& rhs)
            {
                return 0 < memcmp(&lhs, &rhs, sizeof(SRendParams));
            }
        };

        class CRenderNodesManager
            : public UnsupportedRendererNodesManagerBus::Handler
        {
        public:
            enum class ENodeAction
            {
                Register = 1,
                Unregister = 2,
                Remove = 4,
                Update = 8,
                OutOfAction = 16,
            };

            struct SFillNode
            {
                IRenderNode* pNode;
                ENodeAction flag : 8;
                bool bRegistered : 1;
            };

            using TSynchPair = AZStd::pair<IRenderNode*, SFillNode>;

            CRenderNodesManager(bool bLevelLoading);
            ~CRenderNodesManager();
            void RegisterForRender(IRenderNode* pTickNode);
            void UnregisterForRender(IRenderNode* pTickNode);
            void DeleteRenderNode(const IRenderNode* pTickNode);
            bool ChangeRenderNodeRegisteredState(IRenderNode* pRenderNode, bool bRegister);
            void ProcessAsyncRenderNodes();
            void CleanRemovedRenderNodes();
            void SetLevelLoading(bool bLevelLoading);
            void CheckNodesRemovedAfterLevelUnload() const;
            void StartAddingUnsupportedNodesToRenderList();

            // UnsupportedRendererNodesManagerBus
            void AddToRenderList(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo, IRenderNode* pEnt) override;
            void RenderUnsupportedNodes(const SRenderingPassInfo& passInfo) override;
#ifdef IMGUI_ENABLED
            void AddImGuiMenu();
#endif      // IMGUI_ENABLED

        private:
            void ClearUnsupportedNodesRenderList();
            using TRenderParamList = AZStd::set<SRendParams, SComparatorList>;
            using TPassInfoList = AZStd::set<SRenderingPassInfo, SComparatorList>;
            using TRenderListData = AZStd::tuple<IRenderNode*, SRendParams, SRenderingPassInfo>;
            TRenderParamList m_renderParamList;
            TPassInfoList m_passInfoList;
            AZStd::vector<TRenderListData> m_renderList;

            AZ_INLINE void HandleNode(TSynchPair& pair);
            void HandleNodeRegister(TSynchPair& pair);
            void HandleNodeUnregister(TSynchPair& pair);
            void RegisterUnsupportedNode(TSynchPair& pair);
            void HandleNodeRemove(IRenderNode* pNode);
            static void HandleNodeUpdate(TSynchPair& pair);
            using TSynchronizedNodesArr = AZStd::vector<TSynchPair>;
            using TSynchronizedNodesArrIter = TSynchronizedNodesArr::iterator;
            using TFindSynchronizedNodeResult = AZStd::pair<bool, TSynchronizedNodesArrIter>;
            TFindSynchronizedNodeResult FindSynchronizedRenderNode(const IRenderNode* pTickNode);
            void DeleteClonedNode(TSynchronizedNodesArrIter& synchronyzedNodeIt);
            bool ShouldRenderUnsupportedNode(TFindSynchronizedNodeResult& findResult) const;

            RenderNodesRegistratorHelper m_registratorHelper;
            TSynchronizedNodesArr m_synchronizationArray;
            // List of all nodes created by AsyncRender
            using TMapOwnedNodesToOriginal = AZStd::map<IRenderNode*, IRenderNode*>;
            TMapOwnedNodesToOriginal m_ownedNodes;
            CRemovedNodesManager m_removedNodes;
            AZStd::set<IRenderNode*> m_unregisterNodes;
            bool m_bHandlingNodeRemove = false;
            bool m_bLevelLoading;
            bool m_bDisableAsserts = false;
            bool m_bActionsLogging = false;
            bool m_bParallelCopy = true;
            bool m_bUnregisterNodesOnDelete = true;
            bool m_bRegisterUnsupportedNodes = true;

            bool m_bUseAddToRenderList = true;
        };
    }
}
