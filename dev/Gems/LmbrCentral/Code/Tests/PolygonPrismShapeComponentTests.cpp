/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/

#include "StdAfx.h"
#include <AzTest/AzTest.h>

#include <AzCore/Component/ComponentApplication.h>
#include <AzCore/Math/Matrix3x3.h>
#include <AzCore/Math/VertexContainerInterface.h>
#include <AzFramework/Application/Application.h>
#include <AzFramework/Components/TransformComponent.h>
#include <Tests/TestTypes.h>

#include <Shape/PolygonPrismShapeComponent.h>

using namespace AZ;

namespace UnitTest
{
    class PolygonShapeComponentTests
        : public AllocatorsFixture
    {
        AZStd::unique_ptr<SerializeContext> m_serializeContext;
        AZStd::unique_ptr<ComponentDescriptor> m_transformComponentDescriptor;
        AZStd::unique_ptr<ComponentDescriptor> m_polygonPrismShapeComponentDescriptor;

    public:
        void SetUp() override
        {
            AllocatorsFixture::SetUp();
            m_serializeContext = AZStd::make_unique<SerializeContext>();

            m_transformComponentDescriptor = AZStd::unique_ptr<ComponentDescriptor>(AzFramework::TransformComponent::CreateDescriptor());
            m_transformComponentDescriptor->Reflect(&(*m_serializeContext));
            m_polygonPrismShapeComponentDescriptor = AZStd::unique_ptr<ComponentDescriptor>(LmbrCentral::PolygonPrismShapeComponent::CreateDescriptor());
            m_polygonPrismShapeComponentDescriptor->Reflect(&(*m_serializeContext));
        }

        void TearDown() override
        {
            m_serializeContext.reset();
            AllocatorsFixture::TearDown();
        }

        void PolygonShapeComponent_IsPointInside() const
        {
            Entity entity;
            LmbrCentral::PolygonPrismShapeComponent* polygonPrismShapeComponent = entity.CreateComponent<LmbrCentral::PolygonPrismShapeComponent>();
            entity.CreateComponent<AzFramework::TransformComponent>();

            entity.Init();
            entity.Activate();

            TransformBus::Event(entity.GetId(), &TransformBus::Events::SetWorldTM, Transform::CreateIdentity());

            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetHeight, 10.0f);
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 0.0f),
                        Vector2(0.0f, 10.0f),
                        Vector2(10.0f, 10.0f),
                        Vector2(10.0f, 0.0f)
                    }
                    ));

            // verify point inside returns true
            {
                bool inside;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(inside, entity.GetId(), &LmbrCentral::ShapeComponentRequests::IsPointInside, Vector3(5.0f, 5.0f, 5.0f));
                AZ_TEST_ASSERT(inside);
            }

            // verify point outside return false
            {
                bool inside;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(inside, entity.GetId(), &LmbrCentral::ShapeComponentRequests::IsPointInside, Vector3(5.0f, 5.0f, 20.0f));
                AZ_TEST_ASSERT(!inside);
            }

            // verify points at polygon edge return true
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(0.0f, 0.0f, 0.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(0.0f, 10.0f, 0.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(10.0f, 10.0f, 0.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(10.0f, 0.0f, 0.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(5.0f, 10.0f, 0.0f)));

            // verify point lies just inside
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(5.0f, 9.5f, 0.0f)));
            // verify point lies just outside
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(5.0f, 10.1f, 0.0f)));

            // note: the shape and positions/transforms were defined in the editor and replicated here - this
            // gave a good way to create various test cases and replicate them here
            TransformBus::Event(entity.GetId(), &TransformBus::Events::SetWorldTM, Transform::CreateFromMatrix3x3AndTranslation(Matrix3x3::CreateIdentity(), Vector3(497.0f, 595.0f, 32.0f)));
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 9.0f),
                        Vector2(6.5f, 6.5f),
                        Vector2(9.0f, 0.0f),
                        Vector2(6.5f, -6.5f),
                        Vector2(0.0f, -9.0f),
                        Vector2(-6.5f, -6.5f),
                        Vector2(-9.0f, 0.0f),
                        Vector2(-6.5f, 6.5f)
                    }
                    ));

            // verify point inside aabb but not inside polygon returns false
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(488.62f, 588.88f, 32.0f)));

            // verify point inside aabb and inside polygon returns true - when intersecting two vertices
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(496.62f, 595.0f, 32.0f)));

            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 0.0f),
                        Vector2(10.0f, 0.0f),
                        Vector2(5.0f, 10.0f)
                    }
                    ));

            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(496.62f, 595.0f, 32.0f)));

            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 10.0f),
                        Vector2(10.0f, 10.0f),
                        Vector2(5.0f, 0.0f)
                    }
                    ));

            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(496.62f, 595.0f, 32.0f)));

            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 0.0f),
                        Vector2(10.0f, 0.0f),
                        Vector2(5.0f, -10.0f)
                    }
                    ));

            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(496.62f, 595.0f, 32.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(499.62f, 595.0f, 32.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(502.0f, 585.0f, 32.0f)));

            // U shape
            TransformBus::Event(entity.GetId(), &TransformBus::Events::SetWorldTM, Transform::CreateIdentity());
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 0.0f),
                        Vector2(0.0f, 10.0f),
                        Vector2(5.0f, 10.0f),
                        Vector2(5.0f, 5.0f),
                        Vector2(10.0f, 5.0f),
                        Vector2(10.0f, 10.0f),
                        Vector2(15.0f, 15.0f),
                        Vector2(15.0f, 0.0f),
                    }
                    ));

            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(7.5f, 7.5f, 0.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(12.5f, 7.5f, 0.0f)));
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(12.5f, 7.5f, 12.0f)));

            // check polygon prism with rotation
            TransformBus::Event(entity.GetId(), &TransformBus::Events::SetWorldTM, Transform::CreateRotationX(DegToRad(45.0f)));
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetHeight, 10.0f);
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                {
                    Vector2(-5.0f, -5.0f),
                    Vector2(-5.0f, 5.0f),
                    Vector2(5.0f, 5.0f),
                    Vector2(5.0f, -5.0f)
                }
            ));

            // check below
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, 3.5f, 2.0f)));
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, -8.0f, -2.0f)));
            // check above
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, -8.0f, 8.0f)));
            AZ_TEST_ASSERT(!polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, 2.0f, 8.0f)));
            // check inside
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, -3.0f, 8.0f)));
            AZ_TEST_ASSERT(polygonPrismShapeComponent->IsPointInside(Vector3(2.0f, -3.0f, -2.0f)));
        }

        void PolygonShapeComponent_DistanceFromPoint() const
        {
            Entity entity;

            entity.CreateComponent<LmbrCentral::PolygonPrismShapeComponent>();
            entity.CreateComponent<AzFramework::TransformComponent>();

            entity.Init();
            entity.Activate();

            TransformBus::Event(entity.GetId(), &TransformBus::Events::SetWorldTM, Transform::CreateIdentity());

            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetHeight, 10.0f);
            LmbrCentral::PolygonPrismShapeComponentRequestsBus::Event(entity.GetId(), &LmbrCentral::PolygonPrismShapeComponentRequests::SetVertices,
                AZStd::vector<Vector2>(
                    {
                        Vector2(0.0f, 0.0f),
                        Vector2(0.0f, 10.0f),
                        Vector2(10.0f, 10.0f),
                        Vector2(10.0f, 0.0f)
                    }
                    ));

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(15.0f, 5.0f, 0.0f));
                AZ_TEST_ASSERT(distance == 5.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(5.0f, 5.0f, 5.0f));
                AZ_TEST_ASSERT(distance == 0.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(5.0f, 5.0f, 0.0f));
                AZ_TEST_ASSERT(distance == 0.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(1.0f, 1.0f, -1.0f));
                AZ_TEST_ASSERT(distance == 1.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(10.0f, 10.0f, 10.0f));
                AZ_TEST_ASSERT(distance == 0.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(5.0f, 5.0f, 15.0f));
                AZ_TEST_ASSERT(distance == 5.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(5.0f, 5.0f, -10.0f));
                AZ_TEST_ASSERT(distance == 10.0f);
            }

            {
                float distance;
                LmbrCentral::ShapeComponentRequestsBus::EventResult(distance, entity.GetId(), &LmbrCentral::ShapeComponentRequests::DistanceFromPoint, Vector3(5.0f, 13.0f, 14.0f));
                AZ_TEST_ASSERT(distance == 5.0f);
            }
        }
    };

    TEST_F(PolygonShapeComponentTests, PolygonShapeComponent_IsPointInside)
    {
        PolygonShapeComponent_IsPointInside();
    }

    TEST_F(PolygonShapeComponentTests, PolygonShapeComponent_DistanceFromPoint)
    {
        PolygonShapeComponent_DistanceFromPoint();
    }
}