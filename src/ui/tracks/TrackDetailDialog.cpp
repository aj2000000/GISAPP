/**
 * @file TrackDetailDialog.cpp
 * @brief Implementation of tactical track detail and technical telemetry dialog.
 * @date 2026
 */

#include "TrackDetailDialog.h"
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
    setMinimumWidth(450);
    resize(480, 520);
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
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Header Card
    QFrame *headerFrame = new QFrame(this);
    headerFrame->setStyleSheet("QFrame { background-color: #1E293B; border-radius: 8px; border: 1px solid #334155; padding: 10px; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);

    QLabel *iconLabel = new QLabel("✈️", this);
    iconLabel->setStyleSheet("font-size: 28px; padding-right: 8px;");
    headerLayout->addWidget(iconLabel);

    QVBoxLayout *headerTextLayout = new QVBoxLayout();
    QString name = m_track.trackName.isEmpty() ? QString("Track #%1").arg(m_track.trackId) : m_track.trackName;
    QLabel *titleLabel = new QLabel(name, this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #F8FAFC;");

    QString identityStr;
    switch (m_track.trackIdentity) {
        case 1: identityStr = "Friendly"; break;
        case 2: identityStr = "Hostile"; break;
        case 3: identityStr = "Neutral"; break;
        default: identityStr = "Pending / Unknown"; break;
    }

    QLabel *subTitleLabel = new QLabel(QString("ID: %1 | Identity: %2 | Type Code: %3")
                                           .arg(m_track.trackId)
                                           .arg(identityStr)
                                           .arg(m_track.trackType), this);
    subTitleLabel->setStyleSheet("font-size: 11px; color: #38BDF8;");

    headerTextLayout->addWidget(titleLabel);
    headerTextLayout->addWidget(subTitleLabel);
    headerLayout->addLayout(headerTextLayout);
    headerLayout->addStretch();

    mainLayout->addWidget(headerFrame);

    // Group 1: Geographic Position
    QGroupBox *geoGroup = new QGroupBox("Geographic Positioning", this);
    QFormLayout *geoLayout = new QFormLayout(geoGroup);
    geoLayout->setLabelAlignment(Qt::AlignLeft);
    geoLayout->setFormAlignment(Qt::AlignLeft);
    geoLayout->setSpacing(8);

    auto formatDms = [](double val, bool isLat) {
        char dir = isLat ? (val >= 0 ? 'N' : 'S') : (val >= 0 ? 'E' : 'W');
        val = std::abs(val);
        int deg = static_cast<int>(val);
        double minVal = (val - deg) * 60.0;
        int min = static_cast<int>(minVal);
        double sec = (minVal - min) * 60.0;
        return QString("%1° %2' %3\" %4").arg(deg).arg(min, 2, 10, QChar('0')).arg(sec, 5, 'f', 2, QChar('0')).arg(dir);
    };

    auto addRow = [geoLayout](const QString &label, const QString &valStr) {
        QLabel *lbl = new QLabel(label);
        QLabel *val = new QLabel(valStr);
        val->setObjectName("ValLabel");
        geoLayout->addRow(lbl, val);
    };

    addRow("Latitude:", QString("%1°  (%2)").arg(m_track.trackLat, 0, 'f', 6).arg(formatDms(m_track.trackLat, true)));
    addRow("Longitude:", QString("%1°  (%2)").arg(m_track.trackLong, 0, 'f', 6).arg(formatDms(m_track.trackLong, false)));

    mainLayout->addWidget(geoGroup);

    // Group 2: Technical Telemetry & Kinematics
    QGroupBox *telemGroup = new QGroupBox("Technical Telemetry & Dynamics", this);
    QFormLayout *telemLayout = new QFormLayout(telemGroup);
    telemLayout->setSpacing(8);

    auto addTelemRow = [telemLayout](const QString &label, const QString &valStr) {
        QLabel *lbl = new QLabel(label);
        QLabel *val = new QLabel(valStr);
        val->setObjectName("ValLabel");
        telemLayout->addRow(lbl, val);
    };

    addTelemRow("Altitude / Height:", QString("%1 m (%2 ft)").arg(m_track.trackHeight, 0, 'f', 1).arg(m_track.trackHeight * 3.28084, 0, 'f', 0));
    addTelemRow("Bearing / Direction:", QString("%1°").arg(m_track.trackDir, 0, 'f', 1));
    addTelemRow("Plot Type:", QString::number(m_track.trackPlotType));
    addTelemRow("Internal No:", QString::number(m_track.intNo));

    if (!m_track.trackSources.isEmpty()) {
        addTelemRow("Sources:", m_track.trackSources);
    }
    if (!m_track.trackRemarks.isEmpty()) {
        addTelemRow("Remarks:", m_track.trackRemarks);
    }

    mainLayout->addWidget(telemGroup);
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
