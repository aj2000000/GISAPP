/**
 * @file SystemConfigManager.cpp
 * @brief Implementation of SystemConfigManager for loading system settings.
 * @author GIS System Architecture Team
 * @date 2026
 */

#include "core/SystemConfigManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCoreApplication>
#include <QDebug>

namespace GISApp::Core {

SystemConfigManager::SystemConfigManager(QObject *parent)
    : QObject(parent)
{
    loadConfig();
}

SystemConfigManager& SystemConfigManager::instance() {
    static SystemConfigManager instance;
    return instance;
}

QString SystemConfigManager::resolvePath(const QString &rawPath) const {
    if (rawPath.startsWith("$HOME")) {
        QString rel = rawPath.mid(5);
        if (rel.startsWith('/') || rel.startsWith('\\')) rel = rel.mid(1);
        return QDir::cleanPath(QString("%1/%2").arg(QDir::homePath()).arg(rel));
    }
    if (QDir::isRelativePath(rawPath)) {
        QString p1 = QDir::cleanPath(QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg(rawPath));
        if (QFile::exists(p1)) return p1;

        QString p2 = QDir::cleanPath(QString("%1/../%2").arg(QCoreApplication::applicationDirPath()).arg(rawPath));
        if (QFile::exists(p2)) return p2;

        QString p3 = QDir::cleanPath(QString("%1/../../%2").arg(QCoreApplication::applicationDirPath()).arg(rawPath));
        if (QFile::exists(p3)) return p3;

        return QDir::cleanPath(QString("%1/%2").arg(QDir::currentPath()).arg(rawPath));
    }
    return QDir::cleanPath(rawPath);
}

bool SystemConfigManager::loadConfig(const QString &configFilePath) {
    if (!configFilePath.isEmpty()) {
        m_configFilePath = configFilePath;
    } else {
        m_configFilePath = QString("%1/config/system_config.json").arg(QCoreApplication::applicationDirPath());
        if (!QFile::exists(m_configFilePath)) {
            m_configFilePath = QString("%1/../config/system_config.json").arg(QCoreApplication::applicationDirPath());
        }
        if (!QFile::exists(m_configFilePath)) {
            m_configFilePath = "config/system_config.json";
        }
    }

    QFile file(m_configFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[SystemConfig] Config file not found at" << m_configFilePath << ". Using defaults.";
        m_mapDataDir = QString("%1/MAPDATA").arg(QDir::homePath());
        m_vectorTilesDir = QString("%1/MAPDATA/VectorTiles").arg(QDir::homePath());
        m_tileServerHost = "127.0.0.1";
        m_tileServerPort = 8088;
        m_defaultMinZoom = 0;
        m_defaultMaxZoom = 16;
        m_offlineStylePath = "config/offline_dark_style.json";
        m_publishedLayersPath = "config/published_layers.json";
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "[SystemConfig] Invalid JSON in" << m_configFilePath;
        return false;
    }

    QJsonObject obj = doc.object();
    m_mapDataDir = resolvePath(obj.value("mapDataDir").toString("$HOME/MAPDATA"));
    m_vectorTilesDir = resolvePath(obj.value("vectorTilesDir").toString("$HOME/MAPDATA/VectorTiles"));
    m_tileServerHost = obj.value("tileServerHost").toString("127.0.0.1");
    m_tileServerPort = obj.value("tileServerPort").toInt(8088);
    m_defaultMinZoom = obj.value("defaultMinZoom").toInt(0);
    m_defaultMaxZoom = obj.value("defaultMaxZoom").toInt(16);
    m_offlineStylePath = resolvePath(obj.value("offlineStylePath").toString("config/offline_dark_style.json"));
    m_publishedLayersPath = resolvePath(obj.value("publishedLayersPath").toString("config/published_layers.json"));

    // Ensure map directories exist
    QDir().mkpath(m_mapDataDir);
    QDir().mkpath(m_vectorTilesDir);

    qWarning() << "[SystemConfig] Successfully loaded configuration. MapDataDir:" << m_mapDataDir;
    return true;
}

bool SystemConfigManager::saveConfig() {
    QJsonObject obj;
    obj["mapDataDir"] = "$HOME/MAPDATA";
    obj["vectorTilesDir"] = "$HOME/MAPDATA/VectorTiles";
    obj["tileServerHost"] = m_tileServerHost;
    obj["tileServerPort"] = m_tileServerPort;
    obj["defaultMinZoom"] = m_defaultMinZoom;
    obj["defaultMaxZoom"] = m_defaultMaxZoom;
    obj["offlineStylePath"] = "config/offline_dark_style.json";
    obj["publishedLayersPath"] = "config/published_layers.json";

    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[SystemConfig] Failed to write config to" << m_configFilePath;
        return false;
    }

    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QString SystemConfigManager::getMapDataDir() const { return m_mapDataDir; }
QString SystemConfigManager::getVectorTilesDir() const { return m_vectorTilesDir; }
QString SystemConfigManager::getTileServerHost() const { return m_tileServerHost; }
int SystemConfigManager::getTileServerPort() const { return m_tileServerPort; }
int SystemConfigManager::getDefaultMinZoom() const { return m_defaultMinZoom; }
int SystemConfigManager::getDefaultMaxZoom() const { return m_defaultMaxZoom; }
QString SystemConfigManager::getOfflineStylePath() const { return m_offlineStylePath; }
QString SystemConfigManager::getPublishedLayersPath() const { return m_publishedLayersPath; }

void SystemConfigManager::setMapDataDir(const QString &path) {
    m_mapDataDir = resolvePath(path);
    m_vectorTilesDir = QString("%1/VectorTiles").arg(m_mapDataDir);
    QDir().mkpath(m_mapDataDir);
    QDir().mkpath(m_vectorTilesDir);
    saveConfig();
}

} // namespace GISApp::Core
