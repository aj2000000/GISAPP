/**
 * @file UdlToolbarWidget.h
 * @brief Floating/Dockable digitizing toolbar for User Defined Layers.
 */

#ifndef UDLTOOLBARWIDGET_H
#define UDLTOOLBARWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QToolButton>
#include <QCheckBox>
#include <QPushButton>
#include <QColor>
#include "src/ui/udl/UdlEntityStyleDialog.h"

namespace GISApp::UI::UDL {

class UdlToolbarWidget : public QWidget {
    Q_OBJECT

public:
    explicit UdlToolbarWidget(QWidget *parent = nullptr);
    ~UdlToolbarWidget() override = default;

    void refreshLayerList();
    QString activeLayerId() const;
    QString activeLayerName() const;
    void setActiveLayer(const QString &layerId);

    bool isContinuousMode() const;
    bool isQuickNameMode() const;

    void setActiveGeometryType(UdlGeometryType type);
    UdlGeometryType activeGeometryType() const { return m_activeType; }

    QColor currentStrokeColor() const { return m_strokeColor; }
    QColor currentFillColor() const { return m_fillColor; }
    QString pendingTextLabel() const;

signals:
    void activeLayerChanged(const QString &layerId, const QString &layerName);
    void toolSelected(UdlGeometryType type);
    void toolDeactivated();
    void createLayerRequested();
    void manageEntitiesRequested();
    void undoRequested();
    void pendingTextLabelChanged(const QString &text);
    void colorsChanged(const QColor &strokeColor, const QColor &fillColor);
    void continuousModeChanged(bool enabled);
    void quickNameModeChanged(bool enabled);

private:
    void setupUi();
    void updateColorButtons();

    QComboBox *m_layerCombo{nullptr};
    QPushButton *m_btnNewLayer{nullptr};
    QToolButton *m_btnPoint{nullptr};
    QToolButton *m_btnPolyline{nullptr};
    QToolButton *m_btnPolygon{nullptr};
    QToolButton *m_btnCircle{nullptr};
    QToolButton *m_btnText{nullptr};
    QToolButton *m_btnImage{nullptr};
    QLineEdit *m_txtLabelInput{nullptr};
    QToolButton *m_btnUndo{nullptr};
    QToolButton *m_btnStrokeColor{nullptr};
    QToolButton *m_btnFillColor{nullptr};
    QCheckBox *m_chkContinuous{nullptr};
    QCheckBox *m_chkQuickName{nullptr};
    QPushButton *m_btnManageEntities{nullptr};

    UdlGeometryType m_activeType{UdlGeometryType::Point};
    QColor m_strokeColor{QColor("#f59e0b")};
    QColor m_fillColor{QColor("#ff9933")};
};

} // namespace GISApp::UI::UDL

#endif // UDLTOOLBARWIDGET_H
