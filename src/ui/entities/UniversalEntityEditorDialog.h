/**
 * @file UniversalEntityEditorDialog.h
 * @brief Data-driven dialog for editing properties and styling of any spatial GIS entity.
 * Eliminates custom UI code duplication when adding or modifying entity types.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef UNIVERSALENTITYEDITORDIALOG_H
#define UNIVERSALENTITYEDITORDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QMap>
#include <memory>
#include "GenericGisEntity.h"
#include "GisEntityRegistry.h"

namespace GISApp::UI::Entities {

using GISApp::Core::Models::GenericGisEntity;

/**
 * @class UniversalEntityEditorDialog
 * @brief Dynamic property inspector and editor for GenericGisEntity instances.
 */
class UniversalEntityEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit UniversalEntityEditorDialog(
        std::shared_ptr<GenericGisEntity> entity,
        QWidget *parent = nullptr
    );
    virtual ~UniversalEntityEditorDialog() override = default;

    std::shared_ptr<GenericGisEntity> editedEntity() const { return m_entity; }

private slots:
    void onSaveClicked();
    void onSelectStrokeColor();
    void onSelectFillColor();

private:
    void buildDynamicForm();
    void applyDarkTheme();

    std::shared_ptr<GenericGisEntity> m_entity;

    QLineEdit *m_txtName{nullptr};
    QColor m_strokeColor;
    QColor m_fillColor;
    QPushButton *m_btnStrokeColor{nullptr};
    QPushButton *m_btnFillColor{nullptr};
    QDoubleSpinBox *m_spinStrokeWidth{nullptr};

    QMap<QString, QWidget*> m_propertyWidgets;
};

} // namespace GISApp::UI::Entities

#endif // UNIVERSALENTITYEDITORDIALOG_H
