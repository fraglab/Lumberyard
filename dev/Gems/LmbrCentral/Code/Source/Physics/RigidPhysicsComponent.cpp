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
#include "RigidPhysicsComponent.h"
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace LmbrCentral
{
    void RigidPhysicsConfig::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context);
        if (serializeContext)
        {
            serializeContext->Class<RigidPhysicsConfig>()
                ->Version(1)
                ->Field("EnabledInitially", &RigidPhysicsConfig::m_enabledInitially)
                ->Field("SpecifyMassOrDensity", &RigidPhysicsConfig::m_specifyMassOrDensity)
                ->Field("Mass", &RigidPhysicsConfig::m_mass)
                ->Field("Density", &RigidPhysicsConfig::m_density)
                ->Field("AtRestInitially", &RigidPhysicsConfig::m_atRestInitially)
                ->Field("EnableCollisionResponse", &RigidPhysicsConfig::m_enableCollisionResponse)
                ->Field("InteractsWithTriggers", &RigidPhysicsConfig::m_interactsWithTriggers)
                ->Field("RecordCollisions", &RigidPhysicsConfig::m_recordCollisions)
                ->Field("MaxRecordedCollisions", &RigidPhysicsConfig::m_maxRecordedCollisions)
                ->Field("SimulationDamping", &RigidPhysicsConfig::m_simulationDamping)
                ->Field("SimulationMinEnergy", &RigidPhysicsConfig::m_simulationMinEnergy)
                ->Field("BuoyancyDamping", &RigidPhysicsConfig::m_buoyancyDamping)
                ->Field("BuoyancyDensity", &RigidPhysicsConfig::m_buoyancyDensity)
                ->Field("BuoyancyResistance", &RigidPhysicsConfig::m_buoyancyResistance)
            ;
        }
        
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<RigidPhysicsConfig>()
                ->Enum<(int)RigidPhysicsConfig::MassOrDensity::Mass>("MassOrDensity_Mass")
                ->Enum<(int)RigidPhysicsConfig::MassOrDensity::Density>("MassOrDensity_Density")
                ->Property("EnabledInitially", BehaviorValueProperty(&RigidPhysicsConfig::m_enabledInitially))
                ->Property("SpecifyMassOrDensity",
                    [](RigidPhysicsConfig* config) { return (int&)(config->m_specifyMassOrDensity); },
                    [](RigidPhysicsConfig* config, const int& i) { config->m_specifyMassOrDensity = (RigidPhysicsConfig::MassOrDensity)i; })
                ->Property("Mass", BehaviorValueProperty(&RigidPhysicsConfig::m_mass))
                ->Property("Density", BehaviorValueProperty(&RigidPhysicsConfig::m_density))
                ->Property("AtRestInitially", BehaviorValueProperty(&RigidPhysicsConfig::m_atRestInitially))
                ->Property("EnableCollisionResponse", BehaviorValueProperty(&RigidPhysicsConfig::m_enableCollisionResponse))
                ->Property("InteractsWithTriggers", BehaviorValueProperty(&RigidPhysicsConfig::m_interactsWithTriggers))
                ->Property("BuoyancyDamping", BehaviorValueProperty(&RigidPhysicsConfig::m_buoyancyDamping))
                ->Property("BuoyancyDensity", BehaviorValueProperty(&RigidPhysicsConfig::m_buoyancyDensity))
                ->Property("BuoyancyResistance", BehaviorValueProperty(&RigidPhysicsConfig::m_buoyancyResistance))
                ->Property("SimulationDamping", BehaviorValueProperty(&RigidPhysicsConfig::m_simulationDamping))
                ->Property("SimulationMinEnergy", BehaviorValueProperty(&RigidPhysicsConfig::m_simulationMinEnergy))
                ->Property("RecordCollisions", BehaviorValueProperty(&RigidPhysicsConfig::m_recordCollisions))
                ->Property("MaxRecordedCollisions", BehaviorValueProperty(&RigidPhysicsConfig::m_maxRecordedCollisions))
                ;
        }
    }

    void RigidPhysicsComponent::Reflect(AZ::ReflectContext* context)
    {
        PhysicsComponent::Reflect(context);

        RigidPhysicsConfig::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RigidPhysicsComponent, PhysicsComponent>()
                ->Version(1)
                ->Field("Configuration", &RigidPhysicsComponent::m_configuration)
            ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Constant("RigidPhysicsComponentTypeId", BehaviorConstant(RigidPhysicsComponentTypeId));
        }
    }

    bool RigidPhysicsComponent::ReadInConfig(const AZ::ComponentConfig* baseConfig)
    {
        if (auto config = azrtti_cast<const RigidPhysicsConfig*>(baseConfig))
        {
            m_configuration = *config;
            return true;
        }
        return false;
    }

    bool RigidPhysicsComponent::WriteOutConfig(AZ::ComponentConfig* outBaseConfig) const
    {
        if (auto config = azrtti_cast<RigidPhysicsConfig*>(outBaseConfig))
        {
            *config = m_configuration;
            return true;
        }
        return false;
    }

    void RigidPhysicsComponent::Activate()
    {
#ifdef AZ_ENABLE_TRACING
        bool isStaticTransform = false;
        AZ::TransformBus::EventResult(isStaticTransform, GetEntityId(), &AZ::TransformBus::Events::IsStaticTransform);
        AZ_Warning("Rigid Physics Component", !isStaticTransform,
            "Rigid Body Physics needs to move, but entity '%s' %s has a static transform.", GetEntity()->GetName().c_str(), GetEntityId().ToString().c_str());
#endif

        PhysicsComponent::Activate();
    }

    void RigidPhysicsComponent::ConfigureCollisionGeometry()
    {
        // Assign weight to parts
        pe_status_nparts status_nparts;
        const int numParts = m_physicalEntity->GetStatus(&status_nparts);

        if (m_configuration.UseMass())
        {
            // Attempt to distribute mass amongst parts based on their volume.
            // If any part missing volume, just divide mass equally between parts.
            if (numParts > 0)
            {
                AZStd::vector<float> volumes(numParts, 0.f);
                float totalVolume = 0.f;
                bool allHaveVolume = true;
                for (int partI = 0; partI < numParts; ++partI)
                {
                    pe_params_part partParams;
                    partParams.ipart = partI;
                    m_physicalEntity->GetParams(&partParams);
                    if (!is_unused(partParams.pPhysGeom)
                        && partParams.pPhysGeom
                        && partParams.pPhysGeom->V > 0.f)
                    {
                        volumes[partI] = partParams.pPhysGeom->V;
                    }
                    else if (!is_unused(partParams.pPhysGeomProxy)
                             && partParams.pPhysGeomProxy
                             && partParams.pPhysGeomProxy->V > 0.f)
                    {
                        volumes[partI] = partParams.pPhysGeomProxy->V;
                    }

                    if (volumes[partI] > 0.f)
                    {
                        totalVolume += volumes[partI];
                    }
                    else
                    {
                        allHaveVolume = false;
                    }
                }

                for (int partI = 0; partI < numParts; ++partI)
                {
                    pe_params_part partParams;
                    partParams.ipart = partI;
                    if (allHaveVolume)
                    {
                        partParams.mass = (volumes[partI] / totalVolume) * m_configuration.m_mass;
                    }
                    else
                    {
                        partParams.mass = m_configuration.m_mass / numParts;
                    }
                    m_physicalEntity->SetParams(&partParams);
                }
            }
        }

        if (m_configuration.UseDensity())
        {
            for (int partI = 0; partI < numParts; ++partI)
            {
                pe_params_part partParams;
                partParams.ipart = partI;
                partParams.density = m_configuration.m_density;
                m_physicalEntity->SetParams(&partParams);
            }
        }

        if (!m_configuration.m_enableCollisionResponse)
        {
            pe_params_part partParams;
            partParams.flagsOR |= geom_no_coll_response;
            m_physicalEntity->SetParams(&partParams);
        }
    }

    void RigidPhysicsComponent::ConfigurePhysicalEntity()
    {
        // Setup simulation params.
        pe_simulation_params simParams;
        simParams.damping = m_configuration.m_simulationDamping;
        simParams.minEnergy = m_configuration.m_simulationMinEnergy;
        // Don't set maxLoggedCollisions if not recording; it relies on the MARK_UNUSED behavior
        if (m_configuration.m_recordCollisions)
        {
            simParams.maxLoggedCollisions = m_configuration.m_maxRecordedCollisions;
        }
        m_physicalEntity->SetParams(&simParams);

        // Setup buoyancy params.
        pe_params_buoyancy buoyancyParams;
        buoyancyParams.waterDamping = m_configuration.m_buoyancyDamping;
        buoyancyParams.kwaterDensity = m_configuration.m_buoyancyDensity;
        buoyancyParams.kwaterResistance = m_configuration.m_buoyancyResistance;
        buoyancyParams.waterEmin = m_configuration.m_simulationMinEnergy;
        m_physicalEntity->SetParams(&buoyancyParams);

        // If not at rest, wake entity
        pe_action_awake awakeParameters;
        awakeParameters.bAwake = m_configuration.m_atRestInitially ? 0 : 1;
        m_physicalEntity->Action(&awakeParameters);

        // Ensure that certain events bubble up from the physics system
        pe_params_flags flagParameters;
        flagParameters.flagsOR  = pef_log_poststep // enable event when entity is moved by physics system
            | pef_log_collisions;                     // enable collision event
        m_physicalEntity->SetParams(&flagParameters);
    }

} // namespace LmbrCentral
