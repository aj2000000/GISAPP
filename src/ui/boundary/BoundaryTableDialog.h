#ifndef BOUNDARYTABLEDIALOG_H
#define BOUNDARYTABLEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVector>

#include "../../core/models/BoundaryRecord.h"
#include "../../core/services/MapLibreBoundaryAdapter.h"
#include "../../controllers/MapController.h"

namespace GISApp::UI::Boundary {

class BoundaryTableDialog : public QDialog {
    Q_OBJECT
public:
    explicit BoundaryTableDialog(
        const QVector<Core::Models::BoundaryRecord> &boundaries,
        Controllers::MapController *mapController = nullptr,
        Core::Services::MapLibreBoundaryAdapter *adapter = nullptr,
        QWidget *parent = nullptr
    );
    ~BoundaryTableDialog() override = default;

public slots:
    void setBoundaries(const QVector<Core::Models::BoundaryRecord> &boundaries);
    void refreshData();

signals:
    void boundaryCleared();

private slots:
    void onZoomToSelected();
    void onClearAll();

private:
    QVector<Core::Models::BoundaryRecord> m_boundaries;
    Controllers::MapController *m_mapController{nullptr};
    Core::Services::MapLibreBoundaryAdapter *m_adapter{nullptr};

    QTableWidget *m_tableWidget{nullptr};
    QLabel *m_statusLabel{nullptr};
    QLineEdit *m_searchLineEdit{nullptr};

    void setupUi();
};

} // namespace GISApp::UI::Boundary

#endif // BOUNDARYTABLEDIALOG_H
