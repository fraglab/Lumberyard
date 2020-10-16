// Copyright 2019 FragLab Ltd. All rights reserved.

#include "game01asyncrender_precompiled.h"


#include <SystemComponent.h>
#include <platform_impl.h>

#include <IGem.h>

#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include "CVars/AsyncRenderCVars.h"

namespace Fraglab
{
    namespace Game01AsyncRender
    {
        class Game01AsyncRenderModule
            : public CryHooksModule
        {
        public:
            AZ_RTTI(Game01AsyncRenderModule, "{17717034-D5AA-47D4-8695-A6803E9184A1}", CryHooksModule);
            AZ_CLASS_ALLOCATOR(Game01AsyncRenderModule, AZ::SystemAllocator, 0);

            Game01AsyncRenderModule()
                : CryHooksModule()
            {
                m_descriptors.insert(m_descriptors.end(), {
                    SystemComponent::CreateDescriptor(),
                    Game01AsyncRenderCVars::CreateDescriptor(),
                });
            }

            /**
             * Add required SystemComponents to the SystemEntity.
             */
            AZ::ComponentTypeList GetRequiredSystemComponents() const override
            {
                return AZ::ComponentTypeList {
                        azrtti_typeid<SystemComponent>(),
                        azrtti_typeid<Game01AsyncRenderCVars>()
                };
            }
        };
    }
}

// DO NOT MODIFY THIS LINE UNLESS YOU RENAME THE GEM
// The first parameter should be GemName_GemIdLower
// The second should be the fully qualified name of the class above
AZ_DECLARE_MODULE_CLASS(Game01AsyncRender_58a7cdecb233404d9900bc9939c396e1, Fraglab::Game01AsyncRender::Game01AsyncRenderModule)
