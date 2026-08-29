#include "TrackRecord.h"

namespace GISApp::Core::Models {

std::shared_ptr<IGisGeometry> TrackRecord::geometry() const
{
    return std::make_shared<PointGeometry>(trackLat, trackLong, trackHeight);
}

EntityRenderStyle TrackRecord::renderStyle() const
{
    EntityRenderStyle style;
    style.renderType = SymbolRenderType::DefaultCircle;
    style.rotationHeading = trackDir;
    style.labelText = trackName;
    style.iconPath = trackImage;
    style.strokeColor = Qt::cyan;
    style.fillColor = QColor(0, 255, 255, 180);
    style.strokeWidth = 2.0;
    return style;
}


QJsonObject TrackRecord::toGeoJsonFeature() const
{
    QJsonObject feature;
    feature["type"] = "Feature";

    // Geometry
    if (auto geom = geometry()) {
        feature["geometry"] = geom->toGeoJsonGeometry();
    }

    // Properties
    QJsonObject props;
    props["id"] = trackId;
    props["name"] = trackName;
    props["lat"] = trackLat;
    props["long"] = trackLong;
    props["height"] = trackHeight;
    props["heading"] = trackDir;
    props["identity"] = trackIdentity;
    QString colorStr = "#F59E0B";
    if (trackIdentity == 1) colorStr = "#EF4444";      // Hostile -> Red
    else if (trackIdentity == 2) colorStr = "#3B82F6"; // Friendly -> Blue
    else if (trackIdentity == 3) colorStr = "#10B981"; // Neutral -> Green
    props["color"] = colorStr;
    props["type"] = trackType;
    props["subType"] = trackSubType;
    props["class"] = trackClass;
    props["strength"] = trackStrength;
    props["actType"] = trackActType;
    props["actSubType"] = trackActSubType;
    props["actClass"] = trackActClass;
    props["systemType"] = trackSystemType;
    props["sources"] = trackSources;
    props["image"] = trackImage;
    props["remarks"] = trackRemarks;
    props["reportTime"] = trackReportTime;

    feature["properties"] = props;
    return feature;
}


} // namespace GISApp::Core::Models
