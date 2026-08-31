/**
 * @file CUSTOM_MESSAGE.h
 * @brief Abstract base class for telemetry network packet handlers.
 */

#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#include <QObject>
#include <QByteArray>
#include <QString>

namespace GISApp::Core::Services {

/**
 * @class CUSTOM_MESSAGE
 * @brief Abstract interface defining telemetry packet processing, database persistence, layer sync, and signals.
 */
class CUSTOM_MESSAGE : public QObject {
    Q_OBJECT

public:
    explicit CUSTOM_MESSAGE(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~CUSTOM_MESSAGE() = default;

    // Unique Message Identifier
    virtual uint32_t messageId() const = 0;

    // Pure Virtual Handlers
    virtual bool parseAndSaveToDb(const QByteArray &payload) = 0;
    virtual bool updateLayer() = 0;
    virtual bool updateTable() = 0;

signals:
    void messageProcessed(int entityCount);
    void layerUpdated(const QString &layerName, const QString &geoJsonPath);
    void tableDataUpdated(const QString &tableName);
};

} // namespace GISApp::Core::Services

#endif // CUSTOM_MESSAGE_H
