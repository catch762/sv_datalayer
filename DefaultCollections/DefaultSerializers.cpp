#include "DefaultSerializers.h"
#include "SerializationSystem.h"
// Note, that double, bool and QString are the only, exceptional, basic types
// that dont save their type names to JSON.

void DefaultSerializers::RegisterEverything(SerializationSystem *systemInstance)
{
    systemInstance->registerSerialization<double> (double_ser,  double_deser);
    systemInstance->registerSerialization<bool>   (bool_ser,    bool_deser);
    systemInstance->registerSerialization<QString>(qstring_ser, qstring_deser);
}

QJsonValue DefaultSerializers::double_ser(const double &v)
{
    return QJsonValue(v);
}

doubleOpt DefaultSerializers::double_deser(const QJsonValue &json)
{
    return json.isDouble() ? json.toDouble() : doubleOpt();
}

QJsonValue DefaultSerializers::bool_ser(const bool &v)
{
    return QJsonValue(v);
}

boolOpt DefaultSerializers::bool_deser(const QJsonValue &json)
{
    return json.isBool() ? json.toBool() : boolOpt();
}

QJsonValue DefaultSerializers::qstring_ser(const QString &v)
{
    return QJsonValue(v);
}

QStringOpt DefaultSerializers::qstring_deser(const QJsonValue &json)
{
    return json.isString() ? json.toString() : QStringOpt();
}
