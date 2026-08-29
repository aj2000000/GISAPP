#include "fieldkeyvaluemapper.h"

FieldKeyValueMapper& FieldKeyValueMapper::instance()
{
    static FieldKeyValueMapper instance;
    return instance;
}

FieldKeyValueMapper::FieldKeyValueMapper() {}

QString FieldKeyValueMapper::trackIdentityMapping(IDENTITY identity)
{
    return trackIdentityMapping(static_cast<int>(identity));
}

QString FieldKeyValueMapper::trackIdentityMapping(int identity)
{
    switch (identity) {
        case HOSTILE:   // 1
            return "HOSTILE";
        case FRIENDLY:  // 2
            return "FRIENDLY";
        case 3:
            return "NEUTRAL";
        default:
            return "UNKNOWN";
    }
}

QString FieldKeyValueMapper::systemTrackTypeMapping(SYSTEM_TRACK_TYPE type)
{
    return systemTrackTypeMapping(static_cast<int>(type));
}

QString FieldKeyValueMapper::systemTrackTypeMapping(int type)
{
    switch (type) {
        case SYSTEM1: // 1
            return "SYSTEM1";
        case SYSTEM2: // 2 (or FUSED)
            return "SYSTEM2 / FUSED";
        default:
            return "UNKNOWN";
    }
}

QString FieldKeyValueMapper::trackSourceMapping(TRACK_SOURCE source)
{
    return trackSourceMapping(static_cast<int>(source));
}

QString FieldKeyValueMapper::trackSourceMapping(int source)
{
    switch (source) {
        case SOURCE1: // 1
            return "SOURCE1";
        case SOURCE2: // 2
            return "SOURCE2";
        default:
            return "UNKNOWN";
    }
}

QString FieldKeyValueMapper::precedenceMapping(int precedence)
{
    switch (precedence) {
        case PRECEDENCE_NONE:                  return "NONE";
        case PRECEDENCE_FLASH:                 return "FLASH";
        case PRECEDENCE_EMERGENCY:             return "EMERGENCY";
        case PRECEDENCE_OPERATIONAL_IMMEDIATE: return "OPERATIONAL IMMEDIATE";
        case PRECEDENCE_PRIORITY:              return "PRIORITY";
        case PRECEDENCE_ROUTINE:               return "ROUTINE";
        case PRECEDENCE_DEFFERED:              return "DEFERRED";
        default:                               return "UNKNOWN";
    }
}

QString FieldKeyValueMapper::healthStatusMapping(int health)
{
    switch (health) {
        case HEALTH_STATUS_RED:   return "RED (CRITICAL)";
        case HEALTH_STATUS_GREEN: return "GREEN (HEALTHY)";
        default:                  return "UNKNOWN";
    }
}

QString FieldKeyValueMapper::mapValue(const QString &fieldKey, const QVariant &value)
{
    QString key = fieldKey.toLower().trimmed();
    bool ok = false;
    int intVal = value.toInt(&ok);

    if (key == "identity" || key == "trackidentity") {
        return ok ? trackIdentityMapping(intVal) : value.toString();
    } else if (key == "systemtype" || key == "systemtracktype" || key == "tracksystemtype") {
        return ok ? systemTrackTypeMapping(intVal) : value.toString();
    } else if (key == "sources" || key == "tracksource" || key == "tracksources") {
        return ok ? trackSourceMapping(intVal) : value.toString();
    } else if (key == "precedence") {
        return ok ? precedenceMapping(intVal) : value.toString();
    } else if (key == "health" || key == "healthstatus") {
        return ok ? healthStatusMapping(intVal) : value.toString();
    }

    return value.toString();
}
