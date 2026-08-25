/**
 * @file LayerPublishingService.h
 * @brief Facade service coordinating layer publishing strategies and execution.
 * @author GIS System Architecture Team
 * @date 2026
 */

#ifndef LAYERPUBLISHERSERVICE_H
#define LAYERPUBLISHERSERVICE_H

#include <QObject>
#include <memory>
#include "publishing/IPublisherStrategy.h"
#include "publishing/RasterLayerPublisher.h"
#include "publishing/VectorLayerPublisher.h"

namespace GISApp::Publishing {

/**
 * @class LayerPublishingService
 * @brief Service controller coordinating publishing strategy selection and execution.
 */
class LayerPublishingService : public QObject {
    Q_OBJECT

public:
    explicit LayerPublishingService(QObject *parent = nullptr);
    ~LayerPublishingService() override = default;

    /**
     * @brief Publishes layer using specified strategy type.
     */
    bool publishLayer(LayerType type,
                      const QString &folderPath,
                      const QString &layerName,
                      GISApp::Layers::LayerGroupNode *targetGroup,
                      GISApp::Layers::LayerManager *layerManager,
                      QMapLibre::Map *map,
                      ProgressCallback progressCb = nullptr);

    QString lastStatusMessage() const;

private:
    RasterLayerPublisher m_rasterPublisher;
    VectorLayerPublisher m_vectorPublisher;
    QString m_lastStatus;
};

} // namespace GISApp::Publishing

#endif // LAYERPUBLISHER SERVICE_H
