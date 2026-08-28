/**
 * @file DownloadSatImageryDialog.h
 * @brief Modal UI Dialog to configure area bounding box, zoom level range, and output path for Google Satellite imagery downloads.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef DOWNLOADSATIMAGERYDIALOG_H
#define DOWNLOADSATIMAGERYDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include "map/MapLibreWidget.h"
#include "core/tasks/GoogleSatDownloaderTask.h"

namespace GISApp::UI::Download {

class DownloadSatImageryDialog : public QDialog {
    Q_OBJECT

public:
    explicit DownloadSatImageryDialog(GISApp::Map::MapLibreWidget *mapWidget, QWidget *parent = nullptr);
    ~DownloadSatImageryDialog() override = default;

private slots:
    void populateFromMapExtent();
    void updateTileEstimate();
    void browseOutputFile();
    void startDownload();

private:
    void setupUi();

    GISApp::Map::MapLibreWidget *m_mapWidget;

    QDoubleSpinBox *m_northSpin;
    QDoubleSpinBox *m_southSpin;
    QDoubleSpinBox *m_eastSpin;
    QDoubleSpinBox *m_westSpin;

    QSpinBox *m_minZoomSpin;
    QSpinBox *m_maxZoomSpin;

    QLabel *m_estimateLabel;
    QLineEdit *m_outputPathEdit;

    QPushButton *m_downloadBtn;
    QPushButton *m_cancelBtn;
};

} // namespace GISApp::UI::Download

#endif // DOWNLOADSATIMAGERYDIALOG_H
