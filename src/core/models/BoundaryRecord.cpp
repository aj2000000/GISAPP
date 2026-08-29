#include "BoundaryRecord.h"
#include <QJsonArray>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace GISApp::Core::Models {

QJsonObject BoundaryRecord::toGeoJsonFeature() const {
    QJsonObject feature;
    feature["type"] = "Feature";

    QVector<QJsonArray> segments;
    QJsonArray currentSegment;

    for (int i = 0; i < points.size(); ++i) {
        const auto &pt = points[i];
        QJsonArray pointArray;
        pointArray.append(pt.longitude);
        pointArray.append(pt.latitude);
        if (pt.altitude != 0.0) {
            pointArray.append(pt.altitude);
        }

        if (currentSegment.isEmpty()) {
            currentSegment.append(pointArray);
        } else {
            const auto &prevPt = points[i - 1];
            double dLat = (pt.latitude - prevPt.latitude) * 111.0;
            double dLon = (pt.longitude - prevPt.longitude) * 111.0 * std::cos((pt.latitude + prevPt.latitude) * 0.5 * M_PI / 180.0);
            double distKm = std::sqrt(dLat * dLat + dLon * dLon);

            if (distKm > 5.0) { // Discontinuity jump threshold > 5 km
                if (currentSegment.size() >= 2) {
                    segments.append(currentSegment);
                }
                currentSegment = QJsonArray();
                currentSegment.append(pointArray);
            } else {
                currentSegment.append(pointArray);
            }
        }
    }
    if (currentSegment.size() >= 2) {
        segments.append(currentSegment);
    } else if (segments.isEmpty() && !currentSegment.isEmpty()) {
        segments.append(currentSegment);
    }

    QJsonObject geom;
    if (segments.size() <= 1) {
        geom["type"] = "LineString";
        geom["coordinates"] = segments.isEmpty() ? QJsonArray() : segments.first();
    } else {
        geom["type"] = "MultiLineString";
        QJsonArray multiArray;
        for (const auto &seg : segments) {
            multiArray.append(seg);
        }
        geom["coordinates"] = multiArray;
    }

    feature["geometry"] = geom;

    QJsonObject props;
    props["id"] = boundaryId;
    props["name"] = entityName();
    feature["properties"] = props;

    return feature;
}

} // namespace GISApp::Core::Models
