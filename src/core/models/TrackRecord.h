#ifndef TRACKRECORD_H
#define TRACKRECORD_H

#include "IGisEntity.h"
#include <QJsonObject>
#include <memory>

namespace GISApp::Core::Models {

class TrackRecord : public IGisEntity {
public:
    TrackRecord() = default;
    ~TrackRecord() override = default;

    // IGisEntity Interface Implementation
    QString entityId() const override { return QString::number(trackId); }
    QString entityName() const override { return trackName; }
    EntityCategory category() const override { return EntityCategory::Track; }

    std::shared_ptr<IGisGeometry> geometry() const override;
    EntityRenderStyle renderStyle() const override;
    QJsonObject toGeoJsonFeature() const override;

    // 19 Standard Tactical Domain Schema Fields
    int trackId{0};
    QString trackName;
    double trackLat{0.0};
    double trackLong{0.0};
    double trackHeight{0.0};
    double trackDir{0.0};
    int trackIdentity{0};
    int trackType{0};
    int trackSubType{0};
    int trackClass{0};
    int trackStrength{0};
    int trackActType{0};
    int trackActSubType{0};
    int trackActClass{0};
    int trackSystemType{0};
    QString trackSources;
    QString trackImage;
    QString trackRemarks;
    QString trackReportTime;
};

} // namespace GISApp::Core::Models

#endif // TRACKRECORD_H
