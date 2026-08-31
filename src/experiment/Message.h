/**
 * @file Message.h
 * @brief Communication message interpreter (ExpMessage) for processing UDP telemetry packets.
 * @note ExpMessage does not derive from CUSTOM_MESSAGE.
 */

#ifndef EXP_MESSAGE_H
#define EXP_MESSAGE_H

#include "Entity.h"
#include "Table.h"
#include "Layer.h"
#include "MessageId.h"
#include "Structures.h"
#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QDebug>
#include <cstring>

#pragma pack(push, 1)
struct ExpMessageHeader {
    int32_t numberOfEntity = 0;
};
#pragma pack(pop)

namespace GISApp::Experiment {

class ExpMessage : public QObject {
    Q_OBJECT

public:
    ExpMessageHeader msgHeader;
    QVector<ExpEntity> entityList;
    ExpTable *table = nullptr;

    explicit ExpMessage(ExpTable *targetTable = nullptr, QObject *parent = nullptr)
        : QObject(parent), table(targetTable) {}

    virtual ~ExpMessage() = default;

    uint32_t messageId() const { return EXP_MESSAGE_ID; }

    void setTable(ExpTable *targetTable) {
        table = targetTable;
    }

    bool parseAndSaveToDb(const QByteArray &rawPayload) {
        QByteArray payload = rawPayload;

        // Strip system UDP message header if present
        if (static_cast<size_t>(payload.size()) >= sizeof(STRUCT_MESSAGE_HEADER)) {
            STRUCT_MESSAGE_HEADER sysHeader;
            std::memcpy(&sysHeader, payload.constData(), sizeof(STRUCT_MESSAGE_HEADER));
            if (sysHeader.message_id == EXP_MESSAGE_ID) {
                payload = payload.mid(sizeof(STRUCT_MESSAGE_HEADER));
            }
        }

        if (static_cast<size_t>(payload.size()) < sizeof(ExpMessageHeader)) {
            qWarning() << "[ExpMessage] Payload too small for header:" << payload.size();
            return false;
        }

        std::memcpy(&msgHeader, payload.constData(), sizeof(ExpMessageHeader));
        qDebug() << "[ExpMessage] 🔍 Ingested numberOfEntity:" << msgHeader.numberOfEntity << "| Payload size:" << payload.size();

        entityList.clear();
        int offset = sizeof(ExpMessageHeader);
        int expectedSize = offset + (msgHeader.numberOfEntity * sizeof(ExpEntityStruct));

        if (payload.size() < expectedSize) {
            qWarning() << "[ExpMessage] Truncated payload size:" << payload.size() << "expected:" << expectedSize;
            return false;
        }

        for (int i = 0; i < msgHeader.numberOfEntity; ++i) {
            ExpEntityStruct rawStruct;
            std::memcpy(&rawStruct, payload.constData() + offset, sizeof(ExpEntityStruct));
            ExpEntity entity(rawStruct, "Experiment");
            entityList.append(entity);
            offset += sizeof(ExpEntityStruct);
        }

        qDebug() << "[ExpMessage] 📥 Successfully parsed batch of" << entityList.size() << "entities.";

        if (table) {
            table->addOrUpdateEntities(entityList);
        } else {
            qWarning() << "[ExpMessage] No ExpTable attached to save entities!";
        }

        emit messageProcessed(entityList.size());
        return true;
    }

signals:
    void messageProcessed(int count);
};

} // namespace GISApp::Experiment

using ExpMessage = GISApp::Experiment::ExpMessage;

#endif // EXP_MESSAGE_H
