/**
 * @file UdlToolbarWidget.cpp
 * @brief Implementation of UdlToolbarWidget.
 */

#include "src/ui/udl/UdlToolbarWidget.h"
#include "publishing/LayerRegistryManager.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QButtonGroup>
#include <QDebug>

namespace GISApp::UI::UDL {

UdlToolbarWidget::UdlToolbarWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("UdlToolbarWidget");
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("QWidget#UdlToolbarWidget { background-color: #1e1e2e; border: 1px solid #45475a; border-radius: 8px; padding: 4px; }"
                  "QLabel { color: #cdd6f4; font-weight: bold; font-size: 11px; }"
                  "QComboBox { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 6px; padding: 4px 28px 4px 10px; min-width: 150px; font-weight: bold; font-size: 12px; }"
                  "QComboBox:hover, QComboBox:focus { border-color: #89b4fa; }"
                  "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 24px; border-left: none; background: transparent; }"
                  "QComboBox::down-arrow { width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #89b4fa; margin-right: 8px; }"
                  "QComboBox QAbstractItemView { background-color: #181825; color: #f3f4f6; border: 1px solid #89b4fa; border-radius: 6px; padding: 4px; outline: 0px; selection-background-color: #89b4fa; selection-color: #11111b; }"
                  "QComboBox QAbstractItemView::item { min-height: 26px; padding: 4px 10px; color: #f3f4f6; background-color: transparent; }"
                  "QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected { background-color: #89b4fa; color: #11111b; font-weight: bold; border-radius: 4px; }"
                  "QToolButton, QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 5px 10px; font-size: 12px; }"
                  "QToolButton:hover, QPushButton:hover { background-color: #45475a; }"
                  "QToolButton:checked { background-color: #89b4fa; color: #11111b; font-weight: bold; border: 1px solid #b4befe; }"
                  "QCheckBox { color: #cdd6f4; font-size: 11px; font-weight: bold; spacing: 5px; }"
                  "QCheckBox::indicator { width: 14px; height: 14px; border-radius: 3px; border: 1px solid #45475a; background-color: #181825; }"
                  "QCheckBox::indicator:checked { background-color: #a6e3a1; border-color: #a6e3a1; }");

    setupUi();
    refreshLayerList();
}

void UdlToolbarWidget::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(6);

    // --- LINE 1: Entity Types & Layer Selection ---
    auto row1Layout = new QHBoxLayout();
    row1Layout->setContentsMargins(0, 0, 0, 0);
    row1Layout->setSpacing(6);

    auto lblTitle = new QLabel(tr("🎨 UDL Layer:"), this);
    row1Layout->addWidget(lblTitle);

    m_layerCombo = new QComboBox(this);
    connect(m_layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            QString lId = m_layerCombo->itemData(index).toString();
            QString lName = m_layerCombo->currentText();
            emit activeLayerChanged(lId, lName);
        }
    });
    row1Layout->addWidget(m_layerCombo);

    m_btnNewLayer = new QPushButton(tr("➕ New"), this);
    m_btnNewLayer->setToolTip(tr("Create a new User Defined Layer"));
    connect(m_btnNewLayer, &QPushButton::clicked, this, &UdlToolbarWidget::createLayerRequested);
    row1Layout->addWidget(m_btnNewLayer);

    auto line1 = new QFrame(this);
    line1->setFrameShape(QFrame::VLine);
    line1->setStyleSheet("color: #45475a;");
    row1Layout->addWidget(line1);

    auto toolGroup = new QButtonGroup(this);
    toolGroup->setExclusive(true);

    m_btnPoint = new QToolButton(this);
    m_btnPoint->setText(tr("📍 Point"));
    m_btnPoint->setCheckable(true);
    m_btnPoint->setChecked(true);
    toolGroup->addButton(m_btnPoint);
    connect(m_btnPoint, &QToolButton::clicked, this, [this]() {
        setActiveGeometryType(UdlGeometryType::Point);
        emit toolSelected(UdlGeometryType::Point);
    });
    row1Layout->addWidget(m_btnPoint);

    m_btnPolyline = new QToolButton(this);
    m_btnPolyline->setText(tr("📏 Line"));
    m_btnPolyline->setCheckable(true);
    toolGroup->addButton(m_btnPolyline);
    connect(m_btnPolyline, &QToolButton::clicked, this, [this]() {
        setActiveGeometryType(UdlGeometryType::Polyline);
        emit toolSelected(UdlGeometryType::Polyline);
    });
    row1Layout->addWidget(m_btnPolyline);

    m_btnPolygon = new QToolButton(this);
    m_btnPolygon->setText(tr("🔷 Polygon"));
    m_btnPolygon->setCheckable(true);
    toolGroup->addButton(m_btnPolygon);
    connect(m_btnPolygon, &QToolButton::clicked, this, [this]() {
        setActiveGeometryType(UdlGeometryType::Polygon);
        emit toolSelected(UdlGeometryType::Polygon);
    });
    row1Layout->addWidget(m_btnPolygon);

    m_btnCircle = new QToolButton(this);
    m_btnCircle->setText(tr("⭕ Circle"));
    m_btnCircle->setCheckable(true);
    toolGroup->addButton(m_btnCircle);
    connect(m_btnCircle, &QToolButton::clicked, this, [this]() {
        setActiveGeometryType(UdlGeometryType::Circle);
        emit toolSelected(UdlGeometryType::Circle);
    });
    row1Layout->addWidget(m_btnCircle);

    m_btnText = new QToolButton(this);
    m_btnText->setText(tr("🔤 Text"));
    m_btnText->setCheckable(true);
    toolGroup->addButton(m_btnText);
    connect(m_btnText, &QToolButton::clicked, this, [this]() {
        setActiveGeometryType(UdlGeometryType::Text);
        emit toolSelected(UdlGeometryType::Text);
    });
    row1Layout->addWidget(m_btnText);

    auto line1b = new QFrame(this);
    line1b->setFrameShape(QFrame::VLine);
    line1b->setStyleSheet("color: #45475a;");
    row1Layout->addWidget(line1b);

    m_btnUndo = new QToolButton(this);
    m_btnUndo->setText(tr("↩️ Undo"));
    m_btnUndo->setToolTip(tr("Undo last point while drawing or undo last entity action"));
    connect(m_btnUndo, &QToolButton::clicked, this, &UdlToolbarWidget::undoRequested);
    row1Layout->addWidget(m_btnUndo);

    m_btnManageEntities = new QPushButton(tr("📊 Table"), this);
    m_btnManageEntities->setToolTip(tr("Open UDL Entities Table"));
    connect(m_btnManageEntities, &QPushButton::clicked, this, &UdlToolbarWidget::manageEntitiesRequested);
    row1Layout->addWidget(m_btnManageEntities);

    mainLayout->addLayout(row1Layout);

    // --- LINE 2: Entity Properties & Options ---
    auto row2Layout = new QHBoxLayout();
    row2Layout->setContentsMargins(0, 0, 0, 0);
    row2Layout->setSpacing(6);

    auto lblPropTitle = new QLabel(tr("⚙️ Properties:"), this);
    row2Layout->addWidget(lblPropTitle);

    m_txtLabelInput = new QLineEdit(this);
    m_txtLabelInput->setPlaceholderText(tr("Enter text label..."));
    m_txtLabelInput->setToolTip(tr("Type label text here before clicking on the map"));
    m_txtLabelInput->setVisible(false);
    m_txtLabelInput->setStyleSheet("QLineEdit { background-color: #181825; color: #cdd6f4; border: 1px solid #89b4fa; border-radius: 4px; padding: 4px 8px; font-weight: bold; min-width: 150px; }");
    connect(m_txtLabelInput, &QLineEdit::textChanged, this, &UdlToolbarWidget::pendingTextLabelChanged);
    row2Layout->addWidget(m_txtLabelInput);

    m_btnStrokeColor = new QToolButton(this);
    connect(m_btnStrokeColor, &QToolButton::clicked, this, [this]() {
        QString title = tr("Select Color");
        if (m_activeType == UdlGeometryType::Point) title = tr("Select Point Color");
        else if (m_activeType == UdlGeometryType::Polyline) title = tr("Select Line Color");
        else if (m_activeType == UdlGeometryType::Polygon) title = tr("Select Border Color");
        else if (m_activeType == UdlGeometryType::Circle) title = tr("Select Circle Border Color");
        else if (m_activeType == UdlGeometryType::Text) title = tr("Select Text Color");

        QColor c = QColorDialog::getColor(m_strokeColor, this, title);
        if (c.isValid()) {
            m_strokeColor = c;
            updateColorButtons();
            emit colorsChanged(m_strokeColor, m_fillColor);
        }
    });
    row2Layout->addWidget(m_btnStrokeColor);

    m_btnFillColor = new QToolButton(this);
    connect(m_btnFillColor, &QToolButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(m_fillColor, this, tr("Select Fill Color"));
        if (c.isValid()) {
            m_fillColor = c;
            updateColorButtons();
            emit colorsChanged(m_strokeColor, m_fillColor);
        }
    });
    row2Layout->addWidget(m_btnFillColor);

    auto line2 = new QFrame(this);
    line2->setFrameShape(QFrame::VLine);
    line2->setStyleSheet("color: #45475a;");
    row2Layout->addWidget(line2);

    m_chkContinuous = new QCheckBox(tr("🔁 Continuous"), this);
    m_chkContinuous->setToolTip(tr("Keep drawing tool active after creating each shape"));
    m_chkContinuous->setChecked(true);
    connect(m_chkContinuous, &QCheckBox::toggled, this, &UdlToolbarWidget::continuousModeChanged);
    row2Layout->addWidget(m_chkContinuous);

    m_chkQuickName = new QCheckBox(tr("⚡ Quick Name"), this);
    m_chkQuickName->setToolTip(tr("Auto-name entities without popping up the style dialog every time"));
    m_chkQuickName->setChecked(true);
    connect(m_chkQuickName, &QCheckBox::toggled, this, &UdlToolbarWidget::quickNameModeChanged);
    row2Layout->addWidget(m_chkQuickName);

    row2Layout->addStretch();

    mainLayout->addLayout(row2Layout);

    setActiveGeometryType(UdlGeometryType::Point);
}

void UdlToolbarWidget::setActiveGeometryType(UdlGeometryType type) {
    m_activeType = type;
    if (m_btnPoint) m_btnPoint->setChecked(type == UdlGeometryType::Point);
    if (m_btnPolyline) m_btnPolyline->setChecked(type == UdlGeometryType::Polyline);
    if (m_btnPolygon) m_btnPolygon->setChecked(type == UdlGeometryType::Polygon);
    if (m_btnCircle) m_btnCircle->setChecked(type == UdlGeometryType::Circle);
    if (m_btnText) m_btnText->setChecked(type == UdlGeometryType::Text);

    if (m_txtLabelInput) {
        m_txtLabelInput->setVisible(type == UdlGeometryType::Text);
        if (type == UdlGeometryType::Text && m_txtLabelInput->isVisible()) {
            m_txtLabelInput->setFocus();
        }
    }

    updateColorButtons();
}

QString UdlToolbarWidget::pendingTextLabel() const {
    return m_txtLabelInput ? m_txtLabelInput->text() : QString();
}

void UdlToolbarWidget::updateColorButtons() {
    if (!m_btnStrokeColor || !m_btnFillColor) return;

    switch (m_activeType) {
    case UdlGeometryType::Point:
        m_btnStrokeColor->setText(tr("🎨 Point Color"));
        m_btnStrokeColor->setToolTip(tr("Select Point Color"));
        m_btnStrokeColor->setVisible(true);
        m_btnFillColor->setVisible(false);
        break;
    case UdlGeometryType::Polyline:
        m_btnStrokeColor->setText(tr("🎨 Line Color"));
        m_btnStrokeColor->setToolTip(tr("Select Line Color"));
        m_btnStrokeColor->setVisible(true);
        m_btnFillColor->setVisible(false);
        break;
    case UdlGeometryType::Polygon:
        m_btnStrokeColor->setText(tr("🎨 Border Color"));
        m_btnStrokeColor->setToolTip(tr("Select Border Color"));
        m_btnStrokeColor->setVisible(true);
        m_btnFillColor->setText(tr("🪣 Fill Color"));
        m_btnFillColor->setToolTip(tr("Select Fill Color"));
        m_btnFillColor->setVisible(true);
        break;
    case UdlGeometryType::Circle:
        m_btnStrokeColor->setText(tr("🎨 Border Color"));
        m_btnStrokeColor->setToolTip(tr("Select Circle Border Color"));
        m_btnStrokeColor->setVisible(true);
        m_btnFillColor->setText(tr("🪣 Fill Color"));
        m_btnFillColor->setToolTip(tr("Select Fill Color"));
        m_btnFillColor->setVisible(true);
        break;
    case UdlGeometryType::Text:
        m_btnStrokeColor->setText(tr("🎨 Text Color"));
        m_btnStrokeColor->setToolTip(tr("Select Text Color"));
        m_btnStrokeColor->setVisible(true);
        m_btnFillColor->setVisible(false);
        break;
    }

    m_btnStrokeColor->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border: 1px solid #45475a; border-radius: 4px; padding: 4px 8px;")
                                        .arg(m_strokeColor.name())
                                        .arg(m_strokeColor.lightness() < 128 ? "#ffffff" : "#000000"));

    if (m_btnFillColor->isVisible()) {
        m_btnFillColor->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border: 1px solid #45475a; border-radius: 4px; padding: 4px 8px;")
                                          .arg(m_fillColor.name())
                                          .arg(m_fillColor.lightness() < 128 ? "#ffffff" : "#000000"));
    }
}

void UdlToolbarWidget::refreshLayerList() {
    if (!m_layerCombo) return;
    QString currentSelectedId = activeLayerId();

    m_layerCombo->blockSignals(true);
    m_layerCombo->clear();

    auto savedLayers = GISApp::Publishing::LayerRegistryManager::instance().getSavedLayers();
    int matchIdx = -1;

    for (const auto &meta : savedLayers) {
        if (meta.groupName.contains("User Defined Layers") || meta.groupName.contains("UDL") || meta.groupName.contains("🎨")) {
            QString lId = meta.layerId.isEmpty() ? meta.name : meta.layerId;
            m_layerCombo->addItem(QString("📍 %1").arg(meta.name), lId);
            if (lId == currentSelectedId) {
                matchIdx = m_layerCombo->count() - 1;
            }
        }
    }

    if (m_layerCombo->count() == 0) {
        m_layerCombo->addItem(tr("(No UDL Layers)"), "");
    } else if (matchIdx >= 0) {
        m_layerCombo->setCurrentIndex(matchIdx);
    } else {
        m_layerCombo->setCurrentIndex(0);
    }

    m_layerCombo->blockSignals(false);

    if (m_layerCombo->count() > 0 && !activeLayerId().isEmpty()) {
        emit activeLayerChanged(activeLayerId(), activeLayerName());
    }
}

QString UdlToolbarWidget::activeLayerId() const {
    return m_layerCombo ? m_layerCombo->currentData().toString() : QString();
}

QString UdlToolbarWidget::activeLayerName() const {
    if (!m_layerCombo) return QString();
    QString text = m_layerCombo->currentText();
    if (text.startsWith("📍 ")) text = text.mid(3);
    return text;
}

void UdlToolbarWidget::setActiveLayer(const QString &layerId) {
    if (!m_layerCombo) return;
    for (int i = 0; i < m_layerCombo->count(); ++i) {
        if (m_layerCombo->itemData(i).toString() == layerId) {
            m_layerCombo->setCurrentIndex(i);
            break;
        }
    }
}

bool UdlToolbarWidget::isContinuousMode() const {
    return m_chkContinuous ? m_chkContinuous->isChecked() : true;
}

bool UdlToolbarWidget::isQuickNameMode() const {
    return m_chkQuickName ? m_chkQuickName->isChecked() : true;
}

} // namespace GISApp::UI::UDL
