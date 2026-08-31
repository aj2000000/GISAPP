#include "AreaOfViewTableDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>

namespace GISApp::UI::AreaOfView {

AreaOfViewTableDialog::AreaOfViewTableDialog(
    Core::Repositories::IAreaOfViewRepository *repository,
    Controllers::MapController *mapController,
    Core::Services::MapLibreAreaOfViewAdapter *adapter,
    QWidget *parent)
    : QDialog(parent)
    , m_repository(repository)
    , m_mapController(mapController)
    , m_adapter(adapter)
{
    setWindowTitle(tr("👁️ Area of View Tactical Entities"));
    resize(720, 480);
    setStyleSheet(R"(
        QDialog {
            background-color: #121824;
            color: #E2E8F0;
        }
        QLabel {
            color: #F8FAFC;
            font-size: 12px;
        }
        QMessageBox {
            background-color: #121824;
            color: #F8FAFC;
            border: 1px solid #334155;
            border-radius: 8px;
        }
        QMessageBox QLabel {
            color: #F8FAFC;
            font-size: 13px;
            font-weight: 500;
            background: transparent;
        }
        QMessageBox QPushButton {
            background-color: #1E293B;
            color: #38BDF8;
            border: 1px solid #0284C7;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: bold;
            font-size: 12px;
            min-width: 75px;
        }
        QMessageBox QPushButton:hover {
            background-color: #0284C7;
            color: #FFFFFF;
        }
        QTableWidget {
            background-color: #1e293b;
            alternate-background-color: #0f172a;
            color: #f8fafc;
            gridline-color: #334155;
            selection-background-color: #0284c7;
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
            background-color: rgba(56, 189, 248, 0.15);
            color: #ffffff;
        }
        QTableWidget::item:selected {
            background-color: #0284c7;
            color: #ffffff;
            font-weight: bold;
        }
        QHeaderView {
            background-color: #0f172a;
            border: none;
            border-bottom: 2px solid #0284c7;
        }
        QHeaderView::section {
            background-color: #0f172a;
            color: #38bdf8;
            padding: 8px 12px;
            font-weight: bold;
            font-size: 12px;
            border: none;
            border-right: 1px solid #334155;
            border-bottom: 2px solid #0284c7;
        }
        QPushButton {
            background-color: #1E293B;
            color: #38BDF8;
            border: 1px solid #0284C7;
            border-radius: 4px;
            padding: 6px 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0284C7;
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
        QScrollBar:vertical {
            background: #0f172a;
            width: 10px;
            margin: 0px;
            border-radius: 5px;
        }
        QScrollBar::handle:vertical {
            background: #334155;
            min-height: 25px;
            border-radius: 4px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background: #38bdf8;
        }
        QScrollBar::handle:vertical:pressed {
            background: #0284c7;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
            background: none;
            border: none;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
        QScrollBar:horizontal {
            background: #0f172a;
            height: 10px;
            margin: 0px;
            border-radius: 5px;
        }
        QScrollBar::handle:horizontal {
            background: #334155;
            min-width: 25px;
            border-radius: 4px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #38bdf8;
        }
        QScrollBar::handle:horizontal:pressed {
            background: #0284c7;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
            background: none;
            border: none;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }
    )");

    setupUi();
    refreshData();

    if (m_repository) {
        connect(m_repository, &Core::Repositories::IAreaOfViewRepository::areaOfViewUpdated,
                this, &AreaOfViewTableDialog::refreshData);
    }
}

void AreaOfViewTableDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *topLayout = new QHBoxLayout();
    QLabel *searchLabel = new QLabel(tr("Filter:"), this);
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText(tr("Search by Area Name..."));
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &AreaOfViewTableDialog::refreshData);

    topLayout->addWidget(searchLabel);
    topLayout->addWidget(m_searchLineEdit);
    mainLayout->addLayout(topLayout);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(4);
    m_tableWidget->setHorizontalHeaderLabels({tr("ID"), tr("Name"), tr("N Points"), tr("Coordinates Summary")});
    m_tableWidget->setAlternatingRowColors(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    mainLayout->addWidget(m_tableWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *zoomBtn = new QPushButton(tr("🔍 Zoom to Area"), this);
    QPushButton *deleteBtn = new QPushButton(tr("🗑️ Delete Selected"), this);
    deleteBtn->setObjectName("deleteBtn");
    QPushButton *clearBtn = new QPushButton(tr("💣 Clear All"), this);
    clearBtn->setObjectName("deleteBtn");
    QPushButton *closeBtn = new QPushButton(tr("Close"), this);

    connect(zoomBtn, &QPushButton::clicked, this, &AreaOfViewTableDialog::onZoomToSelected);
    connect(deleteBtn, &QPushButton::clicked, this, &AreaOfViewTableDialog::onDeleteSelected);
    connect(clearBtn, &QPushButton::clicked, this, &AreaOfViewTableDialog::onClearAll);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLayout->addWidget(zoomBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);

    mainLayout->addLayout(btnLayout);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #94A3B8; font-size: 11px;");
    mainLayout->addWidget(m_statusLabel);
}

void AreaOfViewTableDialog::refreshData()
{
    if (!m_repository) return;

    auto records = m_repository->getAll();
    QString filter = m_searchLineEdit ? m_searchLineEdit->text().trimmed() : QString();

    m_tableWidget->setRowCount(0);

    int row = 0;
    for (const auto &rec : records) {
        if (!filter.isEmpty() && !rec.name.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        m_tableWidget->insertRow(row);
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(rec.id)));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(rec.name));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(rec.nPoints)));

        QString coordsSummary;
        for (int i = 0; i < std::min<int>(3, rec.points.size()); ++i) {
            coordsSummary += QString("[%1, %2] ").arg(rec.points[i].latitude, 0, 'f', 4).arg(rec.points[i].longitude, 0, 'f', 4);
        }
        if (rec.points.size() > 3) coordsSummary += "...";
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(coordsSummary));

        row++;
    }

    if (m_statusLabel) {
        m_statusLabel->setText(tr("Total Area of View Polygons: %1").arg(row));
    }
}

void AreaOfViewTableDialog::onZoomToSelected()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) return;

    int id = m_tableWidget->item(currentRow, 0)->text().toInt();
    if (m_repository && m_mapController) {
        auto opt = m_repository->getById(id);
        if (opt && !opt->points.isEmpty()) {
            double sumLat = 0, sumLon = 0;
            for (const auto &pt : opt->points) {
                sumLat += pt.latitude;
                sumLon += pt.longitude;
            }
            double centerLat = sumLat / opt->points.size();
            double centerLon = sumLon / opt->points.size();
            m_mapController->centerOn(Core::Models::GeoCoordinate(centerLat, centerLon), 12.0);
        }
    }
}

void AreaOfViewTableDialog::onDeleteSelected()
{
    int currentRow = m_tableWidget->currentRow();
    if (currentRow < 0) return;

    int id = m_tableWidget->item(currentRow, 0)->text().toInt();
    if (m_repository) {
        m_repository->deleteById(id);
        if (m_adapter) m_adapter->refreshFromRepository();
        refreshData();
    }
}

void AreaOfViewTableDialog::onClearAll()
{
    if (!m_repository) return;
    if (QMessageBox::question(this, tr("Clear Database"), tr("Are you sure you want to delete all Area of View polygons?")) == QMessageBox::Yes) {
        m_repository->clearAll();
        if (m_adapter) m_adapter->refreshFromRepository();
        refreshData();
    }
}

} // namespace GISApp::UI::AreaOfView
