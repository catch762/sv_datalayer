#pragma once
#include "sv_qtcommon.h"
#include "SerializationSystem.h"

//*****************************************************************************************
//
// The idea is simple: if we have type T registered in SerializationSystem,
// surely we can have templated serializer and deserializer for container
// types such as std::vector<T> so we dont have to write them every time.
//
// However, for every ConcreteType you will use with the container (lets say its a vector):
//  a) you still need to Q_DECLARE_METATYPE(std::vector<ConcreteType>)
//  b) you still need to register 'std::vector<ConcreteType>' in SerializationSystem
//     passing serializers from this ContainerSerializers class.
//
//*****************************************************************************************
class ContainerSerializers
{
public:
    template<typename T>
	static QJsonValue 					 vector_toJson	(const std::vector<T>& vec)
    {
        using VectorT = std::vector<T>;
        SV_ASSERT(qtTypeIsRegisteredAndNamed<T>());
        SV_ASSERT(qtTypeIsRegisteredAndNamed<VectorT>());

        QJsonObject json;
        addTypeFieldToJson<VectorT>(json);

        QJsonArray jsonValues;
        for (auto value : vec)
        {
            jsonValues.append(SerializationSystem::instance().toJson(value));
        }

        json[ValuesKey] = jsonValues;

        return json;
    }

    template<typename T>
	static std::optional<std::vector<T>> vector_fromJson(const QJsonValue& jsonValue)
    {
        using VectorT = std::vector<T>;
        SV_ASSERT(qtTypeIsRegisteredAndNamed<T>());
        SV_ASSERT(qtTypeIsRegisteredAndNamed<VectorT>());

        const QString err = "Error deserializing " + qtTypeName<VectorT>();

        auto json = convertJsonAndLogError<QJsonObject>(jsonValue, err);
        if (!json) return {};

        auto valuesArr = getFromJsonAndLogError<QJsonArray>(*json, ValuesKey, err);
        if (!valuesArr) return {};

        VectorT vector;
        vector.reserve(valuesArr->size());

        for (auto value : *valuesArr)
        {
            if (auto optT = SerializationSystem::instance().fromJson<T>(value))
            {
                vector.push_back(*optT);
            }
            else
            {
                SV_ERROR((err + ": failed to deserialize array element").toStdString());
                return {};
            }
        }

        return vector;
    }

private:
    static inline const auto ValuesKey = "values";
};