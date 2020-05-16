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

#include "WeightedRandomSequencer.h"

#include <AzCore/std/string/string.h>

#include <Include/ScriptCanvas/Libraries/Logic/WeightedRandomSequencer.generated.cpp>
#include <Include/ScriptCanvas/Libraries/Math/MathNodeUtilities.h>

namespace ScriptCanvas
{
    namespace Nodes
    {

        namespace Logic
        {
            ////////////////////////////
            // WeightedRandomSequencer
            ////////////////////////////

            void WeightedRandomSequencer::ReflectDataTypes(AZ::ReflectContext* reflectContext)
            {
                if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
                {
                    if (serializeContext)
                    {
                        serializeContext->Class<WeightedPairing>()
                            ->Version(1)
                            ->Field("WeightSlotId", &WeightedPairing::m_weightSlotId)
                            ->Field("ExecutionSlotId", &WeightedPairing::m_executionSlotId)
                        ;
                    }
                }
            }

            void WeightedRandomSequencer::OnInit()
            {
                for (const WeightedPairing& weightedPairing : m_weightedPairings)
                {
                    EndpointNotificationBus::MultiHandler::BusConnect({ GetEntityId(), weightedPairing.m_weightSlotId });
                    EndpointNotificationBus::MultiHandler::BusConnect({ GetEntityId(), weightedPairing.m_executionSlotId });
                }
                
                // We always want at least one weighted transition state
                if (m_weightedPairings.empty())
                {
                    AddWeightedPair();                    
                }
            }

            void WeightedRandomSequencer::ConfigureVisualExtensions()
            {
                {
                    VisualExtensionSlotConfiguration visualExtension(VisualExtensionSlotConfiguration::VisualExtensionType::ExtenderSlot);

                    visualExtension.m_name = "Add State";
                    visualExtension.m_tooltip = "Adds a new weighted state to the node.";

                    visualExtension.m_connectionType = ConnectionType::Input;
                    visualExtension.m_displayGroup = GetDisplayGroup();
                    visualExtension.m_identifier = GetWeightExtensionId();

                    RegisterExtension(visualExtension);
                }

                {
                    VisualExtensionSlotConfiguration visualExtension(VisualExtensionSlotConfiguration::VisualExtensionType::ExtenderSlot);

                    visualExtension.m_name = "Add State";
                    visualExtension.m_tooltip = "Adds a new weighted state to the node.";

                    visualExtension.m_connectionType = ConnectionType::Output;
                    visualExtension.m_displayGroup = GetDisplayGroup();
                    visualExtension.m_identifier = GetExecutionExtensionId();

                    RegisterExtension(visualExtension);
                }

            }
            
            void WeightedRandomSequencer::OnInputSignal(const SlotId& slotId)
            {
                const SlotId inSlotId = WeightedRandomSequencerProperty::GetInSlotId(this);
                
                if (slotId == inSlotId)
                {
                    int runningTotal = 0;
                    
                    AZStd::vector< WeightedStruct > weightedStructs;
                    weightedStructs.reserve(m_weightedPairings.size());
                    
                    for (const WeightedPairing& weightedPairing : m_weightedPairings)
                    {
                        const Datum* datum = FindDatum(weightedPairing.m_weightSlotId);
                        
                        if (datum)
                        {
                            WeightedStruct weightedStruct;
                            weightedStruct.m_executionSlotId = weightedPairing.m_executionSlotId;
                            
                            if (datum->GetType().IS_A(ScriptCanvas::Data::Type::Number()))
                            {
                                int weight = aznumeric_cast<int>((*datum->GetAs<Data::NumberType>()));
                                
                                runningTotal += weight;                                
                                weightedStruct.m_totalWeight = runningTotal;
                            }

                            weightedStructs.emplace_back(weightedStruct);
                        }                        
                    }
                    
                    // We have no weights. So just trigger the first execution output
                    // Weighted pairings is controlled to never be empty.
                    if (runningTotal == 0)
                    {          
                        if (!m_weightedPairings.empty())
                        {
                            SignalOutput(m_weightedPairings.front().m_executionSlotId);
                        }
                        
                        return;
                    }
                    
                    int weightedResult = MathNodeUtilities::GetRandomIntegral<int>(1, runningTotal);                    
                    
                    for (const WeightedStruct& weightedStruct : weightedStructs)
                    {
                        if (weightedResult <= weightedStruct.m_totalWeight)
                        {
                            SignalOutput(weightedStruct.m_executionSlotId);
                            break;
                        }
                    }
                }
            }

            SlotId WeightedRandomSequencer::HandleExtension(AZ::Crc32 extensionId)
            {
                auto weightedPairing = AddWeightedPair();

                if (extensionId == GetWeightExtensionId())
                {
                    return weightedPairing.m_weightSlotId;
                }
                else if (extensionId == GetExecutionExtensionId())
                {
                    return weightedPairing.m_executionSlotId;
                }

                return SlotId();
            }

            bool WeightedRandomSequencer::CanDeleteSlot(const SlotId& slotId) const
            {
                return m_weightedPairings.size() > 1;
            }

            void WeightedRandomSequencer::OnSlotRemoved(const SlotId& slotId)
            {
                RemoveWeightedPair(slotId);
                FixupStateNames();
            }

            void WeightedRandomSequencer::RemoveWeightedPair(SlotId slotId)
            {
                for (auto pairIter = m_weightedPairings.begin(); pairIter != m_weightedPairings.end(); ++pairIter)
                {
                    const WeightedPairing& weightedPairing = (*pairIter);
                    
                    if (pairIter->m_executionSlotId == slotId
                        || pairIter->m_weightSlotId == slotId)                        
                    {
                        SlotId executionSlot = pairIter->m_executionSlotId;
                        SlotId weightSlot = pairIter->m_weightSlotId;

                        m_weightedPairings.erase(pairIter);

                        if (slotId == executionSlot)
                        {
                            RemoveSlot(weightSlot);
                        }
                        else if (slotId == weightSlot)
                        {
                            RemoveSlot(executionSlot);
                        }                        
                        
                        break;
                    }
                }
                
                FixupStateNames();
            }
            
            bool WeightedRandomSequencer::AllWeightsFilled() const
            {
                bool isFilled = true;
                for (const WeightedPairing& weightedPairing : m_weightedPairings)
                {
                    if (!IsConnected(weightedPairing.m_weightSlotId))
                    {
                        isFilled = false;
                        break;
                    }
                }             

                return isFilled;
            }

            bool WeightedRandomSequencer::HasExcessEndpoints() const
            {
                bool hasExcess = false;
                bool hasEmpty = false;

                for (const WeightedPairing& weightedPairing : m_weightedPairings)
                {
                    if (!IsConnected(weightedPairing.m_weightSlotId) && !IsConnected(weightedPairing.m_executionSlotId))
                    {
                        if (hasEmpty)
                        {
                            hasExcess = true;
                            break;
                        }
                        else
                        {
                            hasEmpty = true;
                        }
                    }
                }

                return hasExcess;
            }
            
            WeightedRandomSequencer::WeightedPairing WeightedRandomSequencer::AddWeightedPair()
            {
                int counterWeight = static_cast<int>(m_weightedPairings.size()) + 1;
            
                WeightedPairing weightedPairing;                

                DataSlotConfiguration dataSlotConfiguration;
                
                dataSlotConfiguration.SetConnectionType(ConnectionType::Input);
                dataSlotConfiguration.m_name = GenerateDataName(counterWeight);
                dataSlotConfiguration.m_toolTip = "The weight associated with the execution state.";
                dataSlotConfiguration.m_addUniqueSlotByNameAndType = false;
                dataSlotConfiguration.SetType(Data::Type::Number());

                dataSlotConfiguration.m_displayGroup = GetDisplayGroup();
                
                weightedPairing.m_weightSlotId = AddSlot(dataSlotConfiguration);
                
                ExecutionSlotConfiguration  slotConfiguration;
                
                slotConfiguration.m_name = GenerateOutName(counterWeight);
                slotConfiguration.m_addUniqueSlotByNameAndType = false;
                slotConfiguration.SetConnectionType(ConnectionType::Output);

                slotConfiguration.m_displayGroup = GetDisplayGroup();
                
                weightedPairing.m_executionSlotId = AddSlot(slotConfiguration);                
                
                m_weightedPairings.push_back(weightedPairing);

                return weightedPairing;
            }
            
            void WeightedRandomSequencer::FixupStateNames()
            {
                int counter = 1;
                for (const WeightedPairing& weightedPairing : m_weightedPairings)
                {
                    AZStd::string dataName = GenerateDataName(counter);
                    AZStd::string executionName = GenerateOutName(counter);
                    
                    Slot* dataSlot = GetSlot(weightedPairing.m_weightSlotId);
                    
                    if (dataSlot)
                    {
                        dataSlot->Rename(dataName);
                    }
                    
                    Slot* executionSlot = GetSlot(weightedPairing.m_executionSlotId);
                    
                    if (executionSlot)
                    {
                        executionSlot->Rename(executionName);
                    }
                    
                    ++counter;
                }
            }
            
            AZStd::string WeightedRandomSequencer::GenerateDataName(int counter)
            {
                return AZStd::string::format("Weight %i", counter);
            }
            
            AZStd::string WeightedRandomSequencer::GenerateOutName(int counter)
            {
                return AZStd::string::format("Out %i", counter);
            }
        }
    }
}