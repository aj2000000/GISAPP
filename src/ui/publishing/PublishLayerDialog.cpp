/**
 * @file PublishLayerDialog.cpp
 * @brief Implementation of PublishLayerDialog UI layout, custom tactical styling, and signals.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "ui/publishing/PublishLayerDialog.h"
#include "ui/publishing/GroupManagerDialog.h"
#include "publishing/LayerRegistryManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QGroupBox>
#include <QCoreApplication>

namespace GISApp::UI::Publishing {

PublishLayerDialog::PublishLayerDialog(GISApp::Layers::LayerManager *layerManager,
                                       QMapLibre::Map *mapInstance,
                                       QWidget *parent)
    : QDialog(parent)
    , m_layerManager(layerManager)
    , m_mapInstance(mapInstance)
{
    setWindowTitle(tr("Publish Layer - Precision GIS"));
    setMinimumSize(540, 620);
    setupUI();
    populateGroups();
}

void PublishLayerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(18, 18, 18, 18);

    // ── 1. Layer Modality Selection Card ─────────────────────────────
    QGroupBox *boxType = new QGroupBox(tr("1. Select Layer Modality"), this);
    QVBoxLayout *typeLayout = new QVBoxLayout(boxType);
    typeLayout->setSpacing(8);

    m_radioRaster = new QRadioButton(tr("Raster Map (Folder with GeoTIFF .tif files)"), boxType);
    m_radioVector = new QRadioButton(tr("Vector Data (.geojson or .shp file set)"), boxType);
    m_radioRaster->setChecked(true);

    typeLayout->addWidget(m_radioRaster);
    typeLayout->addWidget(m_radioVector);
    mainLayout->addWidget(boxType);

    connect(m_radioRaster, &QRadioButton::toggled, this, &PublishLayerDialog::updateFilePreview);
    connect(m_radioVector, &QRadioButton::toggled, this, &PublishLayerDialog::updateFilePreview);

    // ── 2. Folder Selection & Source Discovery Card ──────────────────
    QGroupBox *boxSource = new QGroupBox(tr("2. Select Spatial Source Directory"), this);
    QVBoxLayout *sourceLayout = new QVBoxLayout(boxSource);
    sourceLayout->setSpacing(10);

    QHBoxLayout *folderLayout = new QHBoxLayout();
    m_editFolderPath = new QLineEdit(boxSource);
    m_editFolderPath->setPlaceholderText(tr("Select folder containing spatial layer files..."));
    
    m_btnBrowse = new QPushButton(tr("📁 Browse..."), boxSource);
    m_btnBrowse->setCursor(Qt::PointingHandCursor);

    folderLayout->addWidget(m_editFolderPath, 1);
    folderLayout->addWidget(m_btnBrowse);
    sourceLayout->addLayout(folderLayout);

    QLabel *lblPreview = new QLabel(tr("Discovered Spatial Source Files:"), boxSource);
    lblPreview->setStyleSheet("color: #9ca3af; font-size: 11px; font-weight: bold;");
    sourceLayout->addWidget(lblPreview);

    m_listFilePreview = new QListWidget(boxSource);
    m_listFilePreview->setMaximumHeight(120);
    sourceLayout->addWidget(m_listFilePreview);

    mainLayout->addWidget(boxSource);

    connect(m_btnBrowse, &QPushButton::clicked, this, &PublishLayerDialog::onBrowseFolder);

    // ── 3. Layer Metadata & Group Assignment Card ───────────────────
    QGroupBox *boxMeta = new QGroupBox(tr("3. Layer Properties & Tree Placement"), this);
    QFormLayout *formLayout = new QFormLayout(boxMeta);
    formLayout->setSpacing(10);

    m_editLayerName = new QLineEdit(boxMeta);
    m_editLayerName->setPlaceholderText(tr("Enter published layer display name"));
    formLayout->addRow(tr("Layer Name:"), m_editLayerName);

    QHBoxLayout *groupLayout = new QHBoxLayout();
    m_comboGroups = new QComboBox(boxMeta);
    m_btnNewGroup = new QPushButton(tr("+ New Group"), boxMeta);
    m_btnNewGroup->setCursor(Qt::PointingHandCursor);

    groupLayout->addWidget(m_comboGroups, 1);
    groupLayout->addWidget(m_btnNewGroup);
    formLayout->addRow(tr("Assign Group:"), groupLayout);

    mainLayout->addWidget(boxMeta);

    connect(m_btnNewGroup, &QPushButton::clicked, this, &PublishLayerDialog::onCreateGroupClicked);

    // ── 4. Progress Bar & Status Line ────────────────────────────────
    QVBoxLayout *progressLayout = new QVBoxLayout();
    progressLayout->setSpacing(4);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    progressLayout->addWidget(m_progressBar);

    m_lblStatus = new QLabel(tr("Ready to publish."), this);
    m_lblStatus->setStyleSheet("color: #9ca3af; font-size: 11px; font-style: italic;");
    progressLayout->addWidget(m_lblStatus);

    mainLayout->addLayout(progressLayout);

    // ── 5. Footer Dialog Buttons ─────────────────────────────────────
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_btnPublish = new QPushButton(tr("🚀 Publish Layer"), this);
    m_btnPublish->setObjectName("btnPublish");
    m_btnPublish->setCursor(Qt::PointingHandCursor);

    m_btnCancel = new QPushButton(tr("Cancel"), this);
    m_btnCancel->setCursor(Qt::PointingHandCursor);

    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnPublish);
    mainLayout->addLayout(btnLayout);

    connect(m_btnPublish, &QPushButton::clicked, this, &PublishLayerDialog::onPublishClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    // ── 6. Precision Tactical Dark QSS Theme ─────────────────────────
    setStyleSheet(R"(
        QDialog {
            background-color: #0f172a;
            color: #f8fafc;
            font-family: 'Segoe UI', Inter, sans-serif;
        }

        QGroupBox {
            background-color: #1e293b;
            border: 1px solid #334155;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 14px;
            font-size: 12px;
            font-weight: bold;
            color: #38bdf8;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 6px;
            background-color: #0f172a;
            border-radius: 4px;
        }

        QLabel {
            color: #e2e8f0;
            font-size: 12px;
        }

        QRadioButton {
            color: #f1f5f9;
            font-size: 13px;
            font-weight: 500;
            spacing: 8px;
            padding: 4px;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            border-radius: 8px;
            border: 2px solid #64748b;
            background-color: #0f172a;
        }
        QRadioButton::indicator:checked {
            border: 2px solid #10b981;
            background-color: #10b981;
        }
        QRadioButton::indicator:hover {
            border-color: #38bdf8;
        }

        QLineEdit {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 6px;
            color: #ffffff;
            padding: 8px 12px;
            font-size: 13px;
            selection-background-color: #10b981;
        }
        QLineEdit:focus {
            border: 1px solid #10b981;
            background-color: #020617;
        }
        QLineEdit::placeholder {
            color: #64748b;
        }

        QListWidget {
            background-color: #020617;
            border: 1px solid #334155;
            border-radius: 6px;
            color: #cbd5e1;
            padding: 6px;
            font-family: 'Cascadia Code', 'Fira Code', monospace;
            font-size: 11px;
        }
        QListWidget::item {
            padding: 4px 8px;
            border-radius: 4px;
        }
        QListWidget::item:hover {
            background-color: #1e293b;
            color: #38bdf8;
        }

        QComboBox {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 6px;
            color: #ffffff;
            padding: 6px 12px;
            font-size: 13px;
        }
        QComboBox:hover {
            border-color: #38bdf8;
        }
        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #ffffff;
            border: 1px solid #334155;
            selection-background-color: #10b981;
            selection-color: #000000;
        }

        QProgressBar {
            background-color: #0f172a;
            border: 1px solid #334155;
            border-radius: 6px;
            text-align: center;
            color: #f8fafc;
            font-weight: bold;
            font-size: 11px;
            height: 22px;
        }
        QProgressBar::chunk {
            background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 #059669, stop:1 #10b981);
            border-radius: 5px;
        }

        QPushButton {
            background-color: #334155;
            color: #f8fafc;
            border: 1px solid #475569;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 13px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: #475569;
            border-color: #94a3b8;
        }
        QPushButton#btnPublish {
            background-color: #10b981;
            color: #020617;
            border: 1px solid #34d399;
            font-weight: bold;
        }
        QPushButton#btnPublish:hover {
            background-color: #059669;
            color: #ffffff;
        }
    )");
}

void PublishLayerDialog::populateGroups()
{
    m_comboGroups->clear();
    m_comboGroups->addItem(tr("(Root Level)"), QVariant::fromValue<void*>(nullptr));

    if (m_layerManager && m_layerManager->model()) {
        auto root = m_layerManager->model()->rootNode();
        for (int i = 0; i < root->childCount(); ++i) {
            auto child = root->child(i);
            if (child->nodeType() == GISApp::Layers::NodeType::Group) {
                m_comboGroups->addItem("📁 " + child->name(), QVariant::fromValue<void*>(child));
            }
        }
    }

    int rasterIdx = m_comboGroups->findText("📁 🗺️ Raster Imagery & DSM");
    if (rasterIdx != -1) {
        m_comboGroups->setCurrentIndex(rasterIdx);
    }
}

void PublishLayerDialog::onBrowseFolder()
{
    if (m_radioVector->isChecked()) {
        QString fileOrDir = QFileDialog::getOpenFileName(
            this,
            tr("Select Vector File (.geojson, .json, .shp)"),
            m_editFolderPath->text(),
            tr("Vector Spatial Files (*.geojson *.json *.shp);;GeoJSON Files (*.geojson *.json);;Shapefiles (*.shp);;All Files (*)")
        );
        if (fileOrDir.isEmpty()) {
            fileOrDir = QFileDialog::getExistingDirectory(this, tr("Or Select Directory Containing Vector Files"));
        }
        if (!fileOrDir.isEmpty()) {
            m_editFolderPath->setText(fileOrDir);
            updateFilePreview();
            if (m_editLayerName->text().trimmed().isEmpty()) {
                QFileInfo fi(fileOrDir);
                m_editLayerName->setText(fi.completeBaseName());
            }
        }
    } else {
        QString folder = QFileDialog::getExistingDirectory(this, tr("Select GeoTIFF Raster Directory"));
        if (!folder.isEmpty()) {
            m_editFolderPath->setText(folder);
            updateFilePreview();
        }
    }
}

void PublishLayerDialog::updateFilePreview()
{
    m_listFilePreview->clear();
    QString pathStr = m_editFolderPath->text();
    if (pathStr.isEmpty()) return;

    QFileInfo pathInfo(pathStr);
    if (pathInfo.isFile()) {
        m_listFilePreview->addItem("📄 " + pathInfo.fileName() + QString(" (%1 KB)").arg(pathInfo.size() / 1024));
        return;
    }

    QDir dir(pathStr);
    QStringList filters;
    if (m_radioRaster->isChecked()) {
        filters << "*.tif" << "*.tiff" << "*.TIF" << "*.TIFF";
    } else {
        filters << "*.geojson" << "*.json" << "*.shp" << "*.dbf" << "*.prj" << "*.sld" << "*.shx";
    }

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const auto &file : files) {
        m_listFilePreview->addItem("📄 " + file.fileName() + QString(" (%1 KB)").arg(file.size() / 1024));
    }
}

void PublishLayerDialog::onCreateGroupClicked()
{
    GroupManagerDialog dialog(m_layerManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        populateGroups();
    }
}

void PublishLayerDialog::onPublishClicked()
{
    QString folder = m_editFolderPath->text();
    QString name = m_editLayerName->text().trimmed();

    if (folder.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Please select a folder and specify a layer name."));
        return;
    }

    GISApp::Layers::LayerGroupNode *targetGroup = static_cast<GISApp::Layers::LayerGroupNode*>(
        m_comboGroups->currentData().value<void*>());

    m_btnPublish->setEnabled(false);
    m_btnCancel->setEnabled(false);
    m_progressBar->setValue(0);
    m_lblStatus->setText(tr("Initializing spatial ingestion pipeline..."));
    QCoreApplication::processEvents();

    GISApp::Publishing::LayerType type = m_radioRaster->isChecked() ?
        GISApp::Publishing::LayerType::Raster : GISApp::Publishing::LayerType::Vector;

    auto progressCb = [this](int percent, const QString &statusText) {
        m_progressBar->setValue(percent);
        m_lblStatus->setText(statusText);
        QCoreApplication::processEvents();
    };

    bool success = m_publishingService.publishLayer(type, folder, name, targetGroup, m_layerManager, m_mapInstance, progressCb);

    m_progressBar->setValue(100);
    m_lblStatus->setText(m_publishingService.lastStatusMessage());
    QCoreApplication::processEvents();

    m_btnPublish->setEnabled(true);
    m_btnCancel->setEnabled(true);

    if (success) {
        QString groupName = targetGroup ? targetGroup->name() : "";
        GISApp::Publishing::LayerRegistryManager::instance().registerPublishedLayer(type, folder, name, groupName);
        QMessageBox::information(this, tr("Success"), tr("Layer successfully published!"));
        accept();
    } else {
        QMessageBox::critical(this, tr("Publishing Failed"), m_publishingService.lastStatusMessage());
    }
}

} // namespace GISApp::UI::Publishing
