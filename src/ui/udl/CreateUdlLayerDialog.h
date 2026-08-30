/**
 * @file CreateUdlLayerDialog.h
 * @brief Modal dialog to input parameters for creating a new User Defined Layer.
 */

#ifndef CREATEUDLLAYERDIALOG_H
#define CREATEUDLLAYERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>

namespace GISApp::UI::UDL {

class CreateUdlLayerDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreateUdlLayerDialog(QWidget *parent = nullptr);
    ~CreateUdlLayerDialog() override = default;

    QString layerName() const;

private:
    void setupUi();

    QLineEdit *m_nameEdit;
    QPushButton *m_btnCreate;
    QPushButton *m_btnCancel;
};

} // namespace GISApp::UI::UDL

#endif // CREATEUDLLAYERDIALOG_H
