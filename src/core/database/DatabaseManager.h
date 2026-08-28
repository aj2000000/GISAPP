/**
 * @file DatabaseManager.h
 * @brief Singleton manager for SQLite database connection lifecycle and schema migration.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QMutex>

namespace GISApp::Core::Database {

class DatabaseManager {
public:
    static DatabaseManager* instance();

    /**
     * @brief Initialize SQLite database connection and create schema tables if missing.
     * @return true if initialization succeeded.
     */
    bool initialize(const QString &dbPath = QString());

    /**
     * @brief Get active QSqlDatabase handle.
     */
    QSqlDatabase database() const;

    /**
     * @brief Check if database connection is open.
     */
    bool isOpen() const;

    /**
     * @brief Close database connection.
     */
    void close();

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    bool createSchema();

    static DatabaseManager* s_instance;
    static QMutex s_mutex;
    QSqlDatabase m_db;
    bool m_initialized{false};
};

} // namespace GISApp::Core::Database

#endif // DATABASEMANAGER_H
