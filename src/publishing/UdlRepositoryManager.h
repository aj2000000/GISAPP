/**
 * @file UdlRepositoryManager.h
 * @brief Singleton repository managing UDL entity persistence (SQLite + GeoJSON sync).
 */

#ifndef UDLREPOSITORYMANAGER_H
#define UDLREPOSITORYMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFile>
#include <QDebug>
#include "layers/ILayerAdapter.h"

namespace GISApp::Publishing {

struct UdlEntityItem {
    QString entityId;
    QString layerId;
    QString entityName;
    QString entityType; // Point, Polyline, Polygon, Text
    QJsonObject geometryJson;
    QJsonObject styleJson;
    QString createdAt;
};

enum class UdlUndoType { Create, Delete, Update };

struct UdlUndoItem {
    UdlUndoType type;
    UdlEntityItem primary;
    UdlEntityItem secondary;
};

class UdlRepositoryManager : public QObject {
    Q_OBJECT

public:
    static UdlRepositoryManager& instance();

    bool saveEntity(const UdlEntityItem &item, bool recordUndo = true);
    bool deleteEntity(const QString &entityId, const QString &layerId, bool recordUndo = true);
    bool deleteLayer(const QString &layerId);
    bool undoLastAction();
    bool canUndo() const { return !m_undoStack.isEmpty(); }

    QList<UdlEntityItem> getEntitiesForLayer(const QString &layerId);
    QList<UdlEntityItem> getAllEntities();
    bool getEntity(const QString &entityId, UdlEntityItem &outItem);

    QString getUdlGeoJsonPath(const QString &layerId);
    bool syncGeoJsonFile(const QString &layerId);
    GISApp::Layers::LayerExtent calculateLayerExtent(const QString &layerId);

    // Entity Clipboard (Copy & Paste)
    void setCopiedEntity(const UdlEntityItem &item) { m_copiedEntity = item; m_hasCopiedEntity = true; }
    UdlEntityItem copiedEntity() const { return m_copiedEntity; }
    bool hasCopiedEntity() const { return m_hasCopiedEntity; }
    void clearCopiedEntity() { m_hasCopiedEntity = false; }

signals:
    void udlLayerUpdated(const QString &layerId, const QString &geojsonPath);
    void undoStateChanged(bool canUndo);

private:
    UdlRepositoryManager(QObject *parent = nullptr);
    ~UdlRepositoryManager() override = default;

    QString storageDirectoryPath() const;
    QList<UdlUndoItem> m_undoStack;
    UdlEntityItem m_copiedEntity;
    bool m_hasCopiedEntity{false};
};

} // namespace GISApp::Publishing

#endif // UDLREPOSITORYMANAGER_H
