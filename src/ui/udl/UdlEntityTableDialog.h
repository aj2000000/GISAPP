/**
 * @file UdlEntityTableDialog.h
 * @brief Table dialog for inspecting, editing, and deleting UDL entities.
 */

#ifndef UDLENTITYTABLEDIALOG_H
#define UDLENTITYTABLEDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace GISApp::UI::UDL {

class UdlEntityTableDialog : public QDialog {
    Q_OBJECT

public:
    explicit UdlEntityTableDialog(QWidget *parent = nullptr);
    ~UdlEntityTableDialog() override = default;

    void refreshData();

signals:
    void zoomToEntityRequested(double latitude, double longitude);

private:
    void setupUi();
    void populateTable();

    QComboBox *m_layerFilterCombo;
    QTableWidget *m_tableWidget;
    QPushButton *m_btnRefresh;
    QPushButton *m_btnDeleteAll;
};

} // namespace GISApp::UI::UDL

#endif // UDLENTITYTABLEDIALOG_H
