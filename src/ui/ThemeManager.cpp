/**
 * @file ThemeManager.cpp
 * @brief Implementation of ThemeManager stylesheet generation.
 */

#include "ui/ThemeManager.h"
#include <QApplication>

namespace GISApp::UI {

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent), m_currentTheme(ThemeType::TacticalDark)
{
}

QString ThemeManager::themeName(ThemeType theme)
{
    switch (theme) {
        case ThemeType::TacticalDark:     return "Tactical Dark (Default)";
        case ThemeType::CyberEmerald:     return "Cyber Emerald";
        case ThemeType::MidnightBlue:     return "Midnight Blue";
        case ThemeType::HighContrastDark: return "High-Contrast Dark";
        case ThemeType::LightOps:         return "Light Operations";
    }
    return "Tactical Dark";
}

void ThemeManager::applyTheme(ThemeType theme)
{
    m_currentTheme = theme;
    QString qss = getStyleSheet(theme);
    qApp->setStyleSheet(qss);
    emit themeChanged(theme);
}
QString ThemeManager::getStyleSheet(ThemeType theme) const
{
    QString bgPrimary, bgPanel, bgFloating, accent, border, textPrimary, textMuted;

    switch (theme) {
        case ThemeType::CyberEmerald:
            bgPrimary   = "#040b07"; bgPanel   = "#08170e"; bgFloating = "rgba(8, 23, 14, 0.88)";
            accent      = "#00ff88"; border    = "#10331f"; textPrimary= "#e6fffa"; textMuted  = "#62a884";
            break;
        case ThemeType::MidnightBlue:
            bgPrimary   = "#080d14"; bgPanel   = "#0f1724"; bgFloating = "rgba(15, 23, 36, 0.88)";
            accent      = "#38bdf8"; border    = "#1e293b"; textPrimary= "#f0f9ff"; textMuted  = "#8497b0";
            break;
        case ThemeType::HighContrastDark:
            bgPrimary   = "#000000"; bgPanel   = "#121212"; bgFloating = "rgba(18, 18, 18, 0.92)";
            accent      = "#eab308"; border    = "#27272a"; textPrimary= "#ffffff"; textMuted  = "#d4d4d8";
            break;
        case ThemeType::LightOps:
            bgPrimary   = "#f1f5f9"; bgPanel   = "#ffffff"; bgFloating = "rgba(255, 255, 255, 0.95)";
            accent      = "#2563eb"; border    = "#cbd5e1"; textPrimary= "#0f172a"; textMuted  = "#64748b";
            break;
        case ThemeType::TacticalDark:
        default:
            bgPrimary   = "#080a0c"; bgPanel   = "#0f1317"; bgFloating = "rgba(15, 19, 23, 0.88)";
            accent      = "#10b981"; border    = "#1a222a"; textPrimary= "#f3f4f6"; textMuted  = "#9ca3af";
            break;
    }

    return QString(R"(
        QMainWindow {
            background-color: %1;
            color: %6;
        }
        QMenuBar {
            background-color: %2;
            color: %6;
            border-bottom: 1px solid %5;
            font-size: 11px;
            font-weight: bold;
        }
        QMenuBar::item {
            padding: 6px 10px;
            background: transparent;
        }
        QMenuBar::item:selected {
            background: %5;
            color: %4;
        }
        QMenu {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
        }
        QMenu::item:selected {
            background-color: %5;
            color: %4;
        }
        QFrame#HeaderBar {
            background-color: %2;
            border-bottom: 1px solid %5;
        }
        QPushButton#HeaderActionButton {
            background-color: rgba(255, 255, 255, 0.05);
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 4px 10px;
            font-size: 11px;
        }
        QPushButton#HeaderActionButton:hover {
            background-color: %5;
            color: %4;
        }
        QWidget#LeftSidebar {
            background-color: %2;
            border-right: 1px solid %5;
        }
        QFrame#RightToolPanel, QFrame#ZoomControlsWidget {
            background-color: %3;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QFrame#RightToolPanel QToolButton, QFrame#ZoomControlsWidget QToolButton {
            background-color: transparent;
            color: %6;
            border: none;
            border-radius: 4px;
            font-size: 14px;
        }
        QFrame#RightToolPanel QToolButton:hover, QFrame#ZoomControlsWidget QToolButton:hover {
            background-color: rgba(255, 255, 255, 0.15);
            color: %4;
        }
        QStatusBar {
            background-color: %2;
            border-top: 1px solid %5;
        }
        QLabel#InfoLabel {
            color: %7;
            font-size: 11px;
            font-family: monospace;
        }
        QLabel#CoordLabel {
            color: %4;
            font-family: 'Monospace', 'Courier New', monospace;
            font-weight: bold;
            font-size: 12px;
        }
        QToolButton {
            background-color: transparent;
            color: %6;
            border: none;
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: rgba(255, 255, 255, 0.10);
            color: %4;
        }
        QToolButton:checked {
            background-color: rgba(16, 185, 129, 0.15);
            color: %4;
            border: 1px solid %4;
        }
        QToolButton#HomeButton {
            background-color: rgba(16, 185, 129, 0.20);
            color: #10b981;
            border: 1px solid rgba(16, 185, 129, 0.5);
            border-radius: 6px;
        }
        QToolButton#HomeButton:hover {
            background-color: rgba(16, 185, 129, 0.30);
            color: #34d399;
        }

        /* Global Dialog & MessageBox Styling for High-Contrast Text Visibility */
        QDialog, QMessageBox, QInputDialog, QFileDialog {
            background-color: %2;
            color: %6;
            border: 1px solid %5;
            border-radius: 8px;
        }
        QMessageBox QLabel, QDialog QLabel, QInputDialog QLabel, QFileDialog QLabel {
            color: %6;
            font-size: 13px;
            font-weight: 500;
            background: transparent;
        }
        QMessageBox QPushButton, QDialogButtonBox QPushButton, QDialog QPushButton {
            background-color: %5;
            color: %6;
            border: 1px solid rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            padding: 6px 16px;
            font-size: 12px;
            font-weight: bold;
            min-width: 75px;
        }
        QMessageBox QPushButton:hover, QDialogButtonBox QPushButton:hover, QDialog QPushButton:hover {
            background-color: %4;
            color: #ffffff;
            border-color: %4;
        }
        QMessageBox QPushButton:pressed, QDialogButtonBox QPushButton:pressed, QDialog QPushButton:pressed {
            background-color: %4;
            color: #ffffff;
        }
        QMessageBox QPushButton:disabled, QDialogButtonBox QPushButton:disabled, QDialog QPushButton:disabled {
            background-color: rgba(255, 255, 255, 0.05);
            color: %7;
            border-color: %5;
        }
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background-color: %1;
            color: %6;
            border: 1px solid %5;
            border-radius: 4px;
            padding: 5px 8px;
            selection-background-color: %4;
            selection-color: #ffffff;
        }
        QToolTip {
            background-color: %2;
            color: %6;
            border: 1px solid %4;
            padding: 4px 8px;
            border-radius: 4px;
        }
    )").arg(bgPrimary, bgPanel, bgFloating, accent, border, textPrimary, textMuted);
}

} // namespace GISApp::UI