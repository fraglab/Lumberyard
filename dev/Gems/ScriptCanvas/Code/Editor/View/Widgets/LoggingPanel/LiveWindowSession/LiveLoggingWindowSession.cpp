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
#include "precompiled.h"

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <EditorCoreAPI.h>
#include <IEditor.h>

#include <Editor/View/Widgets/LoggingPanel/LiveWindowSession/LiveLoggingWindowSession.h>

namespace ScriptCanvasEditor
{
    ///////////////////////
    // TargetManagerModel
    ///////////////////////

    TargetManagerModel::TargetManagerModel()
    {
        AzFramework::TargetInfo editorTargetInfo(0, "Editor");
        m_targetInfo.push_back(editorTargetInfo);

        AzFramework::TargetManager::Bus::BroadcastResult(m_selfInfo, &AzFramework::TargetManager::GetMyTargetInfo);

        ScrapeTargetInfo();
    }

    int TargetManagerModel::rowCount(const QModelIndex& parent) const
    {
        return static_cast<int>(m_targetInfo.size());
    }

    QVariant TargetManagerModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
        {
            return QVariant();
        }

        switch (role)
        {
        case Qt::DisplayRole:
        {
            const AzFramework::TargetInfo& targetInfo = m_targetInfo[index.row()];
            if (index.row() > 0)
            {
                return QString("%1 (%2)").arg(targetInfo.GetDisplayName(), QString::number(targetInfo.GetPersistentId(), 16));
            }
            else
            {
                return QString(targetInfo.GetDisplayName());
            }
        }
        break;
        default:
            break;
        }

        return QVariant();
    }

    void TargetManagerModel::TargetJoinedNetwork(AzFramework::TargetInfo info)
    {
        if (!info.IsIdentityEqualTo(m_selfInfo))
        {
            int element = GetRowForTarget(info.GetPersistentId());

            if (element < 0)
            {
                beginInsertRows(QModelIndex(), rowCount(), rowCount());
                m_targetInfo.push_back(info);
                endInsertRows();
            }
        }
        else
        {
            ScrapeTargetInfo();
        }
    }

    void TargetManagerModel::TargetLeftNetwork(AzFramework::TargetInfo info)
    {
        int element = GetRowForTarget(info.GetPersistentId());

        // 0 is reserved for our fake Editor one.
        // And we don't want to remove it.
        if (element > 0)
        {
            beginRemoveRows(QModelIndex(), element, element);
            m_targetInfo.erase(m_targetInfo.begin() + element);
            endRemoveRows();
        }
    }

    AzFramework::TargetInfo TargetManagerModel::FindTargetInfoForRow(int row)
    {
        if (row < 0 && row >= m_targetInfo.size())
        {
            return AzFramework::TargetInfo();
        }

        return m_targetInfo[row];
    }

    int TargetManagerModel::GetRowForTarget(AZ::u32 targetId)
    {
        for (size_t i = 0; i < m_targetInfo.size(); ++i)
        {
            if (m_targetInfo[i].GetPersistentId() == targetId)
            {
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    void TargetManagerModel::ScrapeTargetInfo()
    {
        AzFramework::TargetContainer targets;
        AzFramework::TargetManager::Bus::Broadcast(&AzFramework::TargetManager::EnumTargetInfos, targets);

        for (const auto& targetPair : targets)
        {
            if (!targetPair.second.IsIdentityEqualTo(m_selfInfo))
            {
                m_targetInfo.push_back(targetPair.second);
            }
        }
    }

    ////////////////////////////
    // LiveLoggingUserSettings
    ////////////////////////////

    AZStd::intrusive_ptr<LiveLoggingUserSettings> LiveLoggingUserSettings::FindSettingsInstance()
    {
        return AZ::UserSettings::CreateFind<LiveLoggingUserSettings>(AZ_CRC("ScriptCanvas::LiveLoggingUserSettings", 0xc79efe7b), AZ::UserSettings::CT_LOCAL);
    }

    void LiveLoggingUserSettings::Reflect(AZ::ReflectContext* reflectContext)
    {
        AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext);

        if (serializeContext)
        {
            serializeContext->Class<LiveLoggingUserSettings>()
                ->Version(1)
                ->Field("AutoCapturing", &LiveLoggingUserSettings::m_isAutoCaptureEnabled)
                ->Field("LiveUpdating", &LiveLoggingUserSettings::m_enableLiveUpdates)
            ;
        }
    }

    void LiveLoggingUserSettings::SetAutoCaptureEnabled(bool enabled)
    {
        m_isAutoCaptureEnabled = enabled;
    }

    bool LiveLoggingUserSettings::IsAutoCaptureEnabled() const
    {
        return m_isAutoCaptureEnabled;
    }

    void LiveLoggingUserSettings::SetLiveUpdates(bool enabled)
    {
        m_enableLiveUpdates = enabled;
    }

    bool LiveLoggingUserSettings::IsLiveUpdating() const
    {
        return m_enableLiveUpdates;
    }

    /////////////////////////////
    // LiveLoggingWindowSession
    /////////////////////////////

    LiveLoggingWindowSession::LiveLoggingWindowSession(QWidget* parent)
        : LoggingWindowSession(parent)
        , m_startedSession(false)
        , m_encodeStaticEntities(false)
        , m_isCapturing(false)
    {
        AzFramework::TargetManagerClient::Bus::Handler::BusConnect();

        m_targetManagerModel = aznew TargetManagerModel();

        {
            QSignalBlocker signalBlocker(m_ui->targetSelector);
            m_ui->targetSelector->setModel(m_targetManagerModel);
        }

        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusConnect();
        ScriptCanvas::Debugger::ServiceNotificationsBus::Handler::BusConnect();

        SetDataId(m_liveDataAggregator.GetDataId());

        RegisterTreeRoot(m_liveDataAggregator.GetTreeRoot());

        m_userSettings = LiveLoggingUserSettings::FindSettingsInstance();

        if (!m_userSettings->IsLiveUpdating())
        {
            m_liveDataAggregator.GetTreeRoot()->SetUpdatePolicy(DebugLogRootItem::UpdatePolicy::SingleTime);
        }
        else
        {
            m_liveDataAggregator.GetTreeRoot()->SetUpdatePolicy(DebugLogRootItem::UpdatePolicy::RealTime);
        }        
        
        // Despite being apart of the base menu for now, the LiveLoggingWindow is the only one that needs to utilize these buttons.
        // Going to control them from here.
        m_ui->liveUpdatesToggle->setChecked(m_userSettings->IsLiveUpdating());
        QObject::connect(m_ui->liveUpdatesToggle, &QToolButton::toggled, this, &LiveLoggingWindowSession::OnLiveUpdateToggled);

        m_ui->autoCaptureToggle->setChecked(m_userSettings->IsAutoCaptureEnabled());
        QObject::connect(m_ui->autoCaptureToggle, &QToolButton::toggled, this, &LiveLoggingWindowSession::OnAutoCaptureToggled);
    }

    LiveLoggingWindowSession::~LiveLoggingWindowSession()
    {
        AzFramework::TargetManagerClient::Bus::Handler::BusDisconnect();
        AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusDisconnect();
        ScriptCanvas::Debugger::ServiceNotificationsBus::Handler::BusDisconnect();
    }

    void LiveLoggingWindowSession::DesiredTargetChanged(AZ::u32 newId, AZ::u32 oldId)
    {
        {
            QSignalBlocker signalBlocker(m_ui->targetSelector);

            int row = m_targetManagerModel->GetRowForTarget(newId);

            if (row < 0)
            {
                m_ui->targetSelector->setCurrentIndex(0);
            }
            else
            {
                m_ui->targetSelector->setCurrentIndex(row);
            }
        }
    }

    void LiveLoggingWindowSession::DesiredTargetConnected(bool connected)
    {
        {
            QSignalBlocker signalBlocker(m_ui->targetSelector);

            bool useFallback = !connected;

            if (connected)
            {
                AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusDisconnect();

                AzFramework::TargetInfo desiredInfo;
                AzFramework::TargetManager::Bus::BroadcastResult(desiredInfo, &AzFramework::TargetManager::GetDesiredTarget);

                if (desiredInfo.IsValid() && !desiredInfo.IsSelf())
                {
                    int index = m_targetManagerModel->GetRowForTarget(desiredInfo.GetPersistentId());

                    if (index > 0)
                    {
                        m_ui->targetSelector->setCurrentIndex(index);
                    }
                }
                else
                {
                    useFallback = true;
                }
            }
            else if (m_isCapturing)
            {
                SetIsCapturing(false);
            }
            
            if (useFallback)
            {
                if (!AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusIsConnected())
                {
                    AzToolsFramework::EditorEntityContextNotificationBus::Handler::BusConnect();
                }

                m_ui->targetSelector->setCurrentIndex(0);
            }
        }
    }

    void LiveLoggingWindowSession::TargetJoinedNetwork(AzFramework::TargetInfo info)
    {
        {
            QSignalBlocker signalBlocker(m_ui->targetSelector);

            m_targetManagerModel->TargetJoinedNetwork(info);
        }
    }

    void LiveLoggingWindowSession::TargetLeftNetwork(AzFramework::TargetInfo info)
    {
        {
            QSignalBlocker signalBlocker(m_ui->targetSelector);

            m_targetManagerModel->TargetLeftNetwork(info);
        }
    }

    void LiveLoggingWindowSession::OnStartPlayInEditorBegin()
    {
        if (isVisible())
        {
            m_encodeStaticEntities = true;
            ScriptCanvas::Debugger::ClientUIRequestBus::Broadcast(&ScriptCanvas::Debugger::ClientUIRequests::StartEditorSession);

            if ((m_userSettings->IsAutoCaptureEnabled()) || m_startedSession)
            {
                SetIsCapturing(true);
            }
        }
    }

    void LiveLoggingWindowSession::OnStopPlayInEditor()
    {
        if (isVisible())
        {
            SetIsCapturing(false);
            m_startedSession = false;

            ScriptCanvas::Debugger::ClientUIRequestBus::Broadcast(&ScriptCanvas::Debugger::ClientUIRequests::StopEditorSession);
            m_encodeStaticEntities = false;
        }
    }

    void LiveLoggingWindowSession::Connected(const ScriptCanvas::Debugger::Target& target)
    {
        if (m_userSettings->IsAutoCaptureEnabled() && isVisible())
        {
            SetIsCapturing(true);
        }
    }

    void LiveLoggingWindowSession::OnCaptureButtonPressed()
    {
        bool isSelfTarget = false;
        ScriptCanvas::Debugger::ClientRequestsBus::BroadcastResult(isSelfTarget, &ScriptCanvas::Debugger::ClientRequests::IsConnectedToSelf);

        if (isSelfTarget)
        {
            if (!m_startedSession)
            {
                bool isRunningGame = false;
                AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(isRunningGame, &AzToolsFramework::EditorEntityContextRequests::IsEditorRunningGame);

                if (!isRunningGame)
                {
                    m_startedSession = true;

                    GetIEditor()->SetInGameMode(true);
                    return;
                }
            }
            else
            {
                GetIEditor()->SetInGameMode(false);
                return;
            }
        }

        SetIsCapturing(!m_isCapturing);
    }

    void LiveLoggingWindowSession::OnPlaybackButtonPressed()
    {
        // Nothing to do in the LiveLoggingWindowSession
    }

    void LiveLoggingWindowSession::OnOptionsButtonPressed()
    {
        QPoint point = QCursor::pos();

        QMenu optionsMenu;

        QAction* autoCaptureAction = optionsMenu.addAction("Auto Capture");

        autoCaptureAction->setCheckable(true);
        autoCaptureAction->setChecked(m_userSettings->IsAutoCaptureEnabled());

        QObject::connect(autoCaptureAction, &QAction::toggled, this, &LiveLoggingWindowSession::OnAutoCaptureToggled);

        QAction* liveUpdateAction = optionsMenu.addAction("Live Updates");

        liveUpdateAction->setCheckable(true);
        liveUpdateAction->setChecked(m_userSettings->IsLiveUpdating());

        QObject::connect(liveUpdateAction, &QAction::toggled, this, &LiveLoggingWindowSession::OnLiveUpdateToggled);

        optionsMenu.exec(point);
    }
    
    void LiveLoggingWindowSession::OnTargetChanged(int index)
    {
        // Special case out the editor
        if (index == 0)
        {
            AzFramework::TargetManager::Bus::Broadcast(&AzFramework::TargetManager::SetDesiredTarget, 0);
        }
        else
        {
            AzFramework::TargetInfo info = m_targetManagerModel->FindTargetInfoForRow(index);

            if (info.IsValid())
            {
                AzFramework::TargetManager::Bus::Broadcast(&AzFramework::TargetManager::SetDesiredTarget, info.GetNetworkId());
            }
        }
    }

    void LiveLoggingWindowSession::OnAutoCaptureToggled(bool checked)
    {
        m_userSettings->SetAutoCaptureEnabled(checked);
    }

    void LiveLoggingWindowSession::OnLiveUpdateToggled(bool checked)
    {
        m_userSettings->SetLiveUpdates(checked);

        if (!m_userSettings->IsLiveUpdating())
        {
            m_liveDataAggregator.GetTreeRoot()->SetUpdatePolicy(DebugLogRootItem::UpdatePolicy::SingleTime);
        }
        else
        {
            // If we enable this we want to update the current display.
            m_liveDataAggregator.GetTreeRoot()->RedoLayout();
            m_liveDataAggregator.GetTreeRoot()->SetUpdatePolicy(DebugLogRootItem::UpdatePolicy::RealTime);
        }
    }

    void LiveLoggingWindowSession::StartDataCapture()
    {
        ScriptCanvas::Debugger::ScriptTarget captureInfo;

        ConfigureScriptTarget(captureInfo);

        m_liveDataAggregator.StartCaptureData();
        m_ui->captureButton->setIcon(QIcon(":/ScriptCanvasEditorResources/Resources/capture_live.png"));

        ScriptCanvas::Debugger::ClientUIRequestBus::Broadcast(&ScriptCanvas::Debugger::ClientUIRequests::StartLogging, captureInfo);
    }

    void LiveLoggingWindowSession::StopDataCapture()
    {
        m_liveDataAggregator.StopCaptureData();
        m_ui->captureButton->setIcon(QIcon(":/ScriptCanvasEditorResources/Resources/capture_offline.png"));

        ScriptCanvas::Debugger::ClientUIRequestBus::Broadcast(&ScriptCanvas::Debugger::ClientUIRequests::StopLogging);

        if (!m_userSettings->IsLiveUpdating())
        {
            m_liveDataAggregator.GetTreeRoot()->RedoLayout();
        }
    }

    void LiveLoggingWindowSession::ConfigureScriptTarget(ScriptCanvas::Debugger::ScriptTarget& captureInfo)
    {
        if (m_encodeStaticEntities)
        {
            const auto& staticRegistrations = m_liveDataAggregator.GetStaticRegistrations();

            for (const auto& registrationPair : staticRegistrations)
            {
                bool gotResult = false;
                AZ::EntityId runtimeId;
                
                AzToolsFramework::EditorEntityContextRequestBus::BroadcastResult(gotResult, &AzToolsFramework::EditorEntityContextRequests::MapEditorIdToRuntimeId, registrationPair.first, runtimeId);

                if (runtimeId.IsValid())
                {
                    auto entityIter = captureInfo.m_entities.find(runtimeId);

                    if (entityIter == captureInfo.m_entities.end())
                    {
                        auto insertResult = captureInfo.m_entities.insert(AZStd::make_pair(runtimeId, AZStd::unordered_set< ScriptCanvas::GraphIdentifier >()));
                        entityIter = insertResult.first;
                    }

                    entityIter->second.insert(registrationPair.second);

                    m_liveDataAggregator.RegisterEntityName(runtimeId, registrationPair.first.GetName());
                }
                else
                {
                    auto insertResult = captureInfo.m_staticEntities.insert(registrationPair.first);
                    insertResult.first->second.insert(registrationPair.second);
                }
            }
        }
        
        const LoggingEntityMap& registrationMap = m_liveDataAggregator.GetLoggingEntityMap();

        for (const auto& registrationPair : registrationMap)
        {
            auto entityIter = captureInfo.m_entities.find(registrationPair.first);

            if (entityIter == captureInfo.m_entities.end())
            {
                auto insertResult = captureInfo.m_entities.insert(AZStd::make_pair(registrationPair.first, AZStd::unordered_set< ScriptCanvas::GraphIdentifier >()));
                entityIter = insertResult.first;
            }

            entityIter->second.insert(registrationPair.second);
        }

        const LoggingAssetSet& registrationSet = m_liveDataAggregator.GetLoggingAssetSet();

        for (const auto& graphIdentifier : registrationSet)
        {
            captureInfo.m_graphs.insert(graphIdentifier.m_assetId);
        }
    }

    void LiveLoggingWindowSession::SetIsCapturing(bool isCapturing)
    {
        if (isCapturing != m_isCapturing)
        {
            m_isCapturing = isCapturing;

            if (m_isCapturing)
            {
                StartDataCapture();
            }
            else
            {
                StopDataCapture();
            }
        }
    }

#include <Editor/View/Widgets/LoggingPanel/LiveWindowSession/LiveLoggingWindowSession.moc>
}
