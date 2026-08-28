/**
 * @file AreaOfViewTableDialog.h
 * @brief Dark-themed Qt Dialog for viewing and managing Area of View Polygon Entities.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef AREAOFVIEWTABLEDIALOG_H
#define AREAOFVIEWTABLEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "../../core/repositories/IAreaOfViewRepository.h"
#include "../../core/services/MapLibreAreaOfViewAdapter.h"
#include "../../controllers/MapController.h"

namespace GISApp::UI::AreaOfView {

class AreaOfViewTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit AreaOfViewTableDialog(
        Core::Repositories::IAreaOfViewRepository *repository,
        Controllers::MapController *mapController = nullptr,
        Core::Services::MapLibreAreaOfViewAdapter *adapter = nullptr,
        QWidget *parent = nullptr
    );
    ~AreaOfViewTableDialog() override = default;

public slots:
    void refreshData();

private slots:
    void onZoomToSelected();
    void onDeleteSelected();
    void onClearAll();

private:
    Core::Repositories::IAreaOfViewRepository *m_repository{nullptr};
    Controllers::MapController *m_mapController{nullptr};
    Core::Services::MapLibreAreaOfViewAdapter *m_adapter{nullptr};

    QTableWidget *m_tableWidget{nullptr};
    QLabel *m_statusLabel{nullptr};
    QLineEdit *m_searchLineEdit{nullptr};

    void setupUi();
};

} // namespace GISApp::UI::AreaOfView

#endif // AREAOFVIEWTABLEDIALOG_H
