/**
 * @file UdlEntityStyleDialog.h
 * @brief Modal dialog for styling and naming UDL entities (Point, Polyline, Polygon, Text).
 */

#ifndef UDLENTITYSTYLEDIALOG_H
#define UDLENTITYSTYLEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QColor>
#include <QJsonObject>

namespace GISApp::UI::UDL {

enum class UdlGeometryType {
    Point,
    Polyline,
    Polygon,
    Circle,
    Text,
    Image
};

class UdlEntityStyleDialog : public QDialog {
    Q_OBJECT

public:
    explicit UdlEntityStyleDialog(UdlGeometryType type, QWidget *parent = nullptr);
    ~UdlEntityStyleDialog() override = default;

    void setEntityName(const QString &name);
    QString entityName() const;

    void setStrokeColor(const QColor &color);
    QColor strokeColor() const;

    void setStrokeOpacity(double opacity);
    double strokeOpacity() const;

    void setFillColor(const QColor &color);
    QColor fillColor() const;

    void setFillOpacity(double opacity);
    double fillOpacity() const;

    void setLineWidth(int width);
    int lineWidth() const;

    void setPointRadius(int radius);
    int pointRadius() const;

    // Text specific properties
    void setTextContent(const QString &text);
    QString textContent() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setTextOpacity(double opacity);
    double textOpacity() const;

    void setBgColor(const QColor &color);
    QColor bgColor() const;

    void setBgOpacity(double opacity);
    double bgOpacity() const;

    void setBorderColor(const QColor &color);
    QColor borderColor() const;

    void setBorderOpacity(double opacity);
    double borderOpacity() const;

    void setBorderWidth(int width);
    int borderWidth() const;

    void setFontFamily(const QString &family);
    QString fontFamily() const;

    void setFontSize(int size);
    int fontSize() const;

    // Image specific properties
    void setImagePath(const QString &path);
    QString imagePath() const;

    void setImageWidth(int width);
    int imageWidth() const;

    void setImageHeight(int height);
    int imageHeight() const;

    void setImageOpacity(double opacity);
    double imageOpacity() const;

    QJsonObject styleJsonObject() const;
    void setStyleJsonObject(const QJsonObject &style);

private:
    void setupUi();
    void updateColorButton(QPushButton *btn, const QColor &color);

    UdlGeometryType m_type;
    QLineEdit *m_txtNameEdit{nullptr};
    QPushButton *m_btnStrokeColor{nullptr};
    QDoubleSpinBox *m_spinStrokeOpacity{nullptr};
    QPushButton *m_btnFillColor{nullptr};
    QDoubleSpinBox *m_spinFillOpacity{nullptr};
    QSpinBox *m_spinLineWidth{nullptr};
    QSpinBox *m_spinPointRadius{nullptr};

    // Text controls
    QLineEdit *m_txtContentEdit{nullptr};
    QPushButton *m_btnTextColor{nullptr};
    QDoubleSpinBox *m_spinTextOpacity{nullptr};
    QPushButton *m_btnBgColor{nullptr};
    QDoubleSpinBox *m_spinBgOpacity{nullptr};
    QPushButton *m_btnBorderColor{nullptr};
    QDoubleSpinBox *m_spinBorderOpacity{nullptr};
    QSpinBox *m_spinBorderWidth{nullptr};
    QComboBox *m_comboFontFamily{nullptr};
    QSpinBox *m_spinFontSize{nullptr};

    // Image controls
    QLineEdit *m_txtImagePath{nullptr};
    QPushButton *m_btnBrowseImage{nullptr};
    QSpinBox *m_spinImageWidth{nullptr};
    QSpinBox *m_spinImageHeight{nullptr};
    QDoubleSpinBox *m_spinImageOpacity{nullptr};

    QColor m_strokeColor{QColor("#f59e0b")};   // Amber
    QColor m_fillColor{QColor("#ff9933")};     // Saffron
    QColor m_textColor{QColor("#ffffff")};     // White
    QColor m_bgColor{QColor("#0f172a")};       // Slate Dark
    QColor m_borderColor{QColor("#f59e0b")};   // Amber border
};

} // namespace GISApp::UI::UDL

#endif // UDLENTITYSTYLEDIALOG_H
