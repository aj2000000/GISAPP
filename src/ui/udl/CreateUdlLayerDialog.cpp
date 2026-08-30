/**
 * @file CreateUdlLayerDialog.cpp
 * @brief Implementation of CreateUdlLayerDialog.
 */

#include "src/ui/udl/CreateUdlLayerDialog.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace GISApp::UI::UDL {

CreateUdlLayerDialog::CreateUdlLayerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Create User Defined Layer"));
    setFixedSize(360, 140);
    setStyleSheet("QDialog { background-color: #1e1e2e; color: #cdd6f4; }"
                  "QLabel { color: #cdd6f4; font-size: 12px; font-weight: bold; }"
                  "QLineEdit { background-color: #181825; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 6px; font-size: 12px; }"
                  "QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 6px 14px; font-weight: bold; }"
                  "QPushButton:hover { background-color: #45475a; }"
                  "QPushButton#btnCreate { background-color: #89b4fa; color: #11111b; border: 1px solid #b4befe; }");

    setupUi();
}

void CreateUdlLayerDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);

    auto formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("e.g., Tactical_Zone_Alpha"));
    formLayout->addRow(tr("Layer Name:"), m_nameEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    auto btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    m_btnCancel = new QPushButton(tr("Cancel"), this);
    connect(m_btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(m_btnCancel);

    m_btnCreate = new QPushButton(tr("Create Layer"), this);
    m_btnCreate->setObjectName("btnCreate");
    connect(m_btnCreate, &QPushButton::clicked, this, [this]() {
        if (m_nameEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Invalid Name"), tr("Please enter a valid name for the layer."));
            return;
        }
        accept();
    });
    btnLayout->addWidget(m_btnCreate);

    mainLayout->addLayout(btnLayout);
}

QString CreateUdlLayerDialog::layerName() const {
    return m_nameEdit ? m_nameEdit->text().trimmed() : QString();
}

} // namespace GISApp::UI::UDL
