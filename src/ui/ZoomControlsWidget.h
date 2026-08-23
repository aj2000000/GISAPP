/**
 * @file ZoomControlsWidget.h
 * @brief Floating map zoom and bearing orientation controls widget.
 *
 * @details Implements Single Responsibility Principle (SRP). Exposes signals for Zoom In,
 * Zoom Out, and Reset Bearing/Center.
 */

#ifndef ZOOMCONTROLSWIDGET_H
#define ZOOMCONTROLSWIDGET_H

#include <QFrame>
#include <QToolButton>
#include <QVBoxLayout>

namespace GISApp::UI {

/**
 * @class ZoomControlsWidget
 * @brief Translucent floating zoom control stack anchored to bottom-left of map viewport.
 */
class ZoomControlsWidget : public QFrame {
    Q_OBJECT

public:
    /**
     * @brief Constructor for ZoomControlsWidget.
     * @param parent Qt parent widget.
     */
    explicit ZoomControlsWidget(QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~ZoomControlsWidget() override = default;

signals:
    /**
     * @brief Emitted when Zoom In (+) is clicked.
     */
    void zoomInRequested();

    /**
     * @brief Emitted when Zoom Out (-) is clicked.
     */
    void zoomOutRequested();

    /**
     * @brief Emitted when Reset Bearing/Center is clicked.
     */
    void resetCenterRequested();
};

} // namespace GISApp::UI

#endif // ZOOMCONTROLSWIDGET_H
