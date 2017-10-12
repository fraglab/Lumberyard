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

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/smart_ptr/intrusive_ptr.h>
#include <ScriptCanvas/Grammar/Grammar.h>

namespace ScriptCanvas
{
    namespace AST
    {
        template<typename t_RefCounted>
        using SmartPtr = AZStd::intrusive_ptr<t_RefCounted>;
        template<typename t_RefCounted>
        using SmartPtrConst = AZStd::intrusive_ptr<const t_RefCounted>;
        using RefCount = unsigned int;
        
        class Node;
        
        // AST Node sub-classes begin
        class BinaryOperation;
        class Block;
        class Expression;
        class ExpressionList;
        class FunctionCall;
        class FunctionCallAsPrefixExpression;
        class FunctionCallAsStatement;
        class Name;
        class Numeral;
        class PrefixExpression;
        class ReturnStatement;
        class Statement;
        class UnaryOperation;
        class Variable;
        // AST Node sub-classes end

        class Visitor;
               
    }
    
} // namespace ScriptCanvas