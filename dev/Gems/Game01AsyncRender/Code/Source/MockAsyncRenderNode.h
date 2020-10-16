// Copyright 2020 FragLab Ltd. All rights reserved.

#pragma once

#include <IEntityRenderState.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class CMockAsyncRenderNode
            : public IRenderNode
        {
        public:
            AZ_CLASS_ALLOCATOR(CMockAsyncRenderNode, AZ::SystemAllocator, 0);
            CMockAsyncRenderNode(IRenderNode * pEnt);
            void FillBBox(AABB& aabb) override;
            Vec3 GetPos(bool bWorldOnly = true) const override { return m_pos; }
            const AABB GetBBox() const override { return m_bbox; }
            float GetMaxViewDist() override { return m_maxViewDist; }
            EERType GetRenderNodeType() override { return eERType_Dummy_0; } // TODO: introduce new enum
            const char* GetEntityClassName() const override { return "MockAsyncRenderNode"; }
            const char* GetName() const override;
            void GetMemoryUsage(ICrySizer * pSizer) const override { return pSizer->AddObject(sizeof(*this)); }
            _smart_ptr<IMaterial> GetMaterialOverride() override { return nullptr; }
            _smart_ptr<IMaterial> GetMaterial(Vec3 *) override { return nullptr; }
            void SetMaterial(_smart_ptr<IMaterial>) override {}
            void SetPhysics(IPhysicalEntity *) override {}
            IPhysicalEntity * GetPhysics(void) const override { return nullptr; }
            void OffsetPosition(const Vec3 &) override {}
            void SetBBox(const AABB &) override {}

            void Render(const struct SRendParams& inRenderParams, const struct SRenderingPassInfo& passInfo) override;

            void CopyUpdatedData(const IRenderNode& renderNode) override;
            bool SupportsAsyncRender() const override { return true; }
            bool UpdateStreamableData(float fEntDistanceReal, float fImportanceFactor, bool bFullUpdate) override;

            void UpdateStreamableComponents(IRenderNode * pEnt, const struct SRenderingPassInfo& passInfo);
        private:
            float m_maxViewDist { 100.0f };
            AABB m_bbox;
            Vec3 m_pos { 0.0f, 0.0f, 0.0f };
            bool m_bStreamable { false };

            struct SStreamableUpdateArgs
            {
                SStreamableUpdateArgs(float fEntDistanceReal, float fImportanceFactor, bool bFull, bool bUpdate)
                    : fEntDistanceReal(fEntDistanceReal), fImportanceFactor(fImportanceFactor), bFullUpdate(bFull), bUpdate(bUpdate)
                    {}
                float fEntDistanceReal;
                float fImportanceFactor;
                bool bFullUpdate;
                bool bUpdate;
            };
            AZStd::unique_ptr<SStreamableUpdateArgs> m_updateArguments;
#if defined(ENABLE_PROFILING_CODE)
            IRenderNode * m_pOriginalNode = nullptr;
            AZStd::string m_name;
#endif
        };
    }
}
