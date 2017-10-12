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

// include the required headers
#include "EMotionFXConfig.h"
#include <MCore/Source/Compare.h>
#include <MCore/Source/UnicodeString.h>
#include <MCore/Source/AttributeSettings.h>
#include "AnimGraphVector2Condition.h"
#include "AnimGraph.h"
#include "AnimGraphInstance.h"
#include "AnimGraphAttributeTypes.h"
#include "EMotionFXManager.h"
#include "AnimGraphManager.h"


namespace EMotionFX
{
    // constructor
    AnimGraphVector2Condition::AnimGraphVector2Condition(AnimGraph* animGraph)
        : AnimGraphTransitionCondition(animGraph, TYPE_ID)
    {
        mParameterIndex     = MCORE_INVALIDINDEX32;

        mTestFunction       = TestGreater;
        mFunction           = FUNCTION_GREATER;

        mOperation          = OPERATION_LENGTH;
        mOperationFunction  = OperationLength;

        CreateAttributeValues();
        InitInternalAttributesForAllInstances();
    }


    // destructor
    AnimGraphVector2Condition::~AnimGraphVector2Condition()
    {
    }


    // create
    AnimGraphVector2Condition* AnimGraphVector2Condition::Create(AnimGraph* animGraph)
    {
        return new AnimGraphVector2Condition(animGraph);
    }


    // create unique data
    AnimGraphObjectData* AnimGraphVector2Condition::CreateObjectData()
    {
        return AnimGraphObjectData::Create(this, nullptr);
    }


    // register the attributes
    void AnimGraphVector2Condition::RegisterAttributes()
    {
        MCore::AttributeSettings* attribInfo;

        // register the source motion file name
        attribInfo = RegisterAttribute("Parameter", "parameter", "The parameter name to apply the condition on.", ATTRIBUTE_INTERFACETYPE_PARAMETERPICKER);
        attribInfo->SetDefaultValue(MCore::AttributeString::Create());

        // create the function combobox
        attribInfo = RegisterAttribute("Operation", "operation", "The type of operation to perform on the vector.", MCore::ATTRIBUTE_INTERFACETYPE_COMBOBOX);
        attribInfo->ResizeComboValues(OPERATION_NUMOPERATIONS);
        attribInfo->SetComboValue(OPERATION_LENGTH,     "Length");
        attribInfo->SetComboValue(OPERATION_GETX,       "Get X");
        attribInfo->SetComboValue(OPERATION_GETY,       "Get Y");
        attribInfo->SetDefaultValue(MCore::AttributeFloat::Create(static_cast<float>(mOperation)));

        // create the function combobox
        attribInfo = RegisterAttribute("Test Function", "testFunction", "The type of test function or condition.", MCore::ATTRIBUTE_INTERFACETYPE_COMBOBOX);
        attribInfo->ResizeComboValues(FUNCTION_NUMFUNCTIONS);
        attribInfo->SetComboValue(FUNCTION_GREATER,     "param > testValue");
        attribInfo->SetComboValue(FUNCTION_GREATEREQUAL, "param >= testValue");
        attribInfo->SetComboValue(FUNCTION_LESS,        "param < testValue");
        attribInfo->SetComboValue(FUNCTION_LESSEQUAL,   "param <= testValue");
        attribInfo->SetComboValue(FUNCTION_NOTEQUAL,    "param != testValue");
        attribInfo->SetComboValue(FUNCTION_EQUAL,       "param == testValue");
        attribInfo->SetComboValue(FUNCTION_INRANGE,     "param INRANGE [testValue..rangeValue]");
        attribInfo->SetComboValue(FUNCTION_NOTINRANGE,  "param NOT INRANGE [testValue..rangeValue]");
        attribInfo->SetDefaultValue(MCore::AttributeFloat::Create(static_cast<float>(mFunction)));

        // create the test value float spinner
        attribInfo = RegisterAttribute("Test Value", "testValue", "The float value to test against the parameter value.", MCore::ATTRIBUTE_INTERFACETYPE_FLOATSPINNER);
        attribInfo->SetDefaultValue(MCore::AttributeFloat::Create(0.0f));
        attribInfo->SetMinValue(MCore::AttributeFloat::Create(-FLT_MAX));
        attribInfo->SetMaxValue(MCore::AttributeFloat::Create(FLT_MAX));

        // create the low range value
        attribInfo = RegisterAttribute("Range Value", "rangeValue", "The range high or low bound value, only used when the function is set to 'In Range' or 'Not in Range'.", MCore::ATTRIBUTE_INTERFACETYPE_FLOATSPINNER);
        attribInfo->SetDefaultValue(MCore::AttributeFloat::Create(0.0f));
        attribInfo->SetMinValue(MCore::AttributeFloat::Create(-FLT_MAX));
        attribInfo->SetMaxValue(MCore::AttributeFloat::Create(FLT_MAX));
    }


    // get the palette name
    const char* AnimGraphVector2Condition::GetPaletteName() const
    {
        return "Vector2 Condition";
    }


    // get the type string
    const char* AnimGraphVector2Condition::GetTypeString() const
    {
        return "AnimGraphVector2Condition";
    }

    
    // get the type of the selected paramter
    uint32 AnimGraphVector2Condition::GetParameterType() const
    {
        // if there is no parameter selected yet, return invalid type
        if (mParameterIndex == MCORE_INVALIDINDEX32)
        {
            return MCORE_INVALIDINDEX32;
        }

        // get access to the parameter info and return the type of its default value
        MCore::AttributeSettings* parameterInfo = mAnimGraph->GetParameter(mParameterIndex);
        return parameterInfo->GetDefaultValue()->GetType();
    }


    // test the condition
    bool AnimGraphVector2Condition::TestCondition(AnimGraphInstance* animGraphInstance) const
    {
        // allow the transition in case we don't have a valid parameter to test against
        if (mParameterIndex == MCORE_INVALIDINDEX32)
        {
            return false; // act like this condition failed
        }
        // make sure we have the right type, otherwise fail
        const uint32 parameterType = GetParameterType();
        if (parameterType != MCore::AttributeVector2::TYPE_ID)
        {
            return false;
        }

        // get the vector value
        AZ::Vector2 vectorValue(0.0f, 0.0f);
        MCore::Attribute* attribute = animGraphInstance->GetParameterValue(mParameterIndex);
        MCORE_ASSERT(attribute->GetType() == MCore::AttributeVector2::TYPE_ID);
        vectorValue = static_cast<MCore::AttributeVector2*>(attribute)->GetValue();

        // perform the operation on the vector
        const float operationResult = mOperationFunction(vectorValue);

        // get the test and range values
        const float testValue   = GetAttributeFloat(ATTRIB_TESTVALUE)->GetValue();
        const float rangeValue  = GetAttributeFloat(ATTRIB_RANGEVALUE)->GetValue();

        // now apply the function
        return mTestFunction(operationResult, testValue, rangeValue);
    }


    // clonse the condition
    AnimGraphObject* AnimGraphVector2Condition::Clone(AnimGraph* animGraph)
    {
        // create the clone
        AnimGraphVector2Condition* clone = new AnimGraphVector2Condition(animGraph);

        // copy base class settings such as parameter values to the new clone
        CopyBaseObjectTo(clone);

        // return a pointer to the clone
        return clone;
    }


    // set the math function to use
    void AnimGraphVector2Condition::SetFunction(EFunction func)
    {
        // if it didn't change, don't update anything
        if (func == mFunction)
        {
            return;
        }

        mFunction = func;
        switch (mFunction)
        {
        case FUNCTION_GREATER:
            mTestFunction = TestGreater;
            return;
        case FUNCTION_GREATEREQUAL:
            mTestFunction = TestGreaterEqual;
            return;
        case FUNCTION_LESS:
            mTestFunction = TestLess;
            return;
        case FUNCTION_LESSEQUAL:
            mTestFunction = TestLessEqual;
            return;
        case FUNCTION_NOTEQUAL:
            mTestFunction = TestNotEqual;
            return;
        case FUNCTION_EQUAL:
            mTestFunction = TestEqual;
            return;
        case FUNCTION_INRANGE:
            mTestFunction = TestInRange;
            return;
        case FUNCTION_NOTINRANGE:
            mTestFunction = TestNotInRange;
            return;
        default:
            MCORE_ASSERT(false);        // function unknown
        }
        ;
    }


    // set the operation
    void AnimGraphVector2Condition::SetOperation(EOperation operation)
    {
        // if it didn't change, don't update anything
        if (operation == mOperation)
        {
            return;
        }

        mOperation = operation;
        switch (mOperation)
        {
        case OPERATION_LENGTH:
            mOperationFunction = OperationLength;
            return;
        case OPERATION_GETX:
            mOperationFunction = OperationGetX;
            return;
        case OPERATION_GETY:
            mOperationFunction = OperationGetY;
            return;
        default:
            MCORE_ASSERT(false);        // function unknown
        }
        ;
    }


    // update the data
    void AnimGraphVector2Condition::OnUpdateAttributes()
    {
        MCORE_ASSERT(mAnimGraph);

        // update the function pointers
        SetFunction((EFunction)((uint32)GetAttributeFloat(ATTRIB_FUNCTION)->GetValue()));
        SetOperation((EOperation)((uint32)GetAttributeFloat(ATTRIB_OPERATION)->GetValue()));

        // get the name of the parameter we want to compare against
        const char* name = GetAttributeString(ATTRIB_PARAMETER)->AsChar();

        // convert this name into an index value, to prevent string based lookups every frame
        mParameterIndex = MCORE_INVALIDINDEX32;
    #ifdef EMFX_EMSTUDIOBUILD
        MCore::AttributeSettings* parameterInfo = nullptr;
    #endif
        if (name)
        {
            const uint32 numParams = mAnimGraph->GetNumParameters();
            for (uint32 i = 0; i < numParams; ++i)
            {
                if (mAnimGraph->GetParameter(i)->GetNameString().CheckIfIsEqual(name))
                {
                #ifdef EMFX_EMSTUDIOBUILD
                    parameterInfo = mAnimGraph->GetParameter(i);
                #endif
                    mParameterIndex = i;
                    break;
                }
            }
        }

        // disable GUI items that have no influence
    #ifdef EMFX_EMSTUDIOBUILD
        // disable all attributes
        EnableAllAttributes(false);

        // always enable the parameter selection attribute
        SetAttributeEnabled(ATTRIB_PARAMETER);

        if (parameterInfo && parameterInfo->GetDefaultValue()->GetType() == MCore::AttributeVector2::TYPE_ID)
        {
            SetAttributeEnabled(ATTRIB_TESTVALUE);
            SetAttributeEnabled(ATTRIB_RANGEVALUE);
            SetAttributeEnabled(ATTRIB_FUNCTION);
            SetAttributeEnabled(ATTRIB_OPERATION);

            // disable the range value if we're not using a function that needs the range
            const int32 function = GetAttributeFloatAsInt32(ATTRIB_FUNCTION);
            if (function == FUNCTION_INRANGE || function == FUNCTION_NOTINRANGE)
            {
                SetAttributeEnabled(ATTRIB_RANGEVALUE);
            }
        }
    #endif
    }


    // get the name of the currently used test function
    const char* AnimGraphVector2Condition::GetTestFunctionString() const
    {
        // get access to the combo values and the currently selected function
        MCore::AttributeSettings*   comboAttributeInfo  = GetAnimGraphManager().GetAttributeInfo(this, ATTRIB_FUNCTION);
        const int32                 functionIndex       = GetAttributeFloatAsInt32(ATTRIB_FUNCTION);
        return comboAttributeInfo->GetComboValue(functionIndex);
    }


    // get the name of the currently used operation
    const char* AnimGraphVector2Condition::GetOperationString() const
    {
        // get access to the combo values and the currently selected function
        MCore::AttributeSettings*   comboAttributeInfo  = GetAnimGraphManager().GetAttributeInfo(this, ATTRIB_OPERATION);
        const int32                 functionIndex       = GetAttributeFloatAsInt32(ATTRIB_OPERATION);
        return comboAttributeInfo->GetComboValue(functionIndex);
    }


    // construct and output the information summary string for this object
    void AnimGraphVector2Condition::GetSummary(MCore::String* outResult) const
    {
        outResult->Format("%s: Parameter Name='%s', Test Function='%s', Test Value=%.2f", GetTypeString(), GetAttributeString(ATTRIB_PARAMETER)->AsChar(), GetTestFunctionString(), GetAttributeFloat(ATTRIB_TESTVALUE)->GetValue());
    }


    // construct and output the tooltip for this object
    void AnimGraphVector2Condition::GetTooltip(MCore::String* outResult) const
    {
        MCore::String columnName, columnValue;

        // add the condition type
        columnName = "Condition Type: ";
        columnValue = GetTypeString();
        outResult->Format("<table border=\"0\"><tr><td width=\"120\"><b>%s</b></td><td><nobr>%s</nobr></td>", columnName.AsChar(), columnValue.AsChar());

        // add the parameter
        columnName = "Parameter Name: ";
        //columnName.ConvertToNonBreakingHTMLSpaces();
        columnValue = GetAttributeString(ATTRIB_PARAMETER)->AsChar();
        //columnValue.ConvertToNonBreakingHTMLSpaces();
        outResult->FormatAdd("</tr><tr><td><b><nobr>%s</nobr></b></td><td><nobr>%s</nobr></td>", columnName.AsChar(), columnValue.AsChar());

        // add the operation
        columnName = "Operation: ";
        columnValue = GetOperationString();
        outResult->FormatAdd("</tr><tr><td><b><nobr>%s</nobr></b></td><td><nobr>%s</nobr></td></tr>", columnName.AsChar(), columnValue.AsChar());

        // add the test function
        columnName = "Test Function: ";
        //columnName.ConvertToNonBreakingHTMLSpaces();
        columnValue = GetTestFunctionString();
        //columnValue.ConvertToNonBreakingHTMLSpaces();
        outResult->FormatAdd("</tr><tr><td><b><nobr>%s</nobr></b></td><td><nobr>%s</nobr></td></tr>", columnName.AsChar(), columnValue.AsChar());

        // add the test value
        columnName = "Test Value: ";
        //columnName.ConvertToNonBreakingHTMLSpaces();
        columnValue.Format("%.3f", GetAttributeFloat(ATTRIB_TESTVALUE)->GetValue());
        //columnValue.ConvertToNonBreakingHTMLSpaces();
        outResult->FormatAdd("</tr><tr><td><b><nobr>%s</nobr></b></td><td><nobr>%s</nobr></td>", columnName.AsChar(), columnValue.AsChar());

        // add the range value
        columnName = "Range Value: ";
        //columnName.ConvertToNonBreakingHTMLSpaces();
        columnValue.Format("%.3f", GetAttributeFloat(ATTRIB_RANGEVALUE)->GetValue());
        //columnValue.ConvertToNonBreakingHTMLSpaces();
        outResult->FormatAdd("</tr><tr><td><b><nobr>%s</nobr></b></td><td><nobr>%s</nobr></td>", columnName.AsChar(), columnValue.AsChar());
    }


    //------------------------------------------------------------------------------------------
    // Test Functions
    //------------------------------------------------------------------------------------------
    bool AnimGraphVector2Condition::TestGreater(float paramValue, float testValue, float rangeValue)           { MCORE_UNUSED(rangeValue); return (paramValue > testValue);    }
    bool AnimGraphVector2Condition::TestGreaterEqual(float paramValue, float testValue, float rangeValue)      { MCORE_UNUSED(rangeValue); return (paramValue >= testValue);   }
    bool AnimGraphVector2Condition::TestLess(float paramValue, float testValue, float rangeValue)              { MCORE_UNUSED(rangeValue); return (paramValue < testValue);    }
    bool AnimGraphVector2Condition::TestLessEqual(float paramValue, float testValue, float rangeValue)         { MCORE_UNUSED(rangeValue); return (paramValue <= testValue); }
    bool AnimGraphVector2Condition::TestEqual(float paramValue, float testValue, float rangeValue)             { MCORE_UNUSED(rangeValue); return MCore::Compare<float>::CheckIfIsClose(paramValue, testValue, MCore::Math::epsilon); }
    bool AnimGraphVector2Condition::TestNotEqual(float paramValue, float testValue, float rangeValue)          { MCORE_UNUSED(rangeValue); return (MCore::Compare<float>::CheckIfIsClose(paramValue, testValue, MCore::Math::epsilon) == false); }
    bool AnimGraphVector2Condition::TestInRange(float paramValue, float testValue, float rangeValue)
    {
        if (testValue <= rangeValue)
        {
            return (MCore::InRange<float>(paramValue, testValue, rangeValue));
        }
        else
        {
            return (MCore::InRange<float>(paramValue, rangeValue, testValue));
        }
    }

    bool AnimGraphVector2Condition::TestNotInRange(float paramValue, float testValue, float rangeValue)
    {
        if (testValue <= rangeValue)
        {
            return (MCore::InRange<float>(paramValue, testValue, rangeValue) == false);
        }
        else
        {
            return (MCore::InRange<float>(paramValue, rangeValue, testValue) == false);
        }
    }


    //------------------------------------------------------------------------------------------
    // Operations
    //------------------------------------------------------------------------------------------
    float AnimGraphVector2Condition::OperationLength(const AZ::Vector2& vec)                               { return vec.GetLength();   }
    float AnimGraphVector2Condition::OperationGetX(const AZ::Vector2& vec)                                 { return vec.GetX();    }
    float AnimGraphVector2Condition::OperationGetY(const AZ::Vector2& vec)                                 { return vec.GetY();    }
}   // namespace EMotionFX
