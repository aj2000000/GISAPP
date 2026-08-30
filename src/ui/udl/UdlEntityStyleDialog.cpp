/**
 * @file UdlEntityStyleDialog.cpp
 * @brief Implementation of UdlEntityStyleDialog with full entity property support.
 */

#include "src/ui/udl/UdlEntityStyleDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>

namespace GISApp::UI::UDL {

UdlEntityStyleDialog::UdlEntityStyleDialog(UdlGeometryType type, QWidget *parent)
    : QDialog(parent), m_type(type)
{
    setWindowTitle(type == UdlGeometryType::Text ? tr("Add Text Label & Font Properties") : tr("Configure UDL Entity Style"));
    setMinimumWidth(400);
    setStyleSheet("QDialog { background-color: #1e1e2e; color: #cdd6f4; }"
                  "QLabel { color: #cdd6f4; font-size: 12px; }"
                  "QLineEdit { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 4px; padding: 4px 8px; }"
                  "QSpinBox, QDoubleSpinBox { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 6px; padding: 4px 24px 4px 10px; font-weight: bold; font-size: 12px; }"
                  "QSpinBox:hover, QSpinBox:focus, QDoubleSpinBox:hover, QDoubleSpinBox:focus { border-color: #89b4fa; }"
                  "QSpinBox::up-button, QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 20px; border-left: 1px solid #45475a; border-bottom: 1px solid #45475a; background-color: #11111b; border-top-right-radius: 5px; }"
                  "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover { background-color: #89b4fa; }"
                  "QSpinBox::up-arrow, QDoubleSpinBox::up-arrow { width: 0; height: 0; border-left: 4px solid transparent; border-right: 4px solid transparent; border-bottom: 5px solid #89b4fa; }"
                  "QSpinBox::up-button:hover QSpinBox::up-arrow, QDoubleSpinBox::up-button:hover QDoubleSpinBox::up-arrow { border-bottom-color: #11111b; }"
                  "QSpinBox::down-button, QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 20px; border-left: 1px solid #45475a; background-color: #11111b; border-bottom-right-radius: 5px; }"
                  "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover { background-color: #89b4fa; }"
                  "QSpinBox::down-arrow, QDoubleSpinBox::down-arrow { width: 0; height: 0; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 5px solid #89b4fa; }"
                  "QSpinBox::down-button:hover QSpinBox::down-arrow, QDoubleSpinBox::down-button:hover QDoubleSpinBox::down-arrow { border-top-color: #11111b; }"
                  "QComboBox { background-color: #181825; color: #f3f4f6; border: 1px solid #45475a; border-radius: 6px; padding: 4px 28px 4px 10px; font-weight: bold; font-size: 12px; }"
                  "QComboBox:hover, QComboBox:focus { border-color: #89b4fa; }"
                  "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 24px; border-left: none; background: transparent; }"
                  "QComboBox::down-arrow { width: 0; height: 0; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid #89b4fa; margin-right: 8px; }"
                  "QComboBox QAbstractItemView { background-color: #181825; color: #f3f4f6; border: 1px solid #89b4fa; border-radius: 6px; padding: 4px; outline: 0px; selection-background-color: #89b4fa; selection-color: #11111b; }"
                  "QComboBox QAbstractItemView::item { min-height: 24px; padding: 4px 10px; color: #f3f4f6; background-color: transparent; }"
                  "QComboBox QAbstractItemView::item:hover, QComboBox QAbstractItemView::item:selected { background-color: #89b4fa; color: #11111b; font-weight: bold; border-radius: 4px; }"
                  "QPushButton { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; border-radius: 4px; padding: 6px 12px; }"
                  "QPushButton:hover { background-color: #45475a; }");

    setupUi();
}

void UdlEntityStyleDialog::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    auto formLayout = new QFormLayout();

    m_txtNameEdit = new QLineEdit(this);
    m_txtNameEdit->setPlaceholderText(tr("Enter Entity Name..."));
    formLayout->addRow(tr("Entity Name:"), m_txtNameEdit);

    if (m_type == UdlGeometryType::Text) {
        // --- Text Content & Font Properties ---
        m_txtContentEdit = new QLineEdit(this);
        m_txtContentEdit->setPlaceholderText(tr("Enter Text Label..."));
        formLayout->addRow(tr("Text Label:"), m_txtContentEdit);

        m_comboFontFamily = new QComboBox(this);
        m_comboFontFamily->addItems({"sans-serif", "Arial", "Roboto", "Courier New", "Georgia", "Times New Roman", "Trebuchet MS", "Verdana"});
        m_comboFontFamily->setCurrentText("sans-serif");
        formLayout->addRow(tr("Font Family:"), m_comboFontFamily);

        m_spinFontSize = new QSpinBox(this);
        m_spinFontSize->setRange(8, 72);
        m_spinFontSize->setValue(14);
        formLayout->addRow(tr("Font Size (pt):"), m_spinFontSize);

        // --- Text Color & Opacity ---
        m_btnTextColor = new QPushButton(this);
        updateColorButton(m_btnTextColor, m_textColor);
        connect(m_btnTextColor, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(m_textColor, this, tr("Select Text Color"), QColorDialog::ShowAlphaChannel);
            if (c.isValid()) setTextColor(c);
        });
        formLayout->addRow(tr("Text Color:"), m_btnTextColor);

        m_spinTextOpacity = new QDoubleSpinBox(this);
        m_spinTextOpacity->setRange(0.0, 1.0);
        m_spinTextOpacity->setSingleStep(0.05);
        m_spinTextOpacity->setValue(1.0);
        formLayout->addRow(tr("Text Opacity:"), m_spinTextOpacity);

        // --- Background Color & Opacity ---
        m_btnBgColor = new QPushButton(this);
        updateColorButton(m_btnBgColor, m_bgColor);
        connect(m_btnBgColor, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(m_bgColor, this, tr("Select Background Color"), QColorDialog::ShowAlphaChannel);
            if (c.isValid()) setBgColor(c);
        });
        formLayout->addRow(tr("Background Color:"), m_btnBgColor);

        m_spinBgOpacity = new QDoubleSpinBox(this);
        m_spinBgOpacity->setRange(0.0, 1.0);
        m_spinBgOpacity->setSingleStep(0.05);
        m_spinBgOpacity->setValue(0.85);
        formLayout->addRow(tr("Background Opacity:"), m_spinBgOpacity);

        // --- Border Color, Opacity & Width ---
        m_btnBorderColor = new QPushButton(this);
        updateColorButton(m_btnBorderColor, m_borderColor);
        connect(m_btnBorderColor, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(m_borderColor, this, tr("Select Border Color"), QColorDialog::ShowAlphaChannel);
            if (c.isValid()) setBorderColor(c);
        });
        formLayout->addRow(tr("Border Color:"), m_btnBorderColor);

        m_spinBorderOpacity = new QDoubleSpinBox(this);
        m_spinBorderOpacity->setRange(0.0, 1.0);
        m_spinBorderOpacity->setSingleStep(0.05);
        m_spinBorderOpacity->setValue(1.0);
        formLayout->addRow(tr("Border Opacity:"), m_spinBorderOpacity);

        m_spinBorderWidth = new QSpinBox(this);
        m_spinBorderWidth->setRange(0, 10);
        m_spinBorderWidth->setValue(1);
        formLayout->addRow(tr("Border Width (px):"), m_spinBorderWidth);

    } else {
        // --- Non-Text Shapes (Point, Polyline, Polygon, Circle) ---
        m_btnStrokeColor = new QPushButton(this);
        updateColorButton(m_btnStrokeColor, m_strokeColor);

        QString colorLabel = tr("Stroke Color:");
        if (m_type == UdlGeometryType::Point) colorLabel = tr("Point Color:");
        else if (m_type == UdlGeometryType::Polyline) colorLabel = tr("Line Color:");
        else if (m_type == UdlGeometryType::Polygon) colorLabel = tr("Border Color:");
        else if (m_type == UdlGeometryType::Circle) colorLabel = tr("Circle Border Color:");

        connect(m_btnStrokeColor, &QPushButton::clicked, this, [this, colorLabel]() {
            QColor c = QColorDialog::getColor(m_strokeColor, this, tr("Select %1").arg(colorLabel), QColorDialog::ShowAlphaChannel);
            if (c.isValid()) setStrokeColor(c);
        });
        formLayout->addRow(colorLabel, m_btnStrokeColor);

        m_spinStrokeOpacity = new QDoubleSpinBox(this);
        m_spinStrokeOpacity->setRange(0.0, 1.0);
        m_spinStrokeOpacity->setSingleStep(0.05);
        m_spinStrokeOpacity->setValue(1.0);
        formLayout->addRow(tr("Stroke Opacity:"), m_spinStrokeOpacity);

        if (m_type == UdlGeometryType::Point) {
            m_spinPointRadius = new QSpinBox(this);
            m_spinPointRadius->setRange(2, 50);
            m_spinPointRadius->setValue(8);
            formLayout->addRow(tr("Point Radius (px):"), m_spinPointRadius);
        }

        if (m_type == UdlGeometryType::Polyline || m_type == UdlGeometryType::Polygon || m_type == UdlGeometryType::Circle) {
            m_spinLineWidth = new QSpinBox(this);
            m_spinLineWidth->setRange(1, 30);
            m_spinLineWidth->setValue(3);
            formLayout->addRow(tr("Border / Line Width (px):"), m_spinLineWidth);
        }

        if (m_type == UdlGeometryType::Polygon || m_type == UdlGeometryType::Circle) {
            m_btnFillColor = new QPushButton(this);
            updateColorButton(m_btnFillColor, m_fillColor);
            connect(m_btnFillColor, &QPushButton::clicked, this, [this]() {
                QColor c = QColorDialog::getColor(m_fillColor, this, tr("Select Fill Color"), QColorDialog::ShowAlphaChannel);
                if (c.isValid()) setFillColor(c);
            });
            formLayout->addRow(tr("Fill Color:"), m_btnFillColor);

            m_spinFillOpacity = new QDoubleSpinBox(this);
            m_spinFillOpacity->setRange(0.0, 1.0);
            m_spinFillOpacity->setSingleStep(0.05);
            m_spinFillOpacity->setValue(0.35);
            formLayout->addRow(tr("Fill Opacity:"), m_spinFillOpacity);
        }
    }

    mainLayout->addLayout(formLayout);

    auto btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);
}

void UdlEntityStyleDialog::updateColorButton(QPushButton *btn, const QColor &color) {
    if (!btn) return;
    btn->setText(color.name());
    btn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border: 1px solid #45475a; border-radius: 4px; padding: 6px;")
                           .arg(color.name())
                           .arg(color.lightness() < 128 ? "#ffffff" : "#000000"));
}

void UdlEntityStyleDialog::setEntityName(const QString &name) {
    if (m_txtNameEdit) m_txtNameEdit->setText(name);
}

QString UdlEntityStyleDialog::entityName() const {
    return m_txtNameEdit ? m_txtNameEdit->text().trimmed() : QString();
}

void UdlEntityStyleDialog::setStrokeColor(const QColor &color) {
    m_strokeColor = color;
    updateColorButton(m_btnStrokeColor, m_strokeColor);
}

QColor UdlEntityStyleDialog::strokeColor() const {
    return m_strokeColor;
}

void UdlEntityStyleDialog::setStrokeOpacity(double opacity) {
    if (m_spinStrokeOpacity) m_spinStrokeOpacity->setValue(opacity);
}

double UdlEntityStyleDialog::strokeOpacity() const {
    return m_spinStrokeOpacity ? m_spinStrokeOpacity->value() : 1.0;
}

void UdlEntityStyleDialog::setFillColor(const QColor &color) {
    m_fillColor = color;
    updateColorButton(m_btnFillColor, m_fillColor);
}

QColor UdlEntityStyleDialog::fillColor() const {
    return m_fillColor;
}

void UdlEntityStyleDialog::setFillOpacity(double opacity) {
    if (m_spinFillOpacity) m_spinFillOpacity->setValue(opacity);
}

double UdlEntityStyleDialog::fillOpacity() const {
    return m_spinFillOpacity ? m_spinFillOpacity->value() : 0.35;
}

void UdlEntityStyleDialog::setLineWidth(int width) {
    if (m_spinLineWidth) m_spinLineWidth->setValue(width);
}

int UdlEntityStyleDialog::lineWidth() const {
    return m_spinLineWidth ? m_spinLineWidth->value() : 3;
}

void UdlEntityStyleDialog::setPointRadius(int radius) {
    if (m_spinPointRadius) m_spinPointRadius->setValue(radius);
}

int UdlEntityStyleDialog::pointRadius() const {
    return m_spinPointRadius ? m_spinPointRadius->value() : 8;
}

void UdlEntityStyleDialog::setTextContent(const QString &text) {
    if (m_txtContentEdit) m_txtContentEdit->setText(text);
}

QString UdlEntityStyleDialog::textContent() const {
    return m_txtContentEdit ? m_txtContentEdit->text().trimmed() : QString();
}

void UdlEntityStyleDialog::setTextColor(const QColor &color) {
    m_textColor = color;
    updateColorButton(m_btnTextColor, m_textColor);
}

QColor UdlEntityStyleDialog::textColor() const {
    return m_textColor;
}

void UdlEntityStyleDialog::setTextOpacity(double opacity) {
    if (m_spinTextOpacity) m_spinTextOpacity->setValue(opacity);
}

double UdlEntityStyleDialog::textOpacity() const {
    return m_spinTextOpacity ? m_spinTextOpacity->value() : 1.0;
}

void UdlEntityStyleDialog::setBgColor(const QColor &color) {
    m_bgColor = color;
    updateColorButton(m_btnBgColor, m_bgColor);
}

QColor UdlEntityStyleDialog::bgColor() const {
    return m_bgColor;
}

void UdlEntityStyleDialog::setBgOpacity(double opacity) {
    if (m_spinBgOpacity) m_spinBgOpacity->setValue(opacity);
}

double UdlEntityStyleDialog::bgOpacity() const {
    return m_spinBgOpacity ? m_spinBgOpacity->value() : 0.85;
}

void UdlEntityStyleDialog::setBorderColor(const QColor &color) {
    m_borderColor = color;
    updateColorButton(m_btnBorderColor, m_borderColor);
}

QColor UdlEntityStyleDialog::borderColor() const {
    return m_borderColor;
}

void UdlEntityStyleDialog::setBorderOpacity(double opacity) {
    if (m_spinBorderOpacity) m_spinBorderOpacity->setValue(opacity);
}

double UdlEntityStyleDialog::borderOpacity() const {
    return m_spinBorderOpacity ? m_spinBorderOpacity->value() : 1.0;
}

void UdlEntityStyleDialog::setBorderWidth(int width) {
    if (m_spinBorderWidth) m_spinBorderWidth->setValue(width);
}

int UdlEntityStyleDialog::borderWidth() const {
    return m_spinBorderWidth ? m_spinBorderWidth->value() : 1;
}

void UdlEntityStyleDialog::setFontFamily(const QString &family) {
    if (m_comboFontFamily) m_comboFontFamily->setCurrentText(family);
}

QString UdlEntityStyleDialog::fontFamily() const {
    return m_comboFontFamily ? m_comboFontFamily->currentText() : QString("sans-serif");
}

void UdlEntityStyleDialog::setFontSize(int size) {
    if (m_spinFontSize) m_spinFontSize->setValue(size);
}

int UdlEntityStyleDialog::fontSize() const {
    return m_spinFontSize ? m_spinFontSize->value() : 14;
}

QJsonObject UdlEntityStyleDialog::styleJsonObject() const {
    QJsonObject obj;
    obj["strokeColor"] = strokeColor().name();
    obj["strokeOpacity"] = strokeOpacity();

    if (m_type == UdlGeometryType::Point) {
        obj["pointRadius"] = pointRadius();
        obj["fillColor"] = fillColor().name();
        obj["fillOpacity"] = fillOpacity();
    }
    if (m_type == UdlGeometryType::Polyline || m_type == UdlGeometryType::Polygon || m_type == UdlGeometryType::Circle) {
        obj["lineWidth"] = lineWidth();
    }
    if (m_type == UdlGeometryType::Polygon || m_type == UdlGeometryType::Circle) {
        obj["fillColor"] = fillColor().name();
        obj["fillOpacity"] = fillOpacity();
    }
    if (m_type == UdlGeometryType::Text) {
        obj["textContent"] = textContent();
        obj["textColor"] = textColor().name();
        obj["textOpacity"] = textOpacity();
        obj["bgColor"] = bgColor().name();
        obj["bgOpacity"] = bgOpacity();
        obj["borderColor"] = borderColor().name();
        obj["borderOpacity"] = borderOpacity();
        obj["borderWidth"] = borderWidth();
        obj["fontFamily"] = fontFamily();
        obj["fontSize"] = fontSize();
    }
    return obj;
}

void UdlEntityStyleDialog::setStyleJsonObject(const QJsonObject &style) {
    if (style.contains("strokeColor")) setStrokeColor(QColor(style["strokeColor"].toString()));
    if (style.contains("strokeOpacity")) setStrokeOpacity(style["strokeOpacity"].toDouble());
    if (style.contains("pointRadius")) setPointRadius(style["pointRadius"].toInt());
    if (style.contains("lineWidth")) setLineWidth(style["lineWidth"].toInt());
    if (style.contains("fillColor")) setFillColor(QColor(style["fillColor"].toString()));
    if (style.contains("fillOpacity")) setFillOpacity(style["fillOpacity"].toDouble());
    if (style.contains("textContent")) setTextContent(style["textContent"].toString());
    if (style.contains("textColor")) setTextColor(QColor(style["textColor"].toString()));
    if (style.contains("textOpacity")) setTextOpacity(style["textOpacity"].toDouble());
    if (style.contains("bgColor")) setBgColor(QColor(style["bgColor"].toString()));
    if (style.contains("bgOpacity")) setBgOpacity(style["bgOpacity"].toDouble());
    if (style.contains("borderColor")) setBorderColor(QColor(style["borderColor"].toString()));
    if (style.contains("borderOpacity")) setBorderOpacity(style["borderOpacity"].toDouble());
    if (style.contains("borderWidth")) setBorderWidth(style["borderWidth"].toInt());
    if (style.contains("fontFamily")) setFontFamily(style["fontFamily"].toString());
    if (style.contains("fontSize")) setFontSize(style["fontSize"].toInt());
}

} // namespace GISApp::UI::UDL
