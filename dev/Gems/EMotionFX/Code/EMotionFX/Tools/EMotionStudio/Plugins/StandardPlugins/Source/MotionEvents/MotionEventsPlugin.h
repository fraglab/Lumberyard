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
#include "../../../../EMStudioSDK/Source/DockWidgetPlugin.h"
#include <EMotionFX/CommandSystem/Source/MotionEventCommands.h>
#include <MysticQt/Source/DialogStack.h>
#include "MotionEventPresetsWidget.h"
#include "MotionEventWidget.h"

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QShortcut)


namespace EMStudio
{
    class MotionEventsPlugin
        : public EMStudio::DockWidgetPlugin
    {
        Q_OBJECT
        MCORE_MEMORYOBJECTCATEGORY(MotionEventsPlugin, MCore::MCORE_DEFAULT_ALIGNMENT, MEMCATEGORY_STANDARDPLUGINS);

    public:
        enum
        {
            CLASS_ID = 0x00000942
        };

        MotionEventsPlugin();
        ~MotionEventsPlugin();

        // overloaded
        const char* GetCompileDate() const override     { return MCORE_DATE; }
        const char* GetName() const override            { return "Motion Events"; }
        uint32 GetClassID() const override              { return MotionEventsPlugin::CLASS_ID; }
        const char* GetCreatorName() const override     { return "MysticGD"; }
        float GetVersion() const override               { return 1.0f;  }
        bool GetIsClosable() const override             { return true;  }
        bool GetIsFloatable() const override            { return true;  }
        bool GetIsVertical() const override             { return false; }

        // overloaded main init function
        bool Init() override;
        EMStudioPlugin* Clone() override;

        void OnBeforeRemovePlugin(uint32 classID) override;

        MotionEventPresetsWidget* GetPresetsWidget() const                      { return mMotionEventPresetsWidget; }

        void ValidatePluginLinks();

        void FireColorChangedSignal()                                           { emit OnColorChanged(); }

    signals:
        void OnColorChanged();

    public slots:
        void ReInit();
        void MotionSelectionChanged();
        void UpdateMotionEventWidget();
        void WindowReInit(bool visible);
        void OnEventPresetDropped(QPoint position);
        bool CheckIfIsPresetReadyToDrop();
        void OnAddEventTrack()                                                                              { CommandSystem::CommandAddEventTrack(); }
        void RemoveEventTrack(int eventTrackNr)                                                             { CommandSystem::CommandRemoveEventTrack(eventTrackNr); }
        void SetEventTrackName(const QString& text, int trackNr)                                            { CommandSystem::CommandRenameEventTrack(trackNr, FromQtString(text).AsChar()); }
        void SetEventTrackEnabled(bool enabled, int trackNr)                                                { CommandSystem::CommandEnableEventTrack(trackNr, enabled); }

    private:
        MCORE_DEFINECOMMANDCALLBACK(CommandAdjustMotionCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandSelectCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandUnselectCallback);
        MCORE_DEFINECOMMANDCALLBACK(CommandClearSelectionCallback);
        CommandAdjustMotionCallback*    mAdjustMotionCallback;
        CommandSelectCallback*          mSelectCallback;
        CommandUnselectCallback*        mUnselectCallback;
        CommandClearSelectionCallback*  mClearSelectionCallback;

        MysticQt::DialogStack*          mDialogStack;
        MotionEventPresetsWidget*       mMotionEventPresetsWidget;
        MotionEventWidget*              mMotionEventWidget;

        QTableWidget*                   mMotionTable;
        TimeViewPlugin*                 mTimeViewPlugin;
        TrackHeaderWidget*              mTrackHeaderWidget;
        TrackDataWidget*                mTrackDataWidget;
        MotionWindowPlugin*             mMotionWindowPlugin;
        MotionListWindow*               mMotionListWindow;
        EMotionFX::Motion*              mMotion;
    };
} // namespace EMStudio