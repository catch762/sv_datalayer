#include "SerializationSystem.h"

#include "Registrations/DefaultSerializers.h"

SerializationSystem &SerializationSystem::instance()
{
    static SerializationSystem s;
    return s;
}

QJsonValue SerializationSystem::anyToJson(const std::any& any, bool logOnError)
{
    if (auto * entry = getSerializerByIndex(typeIndex(any)))
    {
        return entry->serializer(any);
    }

    if (logOnError)
    {
        SV_ERROR(std::format("Couldnt find serializers for {}", any));
    }
    
    return QJsonValue();
}

std::any SerializationSystem::jsonToAny(const QJsonValue &json)
{
    auto logJsonErr = [&](const QString &err)
    {
        SV_ERROR("Serialization", QString("Error trying to deserialize JSON: %1. Json: %2")
                                        .arg(err).arg(jsonValueToString(json)).toStdString());
    };

    QString typeName;
    if (json.isDouble())
    {
        typeName = "double";
    }
    else if (json.isBool())
    {
        typeName = "bool";
    }
    else if (json.isString())
    {
        typeName = "QString";
    }
    else if (json.isObject())
    {
        if (json[TypeFieldKey].isString())
        {
            typeName = json[TypeFieldKey].toString();
        }
        else
        {
            logJsonErr(QString("its object, but it doesnt have [%1] field").arg(TypeFieldKey));
            return QVariant();
        }
    }
    else
    {
        logJsonErr(QString("its something unsupported"));
        return QVariant();
    }

    if (auto * e = instance().getSerializerByTypeName(typeName))
    {
        return e->deserializer(json);
    }
    else
    {
        //OnError("json_to_any: no deserializer for " + type_name);
        SV_ERROR("Serialization", "No deserializer found for typeName=" + typeName.toStdString());
        return QVariant();
    }
}

const SerializationSystem::SerializerEntry *SerializationSystem::getSerializerByIndex(std::type_index id)
{
    return serializerEntries.getEntry(id);
}

const SerializationSystem::SerializerEntry *SerializationSystem::getSerializerByTypeName(QString typeName)
{
    return serializerEntries.getEntry(typeName);
}