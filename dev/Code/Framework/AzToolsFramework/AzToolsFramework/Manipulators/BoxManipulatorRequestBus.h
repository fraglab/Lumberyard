﻿/*
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

#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace AZ
{
    class Vector3;
}

namespace AzToolsFramework
{
    /// Interface for handling box manipulator requests.
    /// Used by \ref BoxComponentMode.
    class BoxManipulatorRequests
        : public AZ::EntityComponentBus
    {
    public:
        /// Get the X/Y/Z dimensions of the box shape/collider.
        virtual AZ::Vector3 GetDimensions() = 0;
        /// Set the X/Y/Z dimensions of the box shape/collider.
        virtual void SetDimensions(const AZ::Vector3& dimensions) = 0;
        /// Get the transform of the box shape/collider.
        /// This is used by \ref BoxComponentMode instead of the \ref \AZ::TransformBus
        /// because a collider may have an additional translation/orientation offset from
        /// the Entity transform.
        virtual AZ::Transform GetCurrentTransform() = 0;
        /// Get the scale currently applied to the box.
        /// With the Box Shape, the largest x/y/z component is taken
        /// so scale is always uniform, with colliders the scale may
        /// be different per component.
        virtual AZ::Vector3 GetBoxScale() = 0;

    protected:
        ~BoxManipulatorRequests() = default;
    };

    /// Type to inherit to implement BoxManipulatorRequests
    using BoxManipulatorRequestBus = AZ::EBus<BoxManipulatorRequests>;
} // namespace AzToolsFramework