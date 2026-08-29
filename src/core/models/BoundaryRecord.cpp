#include "BoundaryRecord.h"
#include <QJsonArray>

namespace GISApp::Core::Models {

QJsonObject BoundaryRecord::toGeoJsonFeature() const {
    QJsonObject feature;
    feature["type"] = "Feature";

    QJsonObject geom;
    geom["type"] = "LineString";

    QJsonArray coordsArray;
    for (const auto &pt : points) {
        QJsonArray pointArray;
        pointArray.append(pt.longitude);
        pointArray.append(pt.latitude);
        if (pt.altitude != 0.0) {
            pointArray.append(pt.altitude);
        }
        coordsArray.append(pointArray);
    }
    geom["coordinates"] = coordsArray;
    feature["geometry"] = geom;

    QJsonObject props;
    props["id"] = boundaryId;
    props["name"] = entityName();
    feature["properties"] = props;

    return feature;
}

} // namespace GISApp::Core::Models
