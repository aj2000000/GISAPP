/**
 * @file TrackDetailDialog.h
 * @brief Modal / Modeless Dialog showing comprehensive track telemetry details.
 */

#ifndef TRACKDETAILDIALOG_H
#define TRACKDETAILDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include "core/models/TrackRecord.h"

namespace GISApp::UI::Tracks {

class TrackDetailDialog : public QDialog {
    Q_OBJECT

public:
    explicit TrackDetailDialog(const GISApp::Core::Models::TrackRecord &track, QWidget *parent = nullptr);
    ~TrackDetailDialog() override = default;

signals:
    void zoomToTrackRequested(const GISApp::Core::Models::TrackRecord &track);

private:
    void setupUi();
    void setupStyle();
    void applyDarkTheme();
    QString formatDms(double val, bool isLatitude) const;

    GISApp::Core::Models::TrackRecord m_track;
};

} // namespace GISApp::UI::Tracks

#endif // TRACKDETAILDIALOG_H
