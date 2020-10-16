// Copyright 2020 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"

#include "MockAsyncRenderNode.h"

#include <AzGameFramework/FragLab/AsyncRender/AsyncRenderWorldRequestBus.h>
#include <IRenderAuxGeom.h>
#include <../Cry3DEngine/Cry3DEngineBase.h>
#include <../Cry3DEngine/CVars.h>
#include <../Cry3DEngine/ObjMan.h>
#include "RenderNodesManagerBus.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        NO_INLINE CMockAsyncRenderNode::CMockAsyncRenderNode(IRenderNode * pEnt)
        {
            CopyUpdatedData(*pEnt);
            const auto nodeType = pEnt->GetRenderNodeType();
            m_bStreamable = (nodeType == eERType_ParticleEmitter) || (nodeType == eERType_MergedMesh);
#if defined(ENABLE_PROFILING_CODE)
            m_pOriginalNode = pEnt;
            m_name = pEnt->GetName();
#endif
        }

        const char* CMockAsyncRenderNode::GetName() const
        {
#if defined(ENABLE_PROFILING_CODE)
            return m_name.c_str();
#else
            return "MockAsyncRenderNode";
#endif
        }

        void CMockAsyncRenderNode::FillBBox(AABB& aabb)
        {
            aabb = GetBBox();
        }

        void CMockAsyncRenderNode::Render(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo)
        {
            // add to render in the end of frame
            EBUS_EVENT(UnsupportedRendererNodesManagerBus, AddToRenderList, inRenderParams, passInfo, this);
        }

        void CMockAsyncRenderNode::CopyUpdatedData(const IRenderNode& renderNode)
        {
            m_bbox = renderNode.GetBBox();
            m_pos = renderNode.GetPos();
            m_maxViewDist = const_cast<IRenderNode*>(&renderNode)->GetMaxViewDist();
        }

        bool CMockAsyncRenderNode::UpdateStreamableData(float fEntDistanceReal, float fImportanceFactor, bool bFullUpdate)
        {
            if (m_bStreamable)
            {
                const bool bUpdate = true;
                if (!m_updateArguments)
                {
                    m_updateArguments = AZStd::make_unique<SStreamableUpdateArgs>(fEntDistanceReal, fImportanceFactor, bFullUpdate, bUpdate);
                }
                else
                {
                    *m_updateArguments = SStreamableUpdateArgs{fEntDistanceReal, fImportanceFactor, bFullUpdate, bUpdate};
                }
            }
            return m_bStreamable;
        }

        void CMockAsyncRenderNode::UpdateStreamableComponents(IRenderNode * pEnt,  const struct SRenderingPassInfo& passInfo)
        {
            if (m_bStreamable)
            {
                if (!!m_updateArguments && m_updateArguments->bUpdate)
                {
                    gEnv->p3DEngine->GetObjManager()->UpdateRenderNodeStreamingPriority(pEnt, m_updateArguments->fEntDistanceReal,
                        m_updateArguments->fImportanceFactor, m_updateArguments->bFullUpdate, passInfo);
                    m_updateArguments->bUpdate = false;
                }
            }
        }

    }
}
