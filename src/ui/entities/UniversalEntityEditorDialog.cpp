/**
 * @file UniversalEntityEditorDialog.cpp
 * @brief Implementation of UniversalEntityEditorDialog.
 */

#include "UniversalEntityEditorDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>
#include <QDebug>

namespace GISApp::UI::Entities {

using namespace GISApp::Core::Models;

UniversalEntityEditorDialog::UniversalEntityEditorDialog(
    std::shared_ptr<GenericGisEntity> entity,
    QWidget *parent)
    : QDialog(parent)
    , m_entity(entity)
{
    setWindowTitle(m_entity ? QString("Edit Entity — %1").arg(m_entity->entityName()) : "Entity Editor");
    setMinimumWidth(450);
    setModal(true);

    if (m_entity) {
        m_strokeColor = m_entity->renderStyle().strokeColor;
        m_fillColor = m_entity->renderStyle().fillColor;
    }

    buildDynamicForm();
    applyDarkTheme();
}

void UniversalEntityEditorDialog::buildDynamicForm()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(14);

    // 1. General Info Group
    QGroupBox *generalGroup = new QGroupBox("General Attributes", this);
    QFormLayout *generalLayout = new QFormLayout(generalGroup);

    m_txtName = new QLineEdit(m_entity ? m_entity->entityName() : "", this);
    generalLayout->addRow("Entity Name:", m_txtName);

    if (m_entity) {
        QLabel *lblType = new QLabel(m_entity->entityType().toUpper(), this);
        generalLayout->addRow("Entity Type:", lblType);
    }
    mainLayout->addWidget(generalGroup);

    // 2. Visual Style Group
    QGroupBox *styleGroup = new QGroupBox("Visual Styling", this);
    QFormLayout *styleLayout = new QFormLayout(styleGroup);

    m_btnStrokeColor = new QPushButton("Pick Color", this);
    m_btnStrokeColor->setStyleSheet(QString("background-color: %1; color: #ffffff; border-radius: 4px;").arg(m_strokeColor.name()));
    connect(m_btnStrokeColor, &QPushButton::clicked, this, &UniversalEntityEditorDialog::onSelectStrokeColor);
    styleLayout->addRow("Stroke Color:", m_btnStrokeColor);

    m_btnFillColor = new QPushButton("Pick Color", this);
    m_btnFillColor->setStyleSheet(QString("background-color: %1; color: #ffffff; border-radius: 4px;").arg(m_fillColor.name()));
    connect(m_btnFillColor, &QPushButton::clicked, this, &UniversalEntityEditorDialog::onSelectFillColor);
    styleLayout->addRow("Fill Color:", m_btnFillColor);

    m_spinStrokeWidth = new QDoubleSpinBox(this);
    m_spinStrokeWidth->setRange(1.0, 20.0);
    m_spinStrokeWidth->setValue(m_entity ? m_entity->renderStyle().strokeWidth : 2.0);
    styleLayout->addRow("Stroke Width (px):", m_spinStrokeWidth);

    mainLayout->addWidget(styleGroup);

    // 3. Dynamic Custom Properties Form
    if (m_entity) {
        auto desc = GisEntityRegistry::instance().descriptor(m_entity->entityType());
        if (!desc.propertySchemas.isEmpty()) {
            QGroupBox *propsGroup = new QGroupBox("Custom Attributes", this);
            QFormLayout *propsLayout = new QFormLayout(propsGroup);

            for (const auto &schema : desc.propertySchemas) {
                QVariant val = m_entity->property(schema.key, schema.defaultValue);

                if (!schema.enumOptions.isEmpty()) {
                    QComboBox *cmb = new QComboBox(this);
                    cmb->addItems(schema.enumOptions);
                    cmb->setCurrentText(val.toString());
                    propsLayout->addRow(schema.label + ":", cmb);
                    m_propertyWidgets.insert(schema.key, cmb);
                } else if (schema.type == QMetaType::Double || schema.type == QMetaType::Int) {
                    QDoubleSpinBox *spin = new QDoubleSpinBox(this);
                    spin->setRange(-999999.0, 999999.0);
                    spin->setValue(val.toDouble());
                    propsLayout->addRow(schema.label + ":", spin);
                    m_propertyWidgets.insert(schema.key, spin);
                } else {
                    QLineEdit *txt = new QLineEdit(val.toString(), this);
                    propsLayout->addRow(schema.label + ":", txt);
                    m_propertyWidgets.insert(schema.key, txt);
                }
            }
            mainLayout->addWidget(propsGroup);
        }
    }

    // 4. Action Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    QPushButton *btnCancel = new QPushButton("Cancel", this);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *btnSave = new QPushButton("Save Changes", this);
    btnSave->setDefault(true);
    connect(btnSave, &QPushButton::clicked, this, &UniversalEntityEditorDialog::onSaveClicked);

    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    mainLayout->addLayout(btnLayout);
}

void UniversalEntityEditorDialog::onSelectStrokeColor()
{
    QColor c = QColorDialog::getColor(m_strokeColor, this, "Select Stroke Color");
    if (c.isValid()) {
        m_strokeColor = c;
        m_btnStrokeColor->setStyleSheet(QString("background-color: %1; color: #ffffff; border-radius: 4px;").arg(m_strokeColor.name()));
    }
}

void UniversalEntityEditorDialog::onSelectFillColor()
{
    QColor c = QColorDialog::getColor(m_fillColor, this, "Select Fill Color");
    if (c.isValid()) {
        m_fillColor = c;
        m_btnFillColor->setStyleSheet(QString("background-color: %1; color: #ffffff; border-radius: 4px;").arg(m_fillColor.name()));
    }
}

void UniversalEntityEditorDialog::onSaveClicked()
{
    if (!m_entity) {
        reject();
        return;
    }

    m_entity->setEntityName(m_txtName->text().trimmed());

    EntityRenderStyle style = m_entity->renderStyle();
    style.strokeColor = m_strokeColor;
    style.fillColor = m_fillColor;
    style.strokeWidth = m_spinStrokeWidth->value();
    m_entity->setRenderStyle(style);

    // Save dynamic property values back into entity
    for (auto it = m_propertyWidgets.constBegin(); it != m_propertyWidgets.constEnd(); ++it) {
        QString key = it.key();
        QWidget *w = it.value();

        if (auto cmb = qobject_cast<QComboBox*>(w)) {
            m_entity->setProperty(key, cmb->currentText());
        } else if (auto spin = qobject_cast<QDoubleSpinBox*>(w)) {
            m_entity->setProperty(key, spin->value());
        } else if (auto txt = qobject_cast<QLineEdit*>(w)) {
            m_entity->setProperty(key, txt->text());
        }
    }

    accept();
}

void UniversalEntityEditorDialog::applyDarkTheme()
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
            font-weight: 500;
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
        QComboBox {
            background-color: #1e293b;
            color: #ffffff;
            border: 1px solid #334155;
            border-radius: 6px;
            padding: 4px 28px 4px 10px;
            font-weight: 600;
        }
        QComboBox:hover, QComboBox:focus {
            border-color: #38bdf8;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 24px;
            border-left: none;
            background: transparent;
        }
        QComboBox::down-arrow {
            width: 0;
            height: 0;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 6px solid #38bdf8;
            margin-right: 8px;
        }
        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #ffffff;
            border: 1px solid #38bdf8;
            border-radius: 6px;
            padding: 4px;
            outline: 0px;
            selection-background-color: #0284c7;
            selection-color: #ffffff;
        }
        QComboBox QAbstractItemView::item {
            min-height: 26px;
            padding: 4px 10px;
            color: #ffffff;
            background-color: transparent;
        }
        QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected {
            background-color: #0284c7;
            color: #ffffff;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #1e293b;
            color: #38bdf8;
            border: 1px solid #0284c7;
            border-radius: 4px;
            padding: 6px 16px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0284c7;
            color: #ffffff;
        }
    )");
}

} // namespace GISApp::UI::Entities
