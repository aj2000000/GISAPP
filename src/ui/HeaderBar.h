/**
 * @file HeaderBar.h
 * @brief Command & Control sub-header bar displaying system status and global operations.
 */

#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

namespace GISApp::UI {

/**
 * @class HeaderBar
 * @brief Tactical sub-header bar below the menu bar.
 */
class HeaderBar : public QFrame {
    Q_OBJECT

public:
    explicit HeaderBar(QWidget *parent = nullptr);
    ~HeaderBar() override = default;

private:
    void setupUi();
    QPushButton* createActionButton(const QString &text, const QString &tooltip);
};

} // namespace GISApp::UI

#endif // HEADERBAR_H
