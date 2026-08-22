/**
 * @file ITool.h
 * @brief Abstract Strategy Interface for interactive GIS Map Tools.
 *
 * @details Implements the Strategy Pattern. Decouples interactive map actions
 * (Pan, Distance Measurement, Drawing) from the core Map Canvas.
 */

#ifndef ITOOL_H
#define ITOOL_H

#include <QString>
#include <QMouseEvent>
#include "core/models/GeoCoordinate.h"

namespace GISApp::Core::Interfaces {

/**
 * @class ITool
 * @brief Abstract interface defining lifecycle and mouse events for interactive GIS tools.
 */
class ITool {
public:
    virtual ~ITool() = default;

    /**
     * @brief Returns unique identifier/name of the tool.
     */
    virtual QString toolName() const = 0;

    /**
     * @brief Called when tool is activated.
     */
    virtual void activate() = 0;

    /**
     * @brief Called when tool is deactivated.
     */
    virtual void deactivate() = 0;

    /**
     * @brief Event handler for mouse press events on the map canvas.
     */
    virtual void onMousePress(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) = 0;

    /**
     * @brief Event handler for mouse move events on the map canvas.
     */
    virtual void onMouseMove(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) = 0;

    /**
     * @brief Event handler for mouse release events on the map canvas.
     */
    virtual void onMouseRelease(QMouseEvent *event, const GISApp::Core::Models::GeoCoordinate &coordinate) = 0;
};

} // namespace GISApp::Core::Interfaces

#endif // ITOOL_H
