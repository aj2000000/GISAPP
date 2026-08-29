/**
 * @file TrackDetailDialog.cpp
 * @brief Implementation of tactical track detail and technical telemetry dialog.
 */

#include "TrackDetailDialog.h"
#include "fieldkeyvaluemapper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QFrame>

namespace GISApp::UI::Tracks {

TrackDetailDialog::TrackDetailDialog(const GISApp::Core::Models::TrackRecord &track, QWidget *parent)
    : QDialog(parent)
    , m_track(track)
{
    setWindowTitle(QString("Track Details — #%1 (%2)")
                       .arg(m_track.trackId)
                       .arg(m_track.trackName.isEmpty() ? "Unnamed Target" : m_track.trackName));
    setMinimumWidth(500);
    resize(520, 640);
    setupStyle();
    setupUi();
}

void TrackDetailDialog::setupStyle()
{
    setStyleSheet(
        "QDialog { background-color: #0F172A; color: #F8FAFC; font-family: 'Segoe UI', Inter, sans-serif; }"
        "QGroupBox { font-weight: bold; font-size: 13px; color: #38BDF8; border: 1px solid #334155; border-radius: 8px; margin-top: 12px; padding-top: 16px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; background-color: #0F172A; }"
        "QLabel { font-size: 12px; color: #94A3B8; }"
        "QLabel#ValLabel { color: #F8FAFC; font-weight: 600; font-size: 12px; }"
        "QPushButton#ActionBtn { background-color: #2563EB; color: white; border: none; border-radius: 6px; padding: 8px 16px; font-weight: bold; font-size: 12px; }"
        "QPushButton#ActionBtn:hover { background-color: #1D4ED8; }"
        "QPushButton#CloseBtn { background-color: #334155; color: #F8FAFC; border: none; border-radius: 6px; padding: 8px 16px; font-weight: 500; font-size: 12px; }"
        "QPushButton#CloseBtn:hover { background-color: #475569; }"
    );
}

void TrackDetailDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Header Card
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet("QFrame { background-color: #1E293B; border-radius: 8px; border: 1px solid #334155; padding: 10px; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);

    QLabel *iconLabel = new QLabel("🎯", this);
    iconLabel->setStyleSheet("font-size: 28px; padding-right: 8px;");
    headerLayout->addWidget(iconLabel);

    QVBoxLayout *headerTextLayout = new QVBoxLayout();
    QString name = m_track.trackName.isEmpty() ? QString("Track #%1").arg(m_track.trackId) : m_track.trackName;
    QLabel *titleLabel = new QLabel(name, this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #F8FAFC;");

    QString identityStr = FieldKeyValueMapper::instance().trackIdentityMapping(m_track.trackIdentity);
    QString systemTypeStr = FieldKeyValueMapper::instance().systemTrackTypeMapping(m_track.trackSystemType);

    QLabel *subTitleLabel = new QLabel(QString("Track ID: %1 | Identity: %2 | System Type: %3")
                                           .arg(m_track.trackId)
                                           .arg(identityStr)
                                           .arg(systemTypeStr), this);
    subTitleLabel->setStyleSheet("font-size: 11px; color: #38BDF8;");

    headerTextLayout->addWidget(titleLabel);
    headerTextLayout->addWidget(subTitleLabel);
    headerLayout->addLayout(headerTextLayout);
    headerLayout->addStretch();

    mainLayout->addWidget(headerFrame);

    // Group 1: Geographic Position & Kinematics
    QGroupBox *geoGroup = new QGroupBox("Geographic Position & Kinematics", this);
    QFormLayout *geoLayout = new QFormLayout(geoGroup);
    geoLayout->setSpacing(6);

    auto formatDms = [](double val, bool isLat) {
        char dir = isLat ? (val >= 0 ? 'N' : 'S') : (val >= 0 ? 'E' : 'W');
        val = std::abs(val);
        int deg = static_cast<int>(val);
        double minVal = (val - deg) * 60.0;
        int min = static_cast<int>(minVal);
        double sec = (minVal - min) * 60.0;
        return QString("%1° %2' %3\" %4").arg(deg).arg(min, 2, 10, QChar('0')).arg(sec, 5, 'f', 2, QChar('0')).arg(dir);
    };

    auto addRow = [](QFormLayout *layout, const QString &label, const QString &valStr) {
        QLabel *lbl = new QLabel(label);
        QLabel *val = new QLabel(valStr);
        val->setObjectName("ValLabel");
        layout->addRow(lbl, val);
    };

    addRow(geoLayout, "Latitude:", QString("%1° (%2)").arg(m_track.trackLat, 0, 'f', 6).arg(formatDms(m_track.trackLat, true)));
    addRow(geoLayout, "Longitude:", QString("%1° (%2)").arg(m_track.trackLong, 0, 'f', 6).arg(formatDms(m_track.trackLong, false)));
    addRow(geoLayout, "Altitude / Height:", QString("%1 m (%2 ft)").arg(m_track.trackHeight, 0, 'f', 1).arg(m_track.trackHeight * 3.28084, 0, 'f', 0));
    addRow(geoLayout, "Heading / Bearing:", QString("%1°").arg(m_track.trackDir, 0, 'f', 1));

    mainLayout->addWidget(geoGroup);

    // Group 2: Tactical Attributes & Classification
    QGroupBox *attrGroup = new QGroupBox("Tactical Attributes & Classification", this);
    QFormLayout *attrLayout = new QFormLayout(attrGroup);
    attrLayout->setSpacing(6);

    addRow(attrLayout, "Type / SubType:", QString("Type: %1 | SubType: %2").arg(m_track.trackType).arg(m_track.trackSubType));
    addRow(attrLayout, "Class / Strength:", QString("Class: %1 | Force Count: %2").arg(m_track.trackClass).arg(m_track.trackStrength));
    addRow(attrLayout, "Activity Type / SubType:", QString("ActType: %1 | SubType: %2").arg(m_track.trackActType).arg(m_track.trackActSubType));
    addRow(attrLayout, "Activity Classification:", QString::number(m_track.trackActClass));
    addRow(attrLayout, "Sensor / System Type:", FieldKeyValueMapper::instance().systemTrackTypeMapping(m_track.trackSystemType));

    mainLayout->addWidget(attrGroup);

    // Group 3: Sources & Remarks
    QGroupBox *metaGroup = new QGroupBox("Telemetry Metadata", this);
    QFormLayout *metaLayout = new QFormLayout(metaGroup);
    metaLayout->setSpacing(6);

    addRow(metaLayout, "Report Date & Time:", m_track.trackReportTime.isEmpty() ? "N/A" : m_track.trackReportTime);
    addRow(metaLayout, "Data Sources:", m_track.trackSources.isEmpty() ? "N/A" : m_track.trackSources);
    addRow(metaLayout, "Symbol Icon Key:", m_track.trackImage.isEmpty() ? "Default" : m_track.trackImage);
    addRow(metaLayout, "Remarks:", m_track.trackRemarks.isEmpty() ? "None" : m_track.trackRemarks);

    mainLayout->addWidget(metaGroup);
    mainLayout->addStretch();

    // Dialog Button Bar
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    QPushButton *zoomBtn = new QPushButton("🔍 Zoom to Entity", this);
    zoomBtn->setObjectName("ActionBtn");
    connect(zoomBtn, &QPushButton::clicked, [this]() {
        emit zoomToTrackRequested(m_track);
        accept();
    });

    QPushButton *closeBtn = new QPushButton("Close", this);
    closeBtn->setObjectName("CloseBtn");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(zoomBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);
}

} // namespace GISApp::UI::Tracks
