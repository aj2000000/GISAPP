/**
 * @file LeftSidebar.h
 * @brief Encapsulated vertical navigation sidebar for Command & Control interface.
 *
 * @details Implements Single Responsibility Principle (SRP). Manages top section navigation 
 * buttons (Home, Settings, Layers, Analytics, Tools) and bottom utility buttons 
 * (User Profile, Search, Help).
 */

#ifndef LEFTSIDEBAR_H
#define LEFTSIDEBAR_H

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QButtonGroup>

namespace GISApp::UI {

/**
 * @class LeftSidebar
 * @brief Custom navigation sidebar widget positioned on the left window margin.
 */
class LeftSidebar : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief Constructor for LeftSidebar.
     * @param parent Qt parent widget.
     */
    explicit LeftSidebar(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~LeftSidebar() override = default;

signals:
    /**
     * @brief Emitted when a sidebar navigation button is clicked.
     * @param actionName Name identifier of the clicked action.
     */
    void actionTriggered(const QString &actionName);

private:
    QVBoxLayout *m_layout;
    QVBoxLayout *m_topLayout;
    QVBoxLayout *m_bottomLayout;
    QButtonGroup *m_buttonGroup;

    void setupButtons();
    QToolButton* createButton(const QString &name, const QString &text, bool isCheckable = true);
};

} // namespace GISApp::UI

#endif // LEFTSIDEBAR_H
