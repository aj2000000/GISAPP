/**
 * @file RightToolPanel.h
 * @brief Floating right quick-action toolbar for map interaction tools.
 *
 * @details Implements Single Responsibility Principle (SRP). Provides floating overlay buttons 
 * for Navigation, Search, Filter, Layers, Target Crosshair, and Bookmarks.
 */

#ifndef RIGHTTOOLPANEL_H
#define RIGHTTOOLPANEL_H

#include <QFrame>
#include <QToolButton>
#include <QVBoxLayout>

namespace GISApp::UI {

/**
 * @class RightToolPanel
 * @brief Translucent floating quick-action toolbar anchored to top-right of map viewport.
 */
class RightToolPanel : public QFrame {
    Q_OBJECT

public:
    /**
     * @brief Constructor for RightToolPanel.
     * @param parent Qt parent widget.
     */
    explicit RightToolPanel(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~RightToolPanel() override = default;

signals:
    /**
     * @brief Emitted when a tool button is clicked.
     * @param toolName Identifier of the clicked tool action.
     */
    void toolTriggered(const QString &toolName);

private:
    QVBoxLayout *m_layout;
    QToolButton* createButton(const QString &name, const QString &text);
};

} // namespace GISApp::UI

#endif // RIGHTTOOLPANEL_H
