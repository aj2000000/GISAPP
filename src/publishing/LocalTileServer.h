/**
 * @file LocalTileServer.h
 * @brief Embedded Dynamic XYZ Raster Tile HTTP Service.
 */

#ifndef LOCALTILESERVER_H
#define LOCALTILESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QImage>

namespace GISApp::Publishing {

class LocalTileServer : public QTcpServer {
    Q_OBJECT

public:
    static LocalTileServer& instance();
    bool startServer(quint16 port = 8088);

    void registerLayerTexture(const QString &layerId, const QImage &image);
    QString getLayerUrl(const QString &layerId) const;
    void registerLayerFolder(const QString &layerId, const QString &folderPath);
    void registerLayerTilePath(const QString &layerId, const QString &tilePath);
    QString getTileUrlTemplate(const QString &layerId) const;

    void registerVectorMbtiles(const QString &layerId, const QString &mbtilesPath);
    QString getVectorUrlTemplate(const QString &layerId) const;
    QByteArray getVectorTileData(const QString &mbtilesPath, int z, int x, int y);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    explicit LocalTileServer(QObject *parent = nullptr);
    ~LocalTileServer() override = default;

    quint16 m_port{8088};
    QMap<QString, QString> m_layerFolders;
    QMap<QString, QString> m_vrtPaths;
    QMap<QString, QString> m_tilePaths;
    QMap<QString, QString> m_vectorMbtilesPaths;
    QMap<QString, QByteArray> m_tileCache;
};

} // namespace GISApp::Publishing

#endif // LOCALTILESERVER_H
