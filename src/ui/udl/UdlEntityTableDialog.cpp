/**
 * @file UdlEntityTableDialog.cpp
 * @brief Implementation of UdlEntityTableDialog.
 */

#include "src/ui/udl/UdlEntityTableDialog.h"
#include "src/publishing/UdlRepositoryManager.h"
#include "publishing/LayerRegistryManager.h"
#include "src/ui/udl/UdlEntityStyleDialog.h"
#include "core/notifications/NotificationManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace GISApp::UI::UDL {

UdlEntityTableDialog::UdlEntityTableDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("UDL Entity Manager"));
    resize(720, 480);
    setStyleSheet("QDialog { background-color: #1e1e2e; color: #cdd6f4; }"
                  "QLabel { color: #cdd6f4; font-weight: bold; font-size: 12px; }"
                  "QComboBox { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 6px; padding: 4px 28px 4px 10px; font-weight: bold; font-size: 12px; }"
                  "QComboBox:hover, QComboBox:focus { border-color: #89b4fa; }"
                  "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 24px; border-left: none; background: transparent; }"
                  "QComboBox::down-arrow { width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #89b4fa; margin-right: 8px; }"
                  "QComboBox QAbstractItemView { background-color: #181825; color: #f3f4f6; border: 1px solid #89b4fa; border-radius: 6px; padding: 4px; outline: 0px; selection-background-color: #89b4fa; selection-color: #11111b; }"
                  "QComboBox QAbstractItemView::item { min-height: 24px; padding: 4px 10px; color: #f3f4f6; background-color: transparent; }"
                  "QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected { background-color: #89b4fa; color: #11111b; font-weight: bold; border-radius: 4px; }"
                  "QTableWidget { background-color: #181825; alternate-background-color: #11111b; color: #f3f4f6; gridline-color: #313244; border: 1px solid #45475a; border-radius: 8px; selection-background-color: #89b4fa; selection-color: #11111b; outline: 0px; font-size: 12px; }"
                  "QTableWidget::item { color: #f3f4f6; padding: 6px 8px; border-bottom: 1px solid rgba(255, 255, 255, 0.05); }"
                  "QTableWidget::item:hover { background-color: rgba(137, 180, 250, 0.15); color: #ffffff; }"
                  "QTableWidget::item:selected { background-color: #89b4fa; color: #11111b; font-weight: bold; }"
                  "QHeaderView { background-color: #11111b; border: none; border-bottom: 2px solid #89b4fa; }"
                  "QHeaderView::section { background-color: #11111b; color: #89b4fa; padding: 8px 10px; font-weight: bold; font-size: 12px; border: none; border-right: 1px solid #313244; border-bottom: 2px solid #89b4fa; }"
                  "QHeaderView::section:hover { background-color: #181825; color: #ffffff; }"
                  "QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 6px; padding: 6px 12px; font-weight: bold; }"
                  "QPushButton:hover { background-color: #45475a; color: #ffffff; border-color: #89b4fa; }");

    setupUi();
    refreshData();
}

void UdlEntityTableDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);

    auto topLayout = new QHBoxLayout();
    topLayout->addWidget(new QLabel(tr("Filter by Layer:"), this));

    m_layerFilterCombo = new QComboBox(this);
    connect(m_layerFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        populateTable();
    });
    topLayout->addWidget(m_layerFilterCombo);

    topLayout->addStretch();

    m_btnRefresh = new QPushButton(tr("🔄 Refresh"), this);
    connect(m_btnRefresh, &QPushButton::clicked, this, &UdlEntityTableDialog::refreshData);
    topLayout->addWidget(m_btnRefresh);

    auto btnClearClipboard = new QPushButton(tr("🧹 Clear Clipboard"), this);
    btnClearClipboard->setStyleSheet("background-color: #45475a; color: #f3f4f6; border: 1px solid #45475a; border-radius: 6px; padding: 6px 12px; font-weight: bold;");
    connect(btnClearClipboard, &QPushButton::clicked, this, []() {
        GISApp::Publishing::UdlRepositoryManager::instance().clearCopiedEntity();
        GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
            "UDL Clipboard", "UDL Clipboard cleared.");
    });
    topLayout->addWidget(btnClearClipboard);

    mainLayout->addLayout(topLayout);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels({tr("Entity ID"), tr("Name"), tr("Type"), tr("Stroke"), tr("Created At"), tr("Actions")});
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->verticalHeader()->setDefaultSectionSize(38);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(m_tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int row, int) {
        QString geomStr = m_tableWidget->item(row, 0)->data(Qt::UserRole).toString();
        QJsonObject geomObj = QJsonDocument::fromJson(geomStr.toUtf8()).object();
        QString type = geomObj["type"].toString();
        if (type == "Point" && geomObj.contains("coordinates")) {
            QJsonArray pt = geomObj["coordinates"].toArray();
            if (pt.size() >= 2) {
                emit zoomToEntityRequested(pt[1].toDouble(), pt[0].toDouble());
            }
        } else if (type == "LineString" && geomObj.contains("coordinates")) {
            QJsonArray line = geomObj["coordinates"].toArray();
            if (!line.isEmpty()) {
                QJsonArray pt = line[0].toArray();
                if (pt.size() >= 2) {
                    emit zoomToEntityRequested(pt[1].toDouble(), pt[0].toDouble());
                }
            }
        }
    });

    mainLayout->addWidget(m_tableWidget);
}

void UdlEntityTableDialog::refreshData() {
    if (!m_layerFilterCombo) return;
    m_layerFilterCombo->blockSignals(true);
    m_layerFilterCombo->clear();
    m_layerFilterCombo->addItem(tr("All UDL Layers"), "ALL");

    auto layers = GISApp::Publishing::LayerRegistryManager::instance().getSavedLayers();
    for (const auto &meta : layers) {
        if (meta.groupName.contains("User Defined Layers") || meta.groupName.contains("UDL") || meta.groupName.contains("🎨")) {
            QString lId = meta.layerId.isEmpty() ? meta.name : meta.layerId;
            m_layerFilterCombo->addItem(QString("📍 %1").arg(meta.name), lId);
        }
    }
    m_layerFilterCombo->blockSignals(false);
    populateTable();
}

void UdlEntityTableDialog::populateTable() {
    if (!m_tableWidget || !m_layerFilterCombo) return;
    m_tableWidget->setRowCount(0);

    QString filterLid = m_layerFilterCombo->currentData().toString();
    QList<GISApp::Publishing::UdlEntityItem> items;

    if (filterLid == "ALL" || filterLid.isEmpty()) {
        items = GISApp::Publishing::UdlRepositoryManager::instance().getAllEntities();
    } else {
        items = GISApp::Publishing::UdlRepositoryManager::instance().getEntitiesForLayer(filterLid);
    }

    m_tableWidget->setRowCount(items.size());
    for (int i = 0; i < items.size(); ++i) {
        const auto &item = items[i];

        auto item0 = new QTableWidgetItem(item.entityId);
        item0->setData(Qt::UserRole, QString(QJsonDocument(item.geometryJson).toJson(QJsonDocument::Compact)));
        m_tableWidget->setItem(i, 0, item0);

        m_tableWidget->setItem(i, 1, new QTableWidgetItem(item.entityName));
        m_tableWidget->setItem(i, 2, new QTableWidgetItem(item.entityType));

        QString color = item.styleJson.value("strokeColor").toString("#f59e0b");
        auto strokeItem = new QTableWidgetItem(color);
        strokeItem->setBackground(QColor(color));
        strokeItem->setForeground(QColor(QColor(color).lightness() < 128 ? "#ffffff" : "#000000"));
        m_tableWidget->setItem(i, 3, strokeItem);

        m_tableWidget->setItem(i, 4, new QTableWidgetItem(item.createdAt));

        auto actionWidget = new QWidget(this);
        auto actionLayout = new QHBoxLayout(actionWidget);
        actionLayout->setContentsMargins(4, 2, 4, 2);
        actionLayout->setSpacing(6);
        actionLayout->setAlignment(Qt::AlignCenter);

        auto btnCopy = new QPushButton(tr("📋 Copy"), this);
        btnCopy->setCursor(Qt::PointingHandCursor);
        btnCopy->setStyleSheet(
            "QPushButton { background-color: #059669; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px 10px; font-size: 11px; border: none; }"
            "QPushButton:hover { background-color: #10b981; color: #ffffff; }"
        );
        actionLayout->addWidget(btnCopy);

        auto btnEdit = new QPushButton(tr("✏️ Edit"), this);
        btnEdit->setCursor(Qt::PointingHandCursor);
        btnEdit->setStyleSheet(
            "QPushButton { background-color: #3158c4; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px 10px; font-size: 11px; border: none; }"
            "QPushButton:hover { background-color: #4b73e5; color: #ffffff; }"
        );
        actionLayout->addWidget(btnEdit);

        auto btnDelete = new QPushButton(tr("🗑️ Delete"), this);
        btnDelete->setCursor(Qt::PointingHandCursor);
        btnDelete->setStyleSheet(
            "QPushButton { background-color: #c0392b; color: #ffffff; font-weight: bold; border-radius: 4px; padding: 4px 10px; font-size: 11px; border: none; }"
            "QPushButton:hover { background-color: #e74c3c; color: #ffffff; }"
        );
        actionLayout->addWidget(btnDelete);

        QString eId = item.entityId;
        QString lId = item.layerId;
        GISApp::Publishing::UdlEntityItem itemCopy = item;

        connect(btnCopy, &QPushButton::clicked, this, [itemCopy]() {
            GISApp::Publishing::UdlRepositoryManager::instance().setCopiedEntity(itemCopy);
            GISApp::Core::Notifications::NotificationManager::instance()->notifyFlash(
                "Clipboard", QString("Copied UDL Entity '%1' to clipboard").arg(itemCopy.entityName));
        });

        connect(btnEdit, &QPushButton::clicked, this, [this, itemCopy]() {
            GISApp::UI::UDL::UdlGeometryType gType = GISApp::UI::UDL::UdlGeometryType::Point;
            if (itemCopy.entityType == "Polyline" || itemCopy.entityType == "LineString") gType = GISApp::UI::UDL::UdlGeometryType::Polyline;
            else if (itemCopy.entityType == "Polygon") gType = GISApp::UI::UDL::UdlGeometryType::Polygon;
            else if (itemCopy.entityType == "Circle") gType = GISApp::UI::UDL::UdlGeometryType::Circle;
            else if (itemCopy.entityType == "Text") gType = GISApp::UI::UDL::UdlGeometryType::Text;

            GISApp::UI::UDL::UdlEntityStyleDialog dlg(gType, this);
            dlg.setEntityName(itemCopy.entityName);
            dlg.setStyleJsonObject(itemCopy.styleJson);

            if (dlg.exec() == QDialog::Accepted) {
                GISApp::Publishing::UdlEntityItem updatedItem = itemCopy;
                updatedItem.entityName = dlg.entityName();
                updatedItem.styleJson = dlg.styleJsonObject();
                GISApp::Publishing::UdlRepositoryManager::instance().saveEntity(updatedItem);
                populateTable();
            }
        });

        connect(btnDelete, &QPushButton::clicked, this, [this, eId, lId]() {
            if (QMessageBox::question(this, tr("Delete Entity"), tr("Delete entity %1?").arg(eId)) == QMessageBox::Yes) {
                GISApp::Publishing::UdlRepositoryManager::instance().deleteEntity(eId, lId);
                populateTable();
            }
        });

        m_tableWidget->setCellWidget(i, 5, actionWidget);
    }
}

} // namespace GISApp::UI::UDL
