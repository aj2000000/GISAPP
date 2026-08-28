/**
 * @file SystemConfigManager.h
 * @brief Singleton manager for loading, saving, and expanding system configuration settings.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef SYSTEMCONFIGMANAGER_H
#define SYSTEMCONFIGMANAGER_H

#include <QString>
#include <QObject>

namespace GISApp::Core {

class SystemConfigManager : public QObject {
    Q_OBJECT

public:
    static SystemConfigManager& instance();

    bool loadConfig(const QString &configFilePath = "");
    bool saveConfig();

    QString getMapDataDir() const;
    QString getVectorTilesDir() const;
    QString getTileServerHost() const;
    int getTileServerPort() const;
    int getDefaultMinZoom() const;
    int getDefaultMaxZoom() const;
    QString getOfflineStylePath() const;
    QString getPublishedLayersPath() const;

    void setMapDataDir(const QString &path);

private:
    explicit SystemConfigManager(QObject *parent = nullptr);
    ~SystemConfigManager() override = default;
    Q_DISABLE_COPY(SystemConfigManager)

    QString resolvePath(const QString &rawPath) const;

    QString m_configFilePath;
    QString m_mapDataDir;
    QString m_vectorTilesDir;
    QString m_tileServerHost;
    int m_tileServerPort{8088};
    int m_defaultMinZoom{0};
    int m_defaultMaxZoom{16};
    QString m_offlineStylePath;
    QString m_publishedLayersPath;
};

} // namespace GISApp::Core

#endif // SYSTEMCONFIGMANAGER_H
