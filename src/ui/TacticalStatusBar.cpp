/**
 * @file TacticalStatusBar.cpp
 * @brief Implementation of TacticalStatusBar 3D telemetry display.
 */

#include "ui/TacticalStatusBar.h"

namespace GISApp::UI {

TacticalStatusBar::TacticalStatusBar(QWidget *parent)
    : QStatusBar(parent)
{
    setSizeGripEnabled(false);

    m_infoLabel = new QLabel("Coordinate System: WGS 84 / EPSG:4326 | Scale: 1:15,000,000", this);
    m_infoLabel->setObjectName("InfoLabel");

    m_coordLabel = new QLabel("Lat: --.----° N  Lon: --.----° E  Alt: 0.0 m", this);
    m_coordLabel->setObjectName("CoordLabel");
    m_coordLabel->setMinimumWidth(380);
    m_coordLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    addWidget(m_infoLabel, 1);
    addPermanentWidget(m_coordLabel);
}


void TacticalStatusBar::updateCoordinates(const GISApp::Core::Models::GeoCoordinate &coord)
{
    if (!coord.isValid()) return;

    QString latDir = coord.latitude() >= 0 ? "N" : "S";
    QString lonDir = coord.longitude() >= 0 ? "E" : "W";

    QString coordText = QString("Lat: %1° %2  Lon: %3° %4  Alt: %5 m")
                            .arg(std::abs(coord.latitude()), 0, 'f', 4)
                            .arg(latDir)
                            .arg(std::abs(coord.longitude()), 0, 'f', 4)
                            .arg(lonDir)
                            .arg(coord.altitude(), 0, 'f', 1);

    m_coordLabel->setText(coordText);
}

void TacticalStatusBar::updateScale(double scaleDenominator)
{
    m_infoLabel->setText(QString("Coordinate System: WGS 84 / EPSG:4326 | Scale: 1:%1")
                             .arg(static_cast<long long>(scaleDenominator)));
}

} // namespace GISApp::UI
