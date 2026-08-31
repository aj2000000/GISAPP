/**
 * @file Layer.h
 * @brief Map visualization configuration (ExpLayer / Explayer) filtering entity data from ExpTable.
 */

#ifndef EXP_LAYER_H
#define EXP_LAYER_H

#include "Table.h"
#include "publishing/LayerRegistryManager.h"
#include <QObject>
#include <QString>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <functional>

namespace GISApp::Experiment {

class ExpLayer : public QObject {
    Q_OBJECT

private:
    QString m_layerName;
    QString m_layerGroup;
    QString m_categoryFilter;
    std::function<bool(const ExpEntity&)> m_customFilter;
    ExpTable *m_table = nullptr;
    QString m_geoJsonFilePath;

public:
    explicit ExpLayer(const QString &layerName = "Experiment Layer",
                      const QString &layerGroup = "Sensors",
                      const QString &categoryFilter = "ALL",
                      ExpTable *table = nullptr,
                      QObject *parent = nullptr)
        : QObject(parent),
          m_layerName(layerName),
          m_layerGroup(layerGroup),
          m_categoryFilter(categoryFilter),
          m_table(table)
    {
        // 1. Register layer group in Layer Tree hierarchy
        GISApp::Publishing::LayerRegistryManager::instance().registerGroup(m_layerGroup);

        // 2. Setup GeoJSON file path
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(dataDir);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        QString sanitizedName = m_layerName;
        sanitizedName = sanitizedName.replace(" ", "_").toLower();
        QString layerId = QString("exp_layer_%1").arg(sanitizedName);
        m_geoJsonFilePath = dir.filePath(QString("%1.geojson").arg(layerId));

        // 3. Register user defined layer in LayerRegistryManager
        GISApp::Publishing::LayerRegistryManager::instance().registerUserDefinedLayer(
            layerId,
            m_layerName,
            1.0f,
            m_layerGroup,
            m_geoJsonFilePath
        );

        if (m_table) {
            connect(m_table, &ExpTable::tableDataUpdated, this, &ExpLayer::onTableDataUpdated);
        }
    }

    virtual ~ExpLayer() = default;

    QString layerName() const { return m_layerName; }
    QString layerGroup() const { return m_layerGroup; }
    QString categoryFilter() const { return m_categoryFilter; }
    QString geoJsonFilePath() const { return m_geoJsonFilePath; }

    void setTable(ExpTable *table) {
        if (m_table == table) return;
        if (m_table) {
            disconnect(m_table, &ExpTable::tableDataUpdated, this, &ExpLayer::onTableDataUpdated);
        }
        m_table = table;
        if (m_table) {
            connect(m_table, &ExpTable::tableDataUpdated, this, &ExpLayer::onTableDataUpdated);
            updateLayer();
        }
    }

    void setCategoryFilter(const QString &category) {
        m_categoryFilter = category;
        updateLayer();
    }

    void setCustomFilter(std::function<bool(const ExpEntity&)> filter) {
        m_customFilter = filter;
        updateLayer();
    }

    QVector<ExpEntity> getFilteredEntities() const {
        if (!m_table) return {};

        QVector<ExpEntity> entities = m_table->filterByCategory(m_categoryFilter);
        if (m_customFilter) {
            QVector<ExpEntity> result;
            for (const auto &entity : entities) {
                if (m_customFilter(entity)) {
                    result.append(entity);
                }
            }
            return result;
        }
        return entities;
    }

    bool updateLayer() {
        QVector<ExpEntity> filteredEntities = getFilteredEntities();
        QJsonObject featureCollection = m_table ? m_table->constructGeoJsonFeatureCollection(filteredEntities)
                                                : QJsonObject{{"type", QString("FeatureCollection")}, {"features", QJsonArray()}};

        QFile file(m_geoJsonFilePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(featureCollection).toJson());
            file.close();
            qDebug() << "[ExpLayer] Updated layer GeoJSON:" << m_layerName << "| File:" << m_geoJsonFilePath << "| Entities count:" << filteredEntities.size();
            emit layerUpdated(m_layerName, m_geoJsonFilePath);
            return true;
        } else {
            qWarning() << "[ExpLayer] Failed to open GeoJSON file for writing:" << m_geoJsonFilePath;
            return false;
        }
    }

private slots:
    void onTableDataUpdated(const QString &tableName) {
        Q_UNUSED(tableName);
        updateLayer();
    }

signals:
    void layerUpdated(const QString &layerName, const QString &geoJsonPath);
};

using Explayer = ExpLayer;

} // namespace GISApp::Experiment

using ExpLayer = GISApp::Experiment::ExpLayer;
using Explayer = GISApp::Experiment::Explayer;

#endif // EXP_LAYER_H
