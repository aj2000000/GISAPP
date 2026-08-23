/**
 * @file ThemeManager.h
 * @brief Dynamic Theme Engine managing QSS stylesheets and color palettes.
 *
 * @details Implements the Strategy/Manager pattern for application-wide visual theme switching.
 * Encapsulates style tokens for 5 presets: Tactical Dark, Cyber Emerald, Midnight Blue,
 * High-Contrast Dark, and Light Operations.
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

namespace GISApp::UI {

/**
 * @enum ThemeType
 * @brief Enumeration of available visual theme presets.
 */
enum class ThemeType {
    TacticalDark,    ///< Default Command & Control Dark Theme (Slate/Emerald)
    CyberEmerald,    ///< Matrix / Cybernetic Neon Green Theme
    MidnightBlue,    ///< Naval / Maritime Deep Blue Theme
    HighContrastDark,///< High-Contrast Black/Yellow Theme
    LightOps         ///< Daylight Field Operations Light Theme
};

/**
 * @class ThemeManager
 * @brief Singleton manager providing QSS stylesheets and handling runtime theme updates.
 */
class ThemeManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Gets the singleton instance of ThemeManager.
     * @return Reference to static ThemeManager instance.
     */
    static ThemeManager& instance();

    /**
     * @brief Applies a specific visual theme to the application runtime.
     * @param theme Target ThemeType enum.
     */
    void applyTheme(ThemeType theme);

    /**
     * @brief Gets the human-readable display name of a theme type.
     * @param theme ThemeType enum.
     * @return QString display title.
     */
    static QString themeName(ThemeType theme);

    /**
     * @brief Gets the currently active theme type.
     * @return Active ThemeType.
     */
    ThemeType currentTheme() const { return m_currentTheme; }

    /**
     * @brief Generates QSS stylesheet for a given theme type.
     * @param theme Target ThemeType.
     * @return QString containing complete QSS rules.
     */
    QString getStyleSheet(ThemeType theme) const;

signals:
    /**
     * @brief Emitted when active visual theme changes.
     * @param theme New active ThemeType.
     */
    void themeChanged(ThemeType theme);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() override = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    ThemeType m_currentTheme;
};

} // namespace GISApp::UI

#endif // THEMEMANAGER_H
