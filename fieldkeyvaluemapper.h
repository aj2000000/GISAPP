#ifndef FIELDKEYVALUEMAPPER_H
#define FIELDKEYVALUEMAPPER_H

#include <QString>
#include <QVariant>
#include "IRS.h"

/**
 * @class FieldKeyValueMapper
 * @brief Singleton class to map system numerical/enum keys to human-readable string values for display in tables, detail views, and map popups.
 */
class FieldKeyValueMapper
{
public:
    static FieldKeyValueMapper& instance();

    // Track identity mapping (e.g., 1 -> "HOSTILE", 2 -> "FRIENDLY", 3 -> "NEUTRAL")
    QString trackIdentityMapping(int identity);
    QString trackIdentityMapping(IDENTITY identity);

    // System track type mapping (e.g., 1 -> "SYSTEM1", 2 -> "SYSTEM2 / FUSED")
    QString systemTrackTypeMapping(int type);
    QString systemTrackTypeMapping(SYSTEM_TRACK_TYPE type);

    // Track source mapping (e.g., 1 -> "SOURCE1", 2 -> "SOURCE2")
    QString trackSourceMapping(int source);
    QString trackSourceMapping(TRACK_SOURCE source);

    // Message precedence mapping
    QString precedenceMapping(int precedence);

    // Health status mapping
    QString healthStatusMapping(int health);

    // Generic key-value mapper for UI display
    QString mapValue(const QString &fieldKey, const QVariant &value);

private:
    FieldKeyValueMapper();
    ~FieldKeyValueMapper() = default;

    FieldKeyValueMapper(const FieldKeyValueMapper&) = delete;
    FieldKeyValueMapper& operator=(const FieldKeyValueMapper&) = delete;
};

#endif // FIELDKEYVALUEMAPPER_H
