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

#include <NvCloth_precompiled.h>

#include <Integration/ActorComponentBus.h>

#include <System/ActorClothColliders.h>

namespace NvCloth
{
    namespace
    {
        SphereCollider CreateSphereCollider(
            const Physics::ColliderConfiguration* colliderConfig,
            const Physics::SphereShapeConfiguration* sphereShapeConfig,
            int jointIndex, int sphereIndex)
        {
            SphereCollider sphereCollider;

            sphereCollider.m_jointIndex = jointIndex;
            sphereCollider.m_offsetTransform =
                AZ::Transform::CreateFromQuaternionAndTranslation(
                    colliderConfig->m_rotation,
                    colliderConfig->m_position);
            sphereCollider.m_radius = sphereShapeConfig->m_radius;
            sphereCollider.m_nvSphereIndex = sphereIndex;

            return sphereCollider;
        }

        CapsuleCollider CreateCapsuleCollider(
            const Physics::ColliderConfiguration* colliderConfig,
            const Physics::CapsuleShapeConfiguration* capsuleShapeConfig,
            int jointIndex, int capsuleIndex, int sphereAIndex, int sphereBIndex)
        {
            CapsuleCollider capsuleCollider;

            capsuleCollider.m_jointIndex = jointIndex;
            capsuleCollider.m_offsetTransform =
                AZ::Transform::CreateFromQuaternionAndTranslation(
                    colliderConfig->m_rotation,
                    colliderConfig->m_position);
            capsuleCollider.m_radius = capsuleShapeConfig->m_radius;
            capsuleCollider.m_height = capsuleShapeConfig->m_height;
            capsuleCollider.m_nvCapsuleIndex = capsuleIndex;
            capsuleCollider.m_nvSphereAIndex = sphereAIndex;
            capsuleCollider.m_nvSphereBIndex = sphereBIndex;

            return capsuleCollider;
        }
    }

    AZStd::unique_ptr<ActorClothColliders> ActorClothColliders::Create(AZ::EntityId entityId)
    {
        Physics::AnimationConfiguration* actorPhysicsConfig = nullptr;
        EMotionFX::Integration::ActorComponentRequestBus::EventResult(
            actorPhysicsConfig, entityId, &EMotionFX::Integration::ActorComponentRequestBus::Events::GetPhysicsConfig);
        if (!actorPhysicsConfig)
        {
            return nullptr;
        }

        const Physics::CharacterColliderConfiguration& clothConfig = actorPhysicsConfig->m_clothConfig;

        // Maximum number of spheres and capsules is imposed by NvCloth library
        const int maxSphereCount = 32;
        const int maxCapsuleCount = 32;
        int sphereCount = 0;
        int capsuleCount = 0;
        bool maxSphereCountReachedWarned = false;
        bool maxCapsuleCountReachedWarned = false;

        AZStd::vector<SphereCollider> sphereColliders;
        AZStd::vector<CapsuleCollider> capsuleColliders;

        for (const Physics::CharacterColliderNodeConfiguration& clothNodeConfig : clothConfig.m_nodes)
        {
            size_t jointIndex = EMotionFX::Integration::ActorComponentRequests::s_invalidJointIndex;
            EMotionFX::Integration::ActorComponentRequestBus::EventResult(
                jointIndex, entityId, 
                &EMotionFX::Integration::ActorComponentRequestBus::Events::GetJointIndexByName, 
                clothNodeConfig.m_name.c_str());
            if (jointIndex == EMotionFX::Integration::ActorComponentRequests::s_invalidJointIndex)
            {
                AZ_Warning("ActorAssetHelper", false, "Joint '%s' not found", clothNodeConfig.m_name.c_str());
                continue;
            }

            for (const Physics::ShapeConfigurationPair& shapeConfigPair : clothNodeConfig.m_shapes)
            {
                const auto& colliderConfig = shapeConfigPair.first;

                switch (shapeConfigPair.second->GetShapeType())
                {
                case Physics::ShapeType::Sphere:
                {
                    if (sphereCount >= maxSphereCount)
                    {
                        AZ_Warning("ActorAssetHelper", maxSphereCountReachedWarned,
                            "Maximum number of cloth sphere colliders (%d) reached",
                            maxSphereCount);
                        maxSphereCountReachedWarned = true;
                        continue;
                    }

                    SphereCollider sphereCollider = CreateSphereCollider(
                        colliderConfig.get(),
                        static_cast<const Physics::SphereShapeConfiguration*>(shapeConfigPair.second.get()),
                        static_cast<int>(jointIndex),
                        sphereCount);

                    sphereColliders.push_back(sphereCollider);
                    ++sphereCount;
                }
                break;

                case Physics::ShapeType::Capsule:
                {
                    if (capsuleCount >= maxCapsuleCount)
                    {
                        AZ_Warning("ActorAssetHelper", maxCapsuleCountReachedWarned,
                            "Maximum number of cloth capsule colliders (%d) reached",
                            maxSphereCount);
                        maxCapsuleCountReachedWarned = true;
                        continue;
                    }

                    // If there is only 1 sphere left to reach the maximum number
                    // of spheres the capsule won't fit as each capsule is formed of 2 spheres.
                    if (sphereCount >= maxSphereCount - 1)
                    {
                        AZ_Warning("ActorAssetHelper", maxCapsuleCountReachedWarned,
                            "Maximum number of cloth capsule colliders reached");
                        maxCapsuleCountReachedWarned = true;
                        continue;
                    }

                    CapsuleCollider capsuleCollider = CreateCapsuleCollider(
                        colliderConfig.get(),
                        static_cast<const Physics::CapsuleShapeConfiguration*>(shapeConfigPair.second.get()),
                        static_cast<int>(jointIndex),
                        capsuleCount * 2, // Each capsule holds 2 sphere indices
                        sphereCount + 0,  // First sphere index
                        sphereCount + 1); // Second sphere index

                    capsuleColliders.push_back(capsuleCollider);
                    ++capsuleCount;
                    sphereCount += 2; // Adds 2 spheres per capsule
                }
                break;

                default:
                    AZ_Warning("ActorAssetHelper", false, "Joint '%s' has an unexpected shape type (%d) for cloth collider.",
                        clothNodeConfig.m_name.c_str(), shapeConfigPair.second->GetShapeType());
                    break;
                }
            }
        }

        if (sphereCount == 0 && capsuleCount == 0)
        {
            return nullptr;
        }

        AZStd::unique_ptr<ActorClothColliders> actorClothColliders = AZStd::make_unique<ActorClothColliders>(entityId);

        actorClothColliders->m_sphereColliders = AZStd::move(sphereColliders);
        actorClothColliders->m_capsuleColliders = AZStd::move(capsuleColliders);
        actorClothColliders->m_nvSpheres.resize(sphereCount);
        actorClothColliders->m_nvCapsuleIndices.resize(capsuleCount * 2); // 2 sphere indices per capsule

        for (const auto& capsuleCollider : actorClothColliders->m_capsuleColliders)
        {
            actorClothColliders->m_nvCapsuleIndices[capsuleCollider.m_nvCapsuleIndex + 0] = capsuleCollider.m_nvSphereAIndex;
            actorClothColliders->m_nvCapsuleIndices[capsuleCollider.m_nvCapsuleIndex + 1] = capsuleCollider.m_nvSphereBIndex;
        }

        // Calculates the current transforms for the colliders
        // and fills the data as nvcloth needs them, ready to be
        // queried by the cloth component.
        actorClothColliders->Update();

        return actorClothColliders;
    }

    ActorClothColliders::ActorClothColliders(AZ::EntityId entityId)
        : m_entityId(entityId)
    {
    }

    void ActorClothColliders::Update()
    {
        for (auto& sphereCollider : m_sphereColliders)
        {
            AZ::Transform jointModelSpaceTransform = AZ::Transform::Identity();
            EMotionFX::Integration::ActorComponentRequestBus::EventResult(
                jointModelSpaceTransform, m_entityId,
                &EMotionFX::Integration::ActorComponentRequestBus::Events::GetJointTransform,
                static_cast<size_t>(sphereCollider.m_jointIndex), EMotionFX::Integration::Space::ModelSpace);

            sphereCollider.m_currentModelSpaceTransform = jointModelSpaceTransform * sphereCollider.m_offsetTransform;
            UpdateSphere(sphereCollider);
        }

        for (auto& capsuleCollider : m_capsuleColliders)
        {
            AZ::Transform jointModelSpaceTransform = AZ::Transform::Identity();
            EMotionFX::Integration::ActorComponentRequestBus::EventResult(
                jointModelSpaceTransform, m_entityId,
                &EMotionFX::Integration::ActorComponentRequestBus::Events::GetJointTransform,
                static_cast<size_t>(capsuleCollider.m_jointIndex), EMotionFX::Integration::Space::ModelSpace);

            capsuleCollider.m_currentModelSpaceTransform = jointModelSpaceTransform * capsuleCollider.m_offsetTransform;
            UpdateCapsule(capsuleCollider);
        }
    }

    void ActorClothColliders::UpdateSphere(const SphereCollider& sphere)
    {
        const AZ::Vector3 spherePosition = sphere.m_currentModelSpaceTransform.GetTranslation();
        AZ_Assert(sphere.m_nvSphereIndex != InvalidIndex, "Sphere collider has invalid index");
        m_nvSpheres[sphere.m_nvSphereIndex] = 
            physx::PxVec4(spherePosition.GetX(), spherePosition.GetY(), spherePosition.GetZ(), sphere.m_radius);
    }

    void ActorClothColliders::UpdateCapsule(const CapsuleCollider& capsule)
    {
        const float halfHeightExclusive = 0.5f * capsule.m_height - capsule.m_radius;
        const AZ::Vector3 basisZ = capsule.m_currentModelSpaceTransform.GetBasisZ() * halfHeightExclusive;
        const AZ::Vector3 capsulePosition = capsule.m_currentModelSpaceTransform.GetTranslation();

        const AZ::Vector3 sphereAPosition = capsulePosition + basisZ;
        const AZ::Vector3 sphereBPosition = capsulePosition - basisZ;
        AZ_Assert(capsule.m_nvSphereAIndex != InvalidIndex, "Capsule collider has an invalid index for its first sphere");
        AZ_Assert(capsule.m_nvSphereBIndex != InvalidIndex, "Capsule collider has an invalid index for its second sphere");
        m_nvSpheres[capsule.m_nvSphereAIndex] = 
            physx::PxVec4(sphereAPosition.GetX(), sphereAPosition.GetY(), sphereAPosition.GetZ(), capsule.m_radius);
        m_nvSpheres[capsule.m_nvSphereBIndex] = 
            physx::PxVec4(sphereBPosition.GetX(), sphereBPosition.GetY(), sphereBPosition.GetZ(), capsule.m_radius);
    }

    const AZStd::vector<SphereCollider>& ActorClothColliders::GetSphereColliders() const
    {
        return m_sphereColliders;
    }

    const AZStd::vector<CapsuleCollider>& ActorClothColliders::GetCapsuleColliders() const
    {
        return m_capsuleColliders;
    }

    const AZStd::vector<physx::PxVec4>& ActorClothColliders::GetSpheres() const
    {
        return m_nvSpheres;
    }

    const AZStd::vector<uint32_t>& ActorClothColliders::GetCapsuleIndices() const
    {
        return m_nvCapsuleIndices;
    }
} // namespace NvCloth
