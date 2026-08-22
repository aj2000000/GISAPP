/**
 * @file PanTool.h
 * @brief Default Navigation / Panning interactive map tool strategy.
 */

#ifndef PANTOOL_H
#define PANTOOL_H

#include <QObject>
#include "core/interfaces/ITool.h"

namespace GISApp::Tools {

/**
 * @class PanTool
 * @brief Strategy tool handling default map panning and navigation.
 */
class PanTool : public QObject, public GISApp::Core::Interfaces::ITool {
    Q_OBJECT

public:
    explicit PanTool(QObject *parent = nullptr);
    ~PanTool() override = default;

    QString toolName() const override { return "PanTool"; }

    void activate() override {}
    void deactivate() override {}

    void onMousePress(QMouseEvent*, const GISApp::Core::Models::GeoCoordinate&) override {}
    void onMouseMove(QMouseEvent*, const GISApp::Core::Models::GeoCoordinate&) override {}
    void onMouseRelease(QMouseEvent*, const GISApp::Core::Models::GeoCoordinate&) override {}
};

} // namespace GISApp::Tools

#endif // PANTOOL_H
