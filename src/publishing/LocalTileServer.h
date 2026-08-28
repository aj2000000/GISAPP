/**
 * @file LocalTileServer.h
 * @brief Embedded Dynamic XYZ Raster Tile HTTP Service with Native C++ GDAL Tile Rendering & LRU Bounded Cache.
 */

#ifndef LOCALTILESERVER_H
#define LOCALTILESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QImage>
#include <QMutex>
#include <QQueue>

typedef void* GDALDatasetH;

namespace GISApp::Publishing {

class LocalTileServer : public QTcpServer {
    Q_OBJECT

public:
    static LocalTileServer& instance();
    bool startServer(quint16 port = 8088);

    void registerLayerTexture(const QString &layerId, const QImage &image);
    QString getLayerUrl(const QString &layerId) const;
    void registerLayerFolder(const QString &layerId, const QString &folderPath);
    void registerLayerVrtPath(const QString &layerId, const QString &vrtPath);
    void registerLayerTilePath(const QString &layerId, const QString &tilePath);
    QString getTileUrlTemplate(const QString &layerId) const;

    void registerVectorMbtiles(const QString &layerId, const QString &mbtilesPath);
    QString getVectorUrlTemplate(const QString &layerId) const;
    QByteArray getVectorTileData(const QString &mbtilesPath, int z, int x, int y);

    QByteArray renderNativeGdalTile(const QString &vrtPath, double west, double north, double east, double south);

    void clearCache();
    void unregisterLayer(const QString &layerId);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    explicit LocalTileServer(QObject *parent = nullptr);
    ~LocalTileServer() override;

    void putTileCache(const QString &key, const QByteArray &data);
    QByteArray getTileCache(const QString &key);

    static constexpr int MAX_CACHE_ENTRIES = 500; // LRU Memory Bounding (~25-50 MB max)

    quint16 m_port{8088};
    mutable QMutex m_mutex;

    QMap<QString, QString> m_layerFolders;
    QMap<QString, QString> m_vrtPaths;
    QMap<QString, QString> m_tilePaths;
    QMap<QString, QString> m_vectorMbtilesPaths;
    QMap<QString, QByteArray> m_tileCache;
    QQueue<QString> m_cacheKeyOrder;
    QMap<QString, GDALDatasetH> m_gdalDatasets;
};

} // namespace GISApp::Publishing

#endif // LOCALTILESERVER_H
