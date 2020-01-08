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
// Original file Copyright Crytek GMBH or its affiliates, used under license.

// Description : Assert dialog box

#pragma once

#include <AzCore/base.h>

//-----------------------------------------------------------------------------------------------------
// Just undef this if you want to use the standard assert function
//-----------------------------------------------------------------------------------------------------

// if AZ_ENABLE_TRACING is enabled, then calls to AZ_Assert(...) will flow in.  This is the case
// even in Profile mode - thus if you want to manage what happens, USE_CRY_ASSERT also needs to be enabled in those cases.
// if USE_CRY_ASSERT is not enabled, but AZ_ENABLE_TRACING is enabled, then the default behavior for assets will occur instead
// which is to throw the DEBUG BREAK exception / signal, which tends to end with application shutdown.
#if defined(AZ_ENABLE_TRACE_ASSERTS)
#define USE_AZ_ASSERT
#endif

#if !defined (USE_AZ_ASSERT) && defined(AZ_ENABLE_TRACING)
#undef USE_CRY_ASSERT
#define USE_CRY_ASSERT
#endif

// you can undefine this.  It will cause the assert message box to appear anywhere that USE_CRY_ASSERT is enabled
// instead of it only appearing in debug. 
// if this is DEFINED then only in debug builds will you see the message box.  In other builds, CRY_ASSERTS become CryWarning instead of
// instead (showing no message box, only a warning).
#define CRY_ASSERT_DIALOG_ONLY_IN_DEBUG

#if defined(FORCE_STANDARD_ASSERT) || defined(USE_AZ_ASSERT)
#undef USE_CRY_ASSERT
#undef CRY_ASSERT_DIALOG_ONLY_IN_DEBUG
#endif

// Using AZ_Assert for all assert kinds (assert =, CRY_ASSERT, AZ_Assert). This is for Provo and Xenia
// see Trace::Assert for implementation
#if defined(USE_AZ_ASSERT)
#undef assert
#define assert(condition) AZ_Assert(condition, "")
#endif //defined(USE_AZ_ASSERT)

//-----------------------------------------------------------------------------------------------------
// Use like this:
// CRY_ASSERT(expression);
// CRY_ASSERT_MESSAGE(expression,"Useful message");
// CRY_ASSERT_TRACE(expression,("This should never happen because parameter n%d named %s is %f",iParameter,szParam,fValue));
//-----------------------------------------------------------------------------------------------------

#if defined(AZ_RESTRICTED_PLATFORM)
    #if defined(AZ_PLATFORM_XENIA)
        #include "Xenia/CryAssert_h_xenia.inl"
    #elif defined(AZ_PLATFORM_PROVO)
        #include "Provo/CryAssert_h_provo.inl"
    #elif defined(AZ_PLATFORM_SALEM)
        #include "Salem/CryAssert_h_salem.inl"
    #endif
#endif
#if defined(AZ_RESTRICTED_SECTION_IMPLEMENTED)
    #undef AZ_RESTRICTED_SECTION_IMPLEMENTED
#elif defined(WIN32) || defined(APPLE) || defined(LINUX)
    #define CRYASSERT_H_TRAIT_USE_CRY_ASSERT_MESSAGE 1
#endif

#if defined(USE_CRY_ASSERT) && CRYASSERT_H_TRAIT_USE_CRY_ASSERT_MESSAGE
void CryAssertTrace(const char*, ...);
bool CryAssert(const char*, const char*, unsigned int, bool*);
void CryDebugBreak();

    #define CRY_ASSERT(condition) CRY_ASSERT_MESSAGE(condition, NULL)

    #define CRY_ASSERT_MESSAGE(condition, message) CRY_ASSERT_TRACE(condition, (message))

    #define CRY_ASSERT_TRACE(condition, parenthese_message)                  \
    do                                                                       \
    {                                                                        \
        static bool s_bIgnoreAssert = false;                                 \
        if (!s_bIgnoreAssert && !(condition))                                \
        {                                                                    \
            CryAssertTrace parenthese_message;                               \
            if (CryAssert(#condition, __FILE__, __LINE__, &s_bIgnoreAssert)) \
            {                                                                \
                DEBUG_BREAK;                                                 \
            }                                                                \
        }                                                                    \
    } while (0)

    #undef assert
    #define assert CRY_ASSERT
#elif !defined(CRY_ASSERT)
#ifndef USE_AZ_ASSERT
    #include <assert.h>
#endif //USE_AZ_ASSERT
    #define CRY_ASSERT(condition) assert(condition)
    #define CRY_ASSERT_MESSAGE(condition, message) assert(condition)
    #define CRY_ASSERT_TRACE(condition, parenthese_message) assert(condition)
#endif

//-----------------------------------------------------------------------------------------------------
