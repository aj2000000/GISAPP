/**
 * @file PublishLayerDialog.h
 * @brief Interactive Qt Dialog for Publishing Raster and Vector GIS Layers.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef PUBLISHLAYERDIALOG_H
#define PUBLISHLAYERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include "layers/LayerManager.h"
#include "publishing/LayerPublishingService.h"

namespace GISApp::UI::Publishing {

/**
 * @class PublishLayerDialog
 * @brief Modal dialog providing step-by-step layer publishing GUI.
 */
class PublishLayerDialog : public QDialog {
    Q_OBJECT

public:
    PublishLayerDialog(GISApp::Layers::LayerManager *layerManager,
                       QMapLibre::Map *mapInstance,
                       QWidget *parent = nullptr);
    ~PublishLayerDialog() override = default;

private slots:
    void onBrowseFolder();
    void onPublishClicked();
    void updateFilePreview();
    void onCreateGroupClicked();
    void onMaxZoomChanged(int value);

private:
    void setupUI();
    void populateGroups();

    GISApp::Layers::LayerManager *m_layerManager;
    QMapLibre::Map *m_mapInstance;
    GISApp::Publishing::LayerPublishingService m_publishingService;

    QRadioButton *m_radioRasterFile;
    QRadioButton *m_radioRasterFolder;
    QRadioButton *m_radioVector;
    QLineEdit *m_editFolderPath;
    QPushButton *m_btnBrowse;
    QListWidget *m_listFilePreview;
    QLineEdit *m_editLayerName;
    QComboBox *m_comboGroups;
    QPushButton *m_btnNewGroup;

    QSpinBox *m_spinMinZoom;
    QSpinBox *m_spinMaxZoom;
    QCheckBox *m_checkBackground;

    QProgressBar *m_progressBar;
    QLabel *m_lblStatus;
    QPushButton *m_btnPublish;
    QPushButton *m_btnCancel;
};

} // namespace GISApp::UI::Publishing

#endif // PUBLISHLAYERDIALOG_H
