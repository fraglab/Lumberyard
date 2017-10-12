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

#include "../StandardPluginsConfig.h"
#include <AzCore/std/containers/vector.h>
#include <MysticQt/Source/DialogStack.h>
#include "../../../../EMStudioSDK/Source/DockWidgetPlugin.h"
#include <EMotionFX/CommandSystem/Source/ImporterCommands.h>
#include <EMotionFX/Tools/EMotionStudio/EMStudioSDK/Source/Commands.h>
#include <EMotionFX/Source/MotionInstance.h>
#include <EMotionFX/Source/EventHandler.h>


QT_FORWARD_DECLARE_CLASS(QLabel)


namespace EMStudio
{
    // forward declaration
    class MotionListWindow;
    class MotionPropertiesWindow;
    class MotionExtractionWindow;
    class MotionRetargetingWindow;
    class SaveDirtyMotionFilesCallback;
    class OutlinerCategoryCallback;

    class MotionWindowPlugin
        : public EMStudio::DockWidgetPlugin
    {
        Q_OBJECT
        MCORE_MEMORYOBJECTCATEGORY(MotionWindowPlugin, MCore::MCORE_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS);

    public:
        enum
        {
            CLASS_ID = 0x00000005
        };

        MotionWindowPlugin();
        ~MotionWindowPlugin();

        // overloaded
        const char* GetCompileDate() const override     { return MCORE_DATE; }
        const char* GetName() const override            { return "Motions"; }
        uint32 GetClassID() const override              { return MotionWindowPlugin::CLASS_ID; }
        const char* GetCreatorName() const override     { return "MysticGD"; }
        float GetVersion() const override               { return 1.0f;  }
        bool GetIsClosable() const override             { return true;  }
        bool GetIsFloatable() const override            { return true;  }
        bool GetIsVertical() const override             { return false; }

        // overloaded main init function
        bool Init() override;
        EMStudioPlugin* Clone() override;

        void Render(RenderPlugin* renderPlugin, EMStudioPlugin::RenderInfo* renderInfo) override;

        void ReInit();

        struct MotionTableEntry
        {
            MCORE_MEMORYOBJECTCATEGORY(MotionTableEntry, MCore::MCORE_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS);

            MotionTableEntry(EMotionFX::Motion* motion);

            EMotionFX::Motion*          mMotion;
            uint32                      mMotionID;
        };

        MotionTableEntry* FindMotionEntryByID(uint32 motionID);
        MCORE_INLINE size_t GetNumMotionEntries()                                                         { return mMotionEntries.size(); }
        MCORE_INLINE MotionTableEntry* GetMotionEntry(size_t index)                                      { return mMotionEntries[index]; }
        bool AddMotion(uint32 motionID);
        bool RemoveMotionByIndex(size_t index);
        bool RemoveMotionById(uint32 motionID);

        static AZStd::vector<EMotionFX::MotionInstance*>& GetSelectedMotionInstances();

        MCORE_INLINE MotionRetargetingWindow* GetMotionRetargetingWindow()                          { return mMotionRetargetingWindow; }
        MCORE_INLINE MotionExtractionWindow* GetMotionExtractionWindow()                            { return mMotionExtractionWindow; }
        MCORE_INLINE MotionListWindow* GetMotionListWindow()                                        { return mMotionListWindow; }
        MCORE_INLINE const char* GetDefaultNodeSelectionLabelText()                                 { return "Click to select node"; }

        int OnSaveDirtyMotions();
        int SaveDirtyMotion(EMotionFX::Motion* motion, MCore::CommandGroup* commandGroup, bool askBeforeSaving, bool showCancelButton = true);

        void PlayMotions(const AZStd::vector<EMotionFX::Motion*>& motions);
        void PlayMotion(EMotionFX::Motion* motion);
        void StopSelectedMotions();

    public slots:
        void UpdateInterface();
        void UpdateMotions();
        void VisibilityChanged(bool visible);

    private:
        void ClearMotionEntries();
        #ifdef _DEBUG
        bool VerifyMotions();
        #endif

        // declare the callbacks
        MCORE_DEFINECOMMANDCALLBACK(CommandImportMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandLoadMotionSetCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandRemoveMotionPostCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandSaveMotionAssetInfoCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandAdjustDefaultPlayBackInfoCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandAdjustMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandWaveletCompressMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandKeyframeCompressMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandScaleMotionDataCallback);

        CommandImportMotionCallback*                    mImportMotionCallback;
        CommandRemoveMotionPostCallback*                mRemoveMotionPostCallback;
        CommandSaveMotionAssetInfoCallback*             mSaveMotionAssetInfoCallback;
        CommandAdjustDefaultPlayBackInfoCallback*       mAdjustDefaultPlayBackInfoCallback;
        CommandAdjustMotionCallback*                    mAdjustMotionCallback;
        CommandLoadMotionSetCallback*                   mLoadMotionSetCallback;
        CommandWaveletCompressMotionCallback*           mWaveletCompressMotionCallback;
        CommandKeyframeCompressMotionCallback*          mKeyframeCompressMotionCallback;
        CommandScaleMotionDataCallback*                 mScaleMotionDataCallback;

        AZStd::vector<MotionTableEntry*>                mMotionEntries;

        // motion bind pose rendering helper variables
        MCore::Array<MCore::Matrix>                     mLocalMatrices;
        MCore::Array<MCore::Matrix>                     mGlobalMatrices;

        MysticQt::DialogStack*                          mDialogStack;
        MotionListWindow*                               mMotionListWindow;
        MotionPropertiesWindow*                         mMotionPropertiesWindow;
        MotionExtractionWindow*                         mMotionExtractionWindow;
        MotionRetargetingWindow*                        mMotionRetargetingWindow;

        SaveDirtyMotionFilesCallback*                   mDirtyFilesCallback;
        OutlinerCategoryCallback*                       mOutlinerCategoryCallback;
        EMotionFX::EventHandler*                        mEventHandler;

        static AZStd::vector<EMotionFX::MotionInstance*> mInternalMotionInstanceSelection;
    };
} // namespace EMStudio