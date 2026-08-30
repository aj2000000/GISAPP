/**
 * @file DownloadSatImageryDialog.cpp
 * @brief Implementation of DownloadSatImageryDialog bounding box & zoom selection UI.
 */

#include "ui/download/DownloadSatImageryDialog.h"
#include "core/SystemConfigManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QMapLibre/Map>

namespace GISApp::UI::Download {

DownloadSatImageryDialog::DownloadSatImageryDialog(GISApp::Map::MapLibreWidget *mapWidget, QWidget *parent)
    : QDialog(parent)
    , m_mapWidget(mapWidget)
{
    setWindowTitle("🌐 Download Google Earth Satellite Imagery");
    resize(520, 480);
    setupUi();
    populateFromMapExtent();
    updateTileEstimate();
}

void DownloadSatImageryDialog::setupUi()
{
    setStyleSheet(R"(
        QDialog, QMessageBox {
            background-color: #0f172a;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 8px;
        }
        QLabel, QMessageBox QLabel {
            color: #f8fafc;
            font-size: 12px;
        }
        QMessageBox QLabel {
            font-size: 13px;
            font-weight: 500;
        }
        QMessageBox QPushButton {
            background-color: #1e293b;
            color: #38bdf8;
            border: 1px solid #0284c7;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: bold;
            font-size: 12px;
            min-width: 75px;
        }
        QMessageBox QPushButton:hover {
            background-color: #0284c7;
            color: #ffffff;
        }
        QGroupBox {
            color: #38bdf8;
            font-weight: bold;
            border: 1px solid #334155;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
        }
        QLineEdit {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QSpinBox, QDoubleSpinBox {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px 24px 4px 10px;
            font-weight: 500;
            font-size: 12px;
        }
        QSpinBox:hover, QSpinBox:focus, QDoubleSpinBox:hover, QDoubleSpinBox:focus {
            border-color: #38bdf8;
        }
        QSpinBox::up-button, QDoubleSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 20px;
            border-left: 1px solid #334155;
            border-bottom: 1px solid #334155;
            background-color: #0f172a;
            border-top-right-radius: 5px;
        }
        QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {
            background-color: #38bdf8;
        }
        QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 5px solid #38bdf8;
        }
        QSpinBox::up-button:hover QSpinBox::up-arrow, QDoubleSpinBox::up-button:hover QDoubleSpinBox::up-arrow {
            border-bottom-color: #0f172a;
        }
        QSpinBox::down-button, QDoubleSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 20px;
            border-left: 1px solid #334155;
            background-color: #0f172a;
            border-bottom-right-radius: 5px;
        }
        QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {
            background-color: #38bdf8;
        }
        QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid #38bdf8;
        }
        QSpinBox::down-button:hover QSpinBox::down-arrow, QDoubleSpinBox::down-button:hover QDoubleSpinBox::down-arrow {
            border-top-color: #0f172a;
        }
        QPushButton {
            background-color: #1e293b;
            color: #f8fafc;
            border: 1px solid #475569;
            border-radius: 4px;
            padding: 6px 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #334155;
            color: #38bdf8;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);

    // Header Label
    QLabel *headerLabel = new QLabel("<b>Download Google Satellite Tiles & Stitch to GeoTIFF</b>", this);
    headerLabel->setStyleSheet("font-size: 14px; color: #38bdf8;");
    mainLayout->addWidget(headerLabel);

    // 1. Area Bounding Box Selection
    QGroupBox *boxGroup = new QGroupBox("1. Target Bounding Box (WGS 84 Coordinates)", this);
    QGridLayout *boxLayout = new QGridLayout(boxGroup);

    m_northSpin = new QDoubleSpinBox(this);
    m_northSpin->setRange(-90.0, 90.0);
    m_northSpin->setDecimals(5);
    m_northSpin->setSuffix("° N");

    m_southSpin = new QDoubleSpinBox(this);
    m_southSpin->setRange(-90.0, 90.0);
    m_southSpin->setDecimals(5);
    m_southSpin->setSuffix("° S");

    m_eastSpin = new QDoubleSpinBox(this);
    m_eastSpin->setRange(-180.0, 180.0);
    m_eastSpin->setDecimals(5);
    m_eastSpin->setSuffix("° E");

    m_westSpin = new QDoubleSpinBox(this);
    m_westSpin->setRange(-180.0, 180.0);
    m_westSpin->setDecimals(5);
    m_westSpin->setSuffix("° W");

    boxLayout->addWidget(new QLabel("North (Max Lat):", this), 0, 0);
    boxLayout->addWidget(m_northSpin, 0, 1);
    boxLayout->addWidget(new QLabel("South (Min Lat):", this), 1, 0);
    boxLayout->addWidget(m_southSpin, 1, 1);
    boxLayout->addWidget(new QLabel("West (Min Lon):", this), 2, 0);
    boxLayout->addWidget(m_westSpin, 2, 1);
    boxLayout->addWidget(new QLabel("East (Max Lon):", this), 3, 0);
    boxLayout->addWidget(m_eastSpin, 3, 1);

    QPushButton *useExtentBtn = new QPushButton("📍 Use Current Map Extent", this);
    useExtentBtn->setStyleSheet("background-color: #1e293b; color: #e2e8f0; font-weight: bold; padding: 6px;");
    connect(useExtentBtn, &QPushButton::clicked, this, &DownloadSatImageryDialog::populateFromMapExtent);
    boxLayout->addWidget(useExtentBtn, 4, 0, 1, 2);

    mainLayout->addWidget(boxGroup);

    // 2. Zoom Level Selection
    QGroupBox *zoomGroup = new QGroupBox("2. Zoom Level Selection", this);
    QGridLayout *zoomLayout = new QGridLayout(zoomGroup);

    m_minZoomSpin = new QSpinBox(this);
    m_minZoomSpin->setRange(0, 18);
    m_minZoomSpin->setValue(10);

    m_maxZoomSpin = new QSpinBox(this);
    m_maxZoomSpin->setRange(0, 18);
    m_maxZoomSpin->setValue(14);

    zoomLayout->addWidget(new QLabel("Min Zoom:", this), 0, 0);
    zoomLayout->addWidget(m_minZoomSpin, 0, 1);
    zoomLayout->addWidget(new QLabel("Max Zoom (Target Detail):", this), 1, 0);
    zoomLayout->addWidget(m_maxZoomSpin, 1, 1);

    m_estimateLabel = new QLabel("Calculated tiles: 0 tiles (~0.0 MB)", this);
    m_estimateLabel->setStyleSheet("color: #38bdf8; font-weight: bold;");
    zoomLayout->addWidget(m_estimateLabel, 2, 0, 1, 2);

    mainLayout->addWidget(zoomGroup);

    // 3. Output Location
    QGroupBox *outGroup = new QGroupBox("3. Output GeoTIFF File Location", this);
    QHBoxLayout *outLayout = new QHBoxLayout(outGroup);

    QString defaultDir = GISApp::Core::SystemConfigManager::instance().getMapDataDir();
    QString defaultPath = QDir(defaultDir).filePath("Google_Sat_Downloaded.tif");

    m_outputPathEdit = new QLineEdit(defaultPath, this);
    QPushButton *browseBtn = new QPushButton("Browse...", this);
    connect(browseBtn, &QPushButton::clicked, this, &DownloadSatImageryDialog::browseOutputFile);

    outLayout->addWidget(m_outputPathEdit, 1);
    outLayout->addWidget(browseBtn);

    mainLayout->addWidget(outGroup);

    // 4. Dialog Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_downloadBtn = new QPushButton("📥 Start Download", this);
    m_downloadBtn->setStyleSheet("background-color: #0284c7; color: white; font-weight: bold; padding: 8px 16px; border-radius: 4px;");
    m_cancelBtn = new QPushButton("Cancel", this);

    connect(m_downloadBtn, &QPushButton::clicked, this, &DownloadSatImageryDialog::startDownload);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addWidget(m_downloadBtn);
    btnLayout->addWidget(m_cancelBtn);

    mainLayout->addLayout(btnLayout);

    // Connect value change signals to update estimate live
    connect(m_northSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
    connect(m_southSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
    connect(m_eastSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
    connect(m_westSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
    connect(m_minZoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
    connect(m_maxZoomSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &DownloadSatImageryDialog::updateTileEstimate);
}

void DownloadSatImageryDialog::populateFromMapExtent()
{
    if (m_mapWidget && m_mapWidget->map()) {
        int w = std::max(10, m_mapWidget->width());
        int h = std::max(10, m_mapWidget->height());

        QMapLibre::Coordinate topLeft = m_mapWidget->map()->coordinateForPixel(QPointF(0, 0));
        QMapLibre::Coordinate bottomRight = m_mapWidget->map()->coordinateForPixel(QPointF(w, h));

        double north = std::max(topLeft.first, bottomRight.first);
        double south = std::min(topLeft.first, bottomRight.first);
        double west = std::min(topLeft.second, bottomRight.second);
        double east = std::max(topLeft.second, bottomRight.second);

        m_northSpin->setValue(north);
        m_southSpin->setValue(south);
        m_westSpin->setValue(west);
        m_eastSpin->setValue(east);

        int currentZoom = static_cast<int>(std::round(m_mapWidget->map()->zoom()));
        m_minZoomSpin->setValue(std::clamp(currentZoom - 2, 0, 18));
        m_maxZoomSpin->setValue(std::clamp(currentZoom + 2, 0, 18));
    }
}

void DownloadSatImageryDialog::updateTileEstimate()
{
    GISApp::Core::Tasks::DownloaderParams params;
    params.maxLat = m_northSpin->value();
    params.minLat = m_southSpin->value();
    params.maxLon = m_eastSpin->value();
    params.minLon = m_westSpin->value();
    params.minZoom = m_minZoomSpin->value();
    params.maxZoom = m_maxZoomSpin->value();

    if (params.minZoom > params.maxZoom) {
        m_minZoomSpin->setValue(params.maxZoom);
        params.minZoom = params.maxZoom;
    }

    int totalTiles = 0;
    double estimatedMb = 0.0;
    GISApp::Core::Tasks::GoogleSatDownloaderTask::calculateTileEstimate(params, totalTiles, estimatedMb);

    m_estimateLabel->setText(QString("Calculated tiles: %1 tiles (~%2 MB)")
                                 .arg(totalTiles)
                                 .arg(estimatedMb, 0, 'f', 1));
}

void DownloadSatImageryDialog::browseOutputFile()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Select Destination GeoTIFF File",
                                                     m_outputPathEdit->text(),
                                                     "GeoTIFF Raster (*.tif *.tiff)");
    if (!filePath.isEmpty()) {
        if (!filePath.endsWith(".tif", Qt::CaseInsensitive) && !filePath.endsWith(".tiff", Qt::CaseInsensitive)) {
            filePath += ".tif";
        }
        m_outputPathEdit->setText(filePath);
    }
}

void DownloadSatImageryDialog::startDownload()
{
    GISApp::Core::Tasks::DownloaderParams params;
    params.maxLat = m_northSpin->value();
    params.minLat = m_southSpin->value();
    params.maxLon = m_eastSpin->value();
    params.minLon = m_westSpin->value();
    params.minZoom = m_minZoomSpin->value();
    params.maxZoom = m_maxZoomSpin->value();
    params.outputPath = m_outputPathEdit->text().trimmed();

    if (params.outputPath.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please specify an output file path.");
        return;
    }

    if (params.minLat >= params.maxLat || params.minLon >= params.maxLon) {
        QMessageBox::warning(this, "Bounding Box Error", "South latitude must be less than North latitude, and West longitude must be less than East longitude.");
        return;
    }

    GISApp::Core::Tasks::GoogleSatDownloaderTask::startDownload(params);

    QMessageBox::information(this, "Download Initiated",
                             QString("Background download started for Google Satellite imagery.\n\n"
                                     "Monitor progress in 'TOOLS -> Background Spatial Tasks Monitor' (Ctrl+B).\n\n"
                                     "Output file: %1").arg(params.outputPath));

    accept();
}

} // namespace GISApp::UI::Download
