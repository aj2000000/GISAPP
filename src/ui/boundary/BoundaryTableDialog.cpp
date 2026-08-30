#include "BoundaryTableDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

namespace GISApp::UI::Boundary {

BoundaryTableDialog::BoundaryTableDialog(
    const QVector<Core::Models::BoundaryRecord> &boundaries,
    Controllers::MapController *mapController,
    Core::Services::MapLibreBoundaryAdapter *adapter,
    QWidget *parent)
    : QDialog(parent)
    , m_boundaries(boundaries)
    , m_mapController(mapController)
    , m_adapter(adapter)
{
    setWindowTitle(tr("🇮🇳 National Boundary Entities"));
    resize(760, 480);
    setStyleSheet(R"(
        QDialog {
            background-color: #121824;
            color: #E2E8F0;
        }
        QLabel {
            color: #F8FAFC;
            font-size: 12px;
        }
        QTableWidget {
            background-color: #1e293b;
            alternate-background-color: #0f172a;
            color: #f8fafc;
            gridline-color: #334155;
            selection-background-color: #10b981;
            selection-color: #ffffff;
            border: 1px solid #334155;
            border-radius: 8px;
            outline: 0px;
            font-size: 12px;
        }
        QTableWidget::item {
            color: #f8fafc;
            padding: 6px 10px;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }
        QTableWidget::item:hover {
            background-color: rgba(16, 185, 129, 0.15);
            color: #ffffff;
        }
        QTableWidget::item:selected {
            background-color: #10b981;
            color: #ffffff;
            font-weight: bold;
        }
        QHeaderView {
            background-color: #0f172a;
            border: none;
            border-bottom: 2px solid #10b981;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #34d399;
            padding: 8px 12px;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-right: 1px solid #334155;
            border-bottom: 2px solid #10b981;
        }
        QPushButton {
            background-color: #1E293B;
            color: #34D399;
            border: 1px solid #10B981;
            border-radius: 4px;
            padding: 6px 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #10B981;
            color: #FFFFFF;
        }
        QPushButton#deleteBtn {
            border: 1px solid #EF4444;
            color: #FCA5A5;
        }
        QPushButton#deleteBtn:hover {
            background-color: #DC2626;
            color: #FFFFFF;
        }
        QLineEdit {
            background-color: #1E293B;
            color: #F8FAFC;
            border: 1px solid #334155;
            border-radius: 4px;
            padding: 5px 8px;
        }
    )");

    setupUi();
    refreshData();
}

void BoundaryTableDialog::setBoundaries(const QVector<Core::Models::BoundaryRecord> &boundaries)
{
    m_boundaries = boundaries;
    refreshData();
}

void BoundaryTableDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel(tr("Filter:"), this);
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText(tr("Filter boundary entities by name..."));
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &BoundaryTableDialog::refreshData);

    topLayout->addWidget(searchLabel);
    topLayout->addWidget(m_searchLineEdit);
    mainLayout->addLayout(topLayout);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(6);
    m_tableWidget->setHorizontalHeaderLabels({
        tr("ID"), tr("Boundary Name"), tr("Geometry Type"), tr("Vertices"), tr("Style / Stroke Color"), tr("Bounding Box Extent")
    });
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addWidget(m_tableWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *zoomBtn = new QPushButton(tr("🔍 Zoom to Boundary Polyline"), this);
    QPushButton *clearBtn = new QPushButton(tr("💣 Clear All"), this);
    clearBtn->setObjectName("deleteBtn");
    QPushButton *closeBtn = new QPushButton(tr("Close"), this);

    connect(zoomBtn, &QPushButton::clicked, this, &BoundaryTableDialog::onZoomToSelected);
    connect(clearBtn, &QPushButton::clicked, this, &BoundaryTableDialog::onClearAll);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(zoomBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #94A3B8; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);
}

void BoundaryTableDialog::refreshData()
{
    QString filter = m_searchLineEdit ? m_searchLineEdit->text().trimmed() : QString();

    m_tableWidget->setRowCount(0);

    int row = 0;
    for (const auto &rec : m_boundaries) {
        if (!filter.isEmpty() && !rec.name.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(rec.boundaryId)));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(rec.name));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(tr("Polyline (LineString)")));
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(rec.points.size())));
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(tr("Outer: Green (#16A34A) | Inner: Saffron (#FF7700)")));

        double minLat = 90.0, maxLat = -90.0;
        double minLon = 180.0, maxLon = -180.0;
        for (const auto &pt : rec.points) {
            if (pt.latitude < minLat) minLat = pt.latitude;
            if (pt.latitude > maxLat) maxLat = pt.latitude;
            if (pt.longitude < minLon) minLon = pt.longitude;
            if (pt.longitude > maxLon) maxLon = pt.longitude;
        }

        QString extentStr = QString("[%1°, %2°] to [%3°, %4°]")
                                .arg(minLat, 0, 'f', 2)
                                .arg(minLon, 0, 'f', 2)
                                .arg(maxLat, 0, 'f', 2)
                                .arg(maxLon, 0, 'f', 2);
        m_tableWidget->setItem(row, 5, new QTableWidgetItem(extentStr));

        row++;
    }

    if (m_statusLabel) {
        m_statusLabel->setText(tr("Total Boundary Polylines: %1").arg(row));
    }
}

void BoundaryTableDialog::onZoomToSelected()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0 && m_tableWidget->rowCount() > 0) {
        currentRow = 0;
    }
    if (currentRow < 0) return;

    int bId = m_tableWidget->item(currentRow, 0)->text().toInt();
    for (const auto &rec : m_boundaries) {
        if (rec.boundaryId == bId && !rec.points.isEmpty()) {
            double minLat = 90.0, maxLat = -90.0;
            double minLon = 180.0, maxLon = -180.0;
            for (const auto &pt : rec.points) {
                if (pt.latitude < minLat) minLat = pt.latitude;
                if (pt.latitude > maxLat) maxLat = pt.latitude;
                if (pt.longitude < minLon) minLon = pt.longitude;
                if (pt.longitude > maxLon) maxLon = pt.longitude;
            }
            double centerLat = (minLat + maxLat) / 2.0;
            double centerLon = (minLon + maxLon) / 2.0;

            if (m_mapController) {
                m_mapController->centerOn(Core::Models::GeoCoordinate(centerLat, centerLon), 7.0);
            }
            break;
        }
    }
}

void BoundaryTableDialog::onClearAll()
{
    if (m_boundaries.isEmpty()) {
        QMessageBox::information(this, tr("Clear Boundary"), tr("No boundary entities to clear."));
        return;
    }

    if (QMessageBox::question(this, tr("Clear Boundary Entities"),
                              tr("Are you sure you want to clear all boundary entities from the map?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        m_boundaries.clear();
        if (m_adapter) {
            m_adapter->setBoundaries({});
        }
        refreshData();
    }
}

} // namespace GISApp::UI::Boundary
