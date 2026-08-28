#include "AreaOfViewRecord.h"

namespace GISApp::Core::Models {

std::shared_ptr<IGisGeometry> AreaOfViewRecord::geometry() const
{
    return std::make_shared<PolygonGeometry>(points);
}

EntityRenderStyle AreaOfViewRecord::renderStyle() const
{
    EntityRenderStyle style;
    style.renderType = SymbolRenderType::CustomPaintDelegate;
    style.labelText = name;
    style.strokeColor = QColor(139, 69, 19);       // Brown border (#8B4513)
    style.fillColor = QColor(0, 0, 0, 0);          // No fill color
    style.strokeWidth = 3.5;
    return style;
}

QJsonObject AreaOfViewRecord::toGeoJsonFeature() const
{
    QJsonObject feature;
    feature["type"] = "Feature";

    // Polygon Geometry
    if (auto geom = geometry()) {
        feature["geometry"] = geom->toGeoJsonGeometry();
    }

    // Properties
    QJsonObject props;
    props["id"] = id;
    props["name"] = name;
    props["n_points"] = nPoints;
    props["layerType"] = "area_of_view";

    feature["properties"] = props;
    return feature;
}

} // namespace GISApp::Core::Models
