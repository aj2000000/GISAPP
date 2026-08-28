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
#include "publishing/PublisherFactory.h"

namespace GISApp::Publishing {

/**
 * @class LayerPublishingService
 * @brief Service controller coordinating publishing strategy selection and execution.
 * Uses PublisherFactory to adhere to Dependency Inversion (DIP) and Open/Closed Principle (OCP).
 */
class LayerPublishingService : public QObject {
    Q_OBJECT

public:
    explicit LayerPublishingService(QObject *parent = nullptr);
    ~LayerPublishingService() override = default;

    /**
     * @brief Publishes layer using specified strategy type, zoom controls, and optional background thread.
     */
    bool publishLayer(LayerType type,
                      const QString &folderPath,
                      const QString &layerName,
                      GISApp::Layers::LayerGroupNode *targetGroup,
                      GISApp::Layers::LayerManager *layerManager,
                      QMapLibre::Map *map,
                      ProgressCallback progressCb = nullptr,
                      int minZoom = 0,
                      int maxZoom = 22,
                      bool runInBackground = false,
                      bool suppressNotification = false,
                      float initialOpacity = 1.0f,
                      bool initialVisible = true);

    QString lastStatusMessage() const;

private:
    QString m_lastStatus;
};

} // namespace GISApp::Publishing

#endif // LAYERPUBLISHERSERVICE_H
