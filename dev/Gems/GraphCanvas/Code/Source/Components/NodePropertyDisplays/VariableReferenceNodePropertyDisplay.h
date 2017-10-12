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

#include <qabstractitemmodel.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qcompleter.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qsortfilterproxymodel.h>

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Components/NodePropertyDisplays/StringNodePropertyDisplay.h>

#include <Components/NodePropertyDisplay/NodePropertyDisplay.h>
#include <Components/NodePropertyDisplay/VariableDataInterface.h>
#include <GraphCanvas/Components/Nodes/Variable/VariableNodeBus.h>

namespace GraphCanvas
{
    class GraphCanvasLabel;

    // TODO: Make this into a single static instance that gets updated for each scene
    //       rather then a 1:1 relationship with the number of variable elements we have.
    class VariableItemModel
        : public QAbstractListModel
    {
    public:
        AZ_CLASS_ALLOCATOR(VariableItemModel, AZ::SystemAllocator, 0);

        VariableItemModel();
        ~VariableItemModel() override;

        // QAstractListModel
        int rowCount(const QModelIndex& parent) const override;
        QVariant data(const QModelIndex& index, int role) const override;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        ////

        void SetSceneId(const AZ::EntityId& sceneId);
        void SetDataType(const AZ::Uuid& variableType);
        void RefreshData();
        void ClearData();

        int FindRowForVariable(const AZ::EntityId& variableId) const;
        AZ::EntityId FindVariableIdForRow(int row) const;
        AZ::EntityId FindVariableIdForName(const AZStd::string& variableName) const;

    private:

        AZ::EntityId m_sceneId;

        AZ::Uuid m_dataType;
        AZStd::vector< AZ::EntityId > m_variableIds;
    };

    class VariableSelectionWidget
        : public QWidget
        , public AzToolsFramework::EditorEvents::Bus::Handler
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(VariableSelectionWidget, AZ::SystemAllocator, 0);

        VariableSelectionWidget();
        ~VariableSelectionWidget();

        void SetSceneId(const AZ::EntityId& sceneId);
        void SetDataType(const AZ::Uuid& dataType);
        void SetSelectedVariable(const AZ::EntityId& variableId);

        // AzToolsFramework::EditorEvents::Bus
        void OnEscape() override;
        ////

    public slots:

        // Line Edit
        void HandleFocusIn();
        void HandleFocusOut();
        void SubmitName();

    signals:

        void OnFocusIn();
        void OnFocusOut();

        void OnVariableSelected(const AZ::EntityId& variableId);

    private:

        Internal::FocusableLineEdit*        m_lineEdit;
        VariableItemModel*                  m_itemModel;

        QCompleter*                         m_completer;

        QVBoxLayout*                        m_layout;

        AZ::EntityId m_initialVariable;
    };

    class VariableReferenceNodePropertyDisplay
        : public NodePropertyDisplay
        , public VariableNotificationBus::Handler
    {
    public:
        AZ_CLASS_ALLOCATOR(VariableReferenceNodePropertyDisplay, AZ::SystemAllocator, 0);
        VariableReferenceNodePropertyDisplay(VariableReferenceDataInterface* dataInterface);
        ~VariableReferenceNodePropertyDisplay() override;
        
        // NodePropertyDisplay
        void RefreshStyle() override;
        void UpdateDisplay() override;
        
        QGraphicsLayoutItem* GetDisabledGraphicsLayoutItem() const override;
        QGraphicsLayoutItem* GetDisplayGraphicsLayoutItem() const override;
        QGraphicsLayoutItem* GetEditableGraphicsLayoutItem() const override;
        ////

        // VariableNotificationBus
        void OnNameChanged() override;
        void OnVariableActivated() override;
        ////
        
    private:

        // NodePropertyDisplay
        void OnIdSet() override;
        ////

        void DisplayVariableString(const AZ::EntityId& variableId);

        void EditStart();
        void AssignVariable(const AZ::EntityId& variableId);
        void EditFinished();

        VariableReferenceDataInterface*             m_variableReferenceDataInterface;
        
        GraphCanvasLabel*                           m_disabledLabel;
        GraphCanvasLabel*                           m_displayLabel;

        QGraphicsProxyWidget*                       m_proxyWidget;
        VariableSelectionWidget*                    m_variableSelectionWidget;
    };
}