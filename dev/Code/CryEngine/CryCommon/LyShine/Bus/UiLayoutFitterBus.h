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
#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <LyShine/UiBase.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//! This component resizes its element to fit its content. It uses cell sizing information given to
//! it by other Layout components, Text component, or Image component (fixed type).
class UiLayoutFitterInterface
    : public AZ::ComponentBus
{
public: // types

    //! Fit type indicating enabled fits
    enum class FitType
    {
        None,
        HorizontalOnly,
        VerticalOnly,
        HorizontalAndVertical
    };

public: // member functions

    virtual ~UiLayoutFitterInterface() {}

    //! Get whether to resize the element horizontally
    virtual bool GetHorizontalFit() = 0;

    //! Set whether to resize the element horizontally
    virtual void SetHorizontalFit(bool horizontalFit) = 0;

    //! Get whether to resize the element vertically
    virtual bool GetVerticalFit() = 0;

    //! Set whether to resize the element vertically
    virtual void SetVerticalFit(bool verticalFit) = 0;

    //! Get the fit type
    virtual FitType GetFitType() = 0;

public: // static member data

    //! Only one component on a entity can implement the events
    static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
};

typedef AZ::EBus<UiLayoutFitterInterface> UiLayoutFitterBus;
