#include "DefaultSerializers.h"
#include "SerializationSystem.h"
// Note, that double, bool and QString are the only, exceptional, basic types
// that dont save their type names to JSON.

void DefaultSerializers::RegisterAllDefaultTypes()
{
    SerializationSystem::instance().registerSerialization<double> (double_ser,  double_deser);
    SerializationSystem::instance().registerSerialization<bool>   (bool_ser,    bool_deser);
    SerializationSystem::instance().registerSerialization<QString>(qstring_ser, qstring_deser);
}

QJsonValue DefaultSerializers::double_ser(const double &v)
{
    return QJsonValue(v);
}

double DefaultSerializers::double_deser(const QJsonValue &json)
{
    return json.toDouble();
}

QJsonValue DefaultSerializers::bool_ser(const bool &v)
{
    return QJsonValue(v);
}

bool DefaultSerializers::bool_deser(const QJsonValue &json)
{
    return json.toBool();
}

QJsonValue DefaultSerializers::qstring_ser(const QString &v)
{
    return QJsonValue(v);
}

QString DefaultSerializers::qstring_deser(const QJsonValue &json)
{
    return json.toString();
}
