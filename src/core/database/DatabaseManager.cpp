#include "DatabaseManager.h"
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>

namespace GISApp::Core::Database {

DatabaseManager* DatabaseManager::s_instance = nullptr;
QMutex DatabaseManager::s_mutex;

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    close();
}

DatabaseManager* DatabaseManager::instance()
{
    QMutexLocker locker(&s_mutex);
    if (!s_instance) {
        s_instance = new DatabaseManager();
    }
    return s_instance;
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    if (m_initialized && m_db.isOpen()) {
        return true;
    }

    QString finalDbPath = dbPath;
    if (finalDbPath.isEmpty()) {
        QString mapDataDir = QDir::homePath() + "/MAPDATA";
        QDir dir;
        if (!dir.exists(mapDataDir)) {
            dir.mkpath(mapDataDir);
        }
        finalDbPath = mapDataDir + "/gis_app_database.sqlite";
    }

    qDebug() << "[DatabaseManager] Initializing SQLite database at:" << finalDbPath;

    m_db = QSqlDatabase::addDatabase("QSQLITE", "GISAppConnection");
    m_db.setDatabaseName(finalDbPath);

    if (!m_db.open()) {
        qCritical() << "[DatabaseManager] Failed to open SQLite database:" << m_db.lastError().text();
        return false;
    }

    if (!createSchema()) {
        qCritical() << "[DatabaseManager] Failed to execute database schema creation DDL.";
        return false;
    }

    m_initialized = true;
    qDebug() << "[DatabaseManager] SQLite database initialized successfully.";
    return true;
}

QSqlDatabase DatabaseManager::database() const
{
    return QSqlDatabase::database("GISAppConnection");
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    QSqlDatabase::removeDatabase("GISAppConnection");
    m_initialized = false;
}

bool DatabaseManager::createSchema()
{
    QSqlQuery query(m_db);

    // Schema Check: If table exists but lacks TRACK_PLOT_TYPE or INT_NO column, upgrade it
    if (query.exec("PRAGMA table_info(TRACKS);")) {
        bool hasPlotType = false;
        bool sourcesIsText = false;
        while (query.next()) {
            QString name = query.value(1).toString();
            QString type = query.value(2).toString();
            if (name.compare("TRACK_PLOT_TYPE", Qt::CaseInsensitive) == 0) {
                hasPlotType = true;
            }
            if (name.compare("TRACK_SOURCES", Qt::CaseInsensitive) == 0 && type.compare("TEXT", Qt::CaseInsensitive) == 0) {
                sourcesIsText = true;
            }
        }
        if (!hasPlotType || !sourcesIsText) {
            qDebug() << "[DatabaseManager] Existing TRACKS schema outdated. Upgrading to 20-column schema...";
            QSqlQuery dropQuery(m_db);
            dropQuery.exec("DROP TABLE IF EXISTS TRACKS;");
        }
    }

    const QString createTracksDdl = R"(
        CREATE TABLE IF NOT EXISTS TRACKS (
            TRACK_ID INTEGER PRIMARY KEY,
            TRACK_NAME TEXT,
            TRACK_PLOT_TYPE REAL,
            INT_NO INTEGER,
            TRACK_LAT REAL,
            TRACK_LONG REAL,
            TRACK_HEIGHT REAL,
            TRACK_DIR REAL,
            TRACK_IDENTITY INTEGER,
            TRACK_TYPE INTEGER,
            TRACK_SUB_TYPE INTEGER,
            TRACK_CLASS INTEGER,
            TRACK_STRENGTH INTEGER,
            TRACK_ACT_TYPE INTEGER,
            TRACK_ACT_SUB_TYPE INTEGER,
            TRACK_ACT_CLASS INTEGER,
            TRACK_SYSTEM_TYPE INTEGER,
            TRACK_SOURCES TEXT,
            TRACK_IMAGE TEXT,
            TRACK_REMARKS TEXT
        );
    )";

    if (!query.exec(createTracksDdl)) {
        qCritical() << "[DatabaseManager] DDL error creating TRACKS table:" << query.lastError().text();
        return false;
    }

    const QString createAovDdl = R"(
        CREATE TABLE IF NOT EXISTS AREA_OF_VIEW (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            NAME TEXT,
            N_POINTS INTEGER,
            POINTS_JSON TEXT
        );
    )";

    if (!query.exec(createAovDdl)) {
        qCritical() << "[DatabaseManager] DDL error creating AREA_OF_VIEW table:" << query.lastError().text();
        return false;
    }

    return true;
}

} // namespace GISApp::Core::Database
