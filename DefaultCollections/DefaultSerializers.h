#pragma once
#include <QJsonValue>
#include "sv_qtcommon.h"
#include "SerializerInterface.h"

class SerializationSystem;
class DefaultSerializers
{
public:
	static void Register(SerializationSystem *systemInstance);
};

// Note, that double, bool and QString are the only, exceptional, basic types
// that dont save their type names to JSON.

template <>
class Serializer<double>
{
public:
    static QJsonValue toJson(const double& value)
    {
        return QJsonValue(value);
    }
    static std::optional<double> fromJson(const QJsonValue& json)
    {
        return json.isDouble() ? json.toDouble() : doubleOpt();
    }
};

template <>
class Serializer<bool>
{
public:
    static QJsonValue toJson(const bool& value)
    {
        return QJsonValue(value);
    }
    static std::optional<bool> fromJson(const QJsonValue& json)
    {
        return json.isBool() ? json.toBool() : boolOpt();
    }
};

template <>
class Serializer<QString>
{
public:
    static QJsonValue toJson(const QString& value)
    {
        return QJsonValue(value);
    }
    static std::optional<QString> fromJson(const QJsonValue& json)
    {
        return json.isString() ? json.toString() : QStringOpt();
    }
};

