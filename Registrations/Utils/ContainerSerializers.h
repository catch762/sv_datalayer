#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializationSystem.h"
#include "SerializationLogic/SerializerInterface.h"

//*****************************************************************************************
//
// The idea is simple: if we have type T registered in SerializationSystem,
// surely we can have templated serializer and deserializer for container
// types such as std::vector<T> so we dont have to write them every time.
//
// (Same idea as in ContainerInterpolators.h)
//
// However, for every ConcreteType you will use with the container (lets say its a vector):
//  a) you still need to Q_DECLARE_METATYPE(std::vector<ConcreteType>)
//  b) you still need to register 'std::vector<ConcreteType>' in SerializationSystem
//     passing serializers from this ContainerSerializers class.
//
//*****************************************************************************************

//Note: in this impl, we ARE adding info that it is a vector<T>, so that we can find
//appropriate serializer in runtime
template<Serializable T>
class Serializer< std::vector<T> >
{
public:
    using VectorT = std::vector<T>;

	QJsonValue toJson(const VectorT& vec)
    {
        SV_ASSERT(typeIsNamed<T>()); //not really needed
        SV_ASSERT(typeIsNamed<VectorT>()); //this IS needed, because im adding typeName in next line

        QJsonObject json;
        addTypeFieldToJson<VectorT>(json);

        QJsonArray jsonValues;
        for (auto value : vec)
        {
            jsonValues.append( Serializer<T>().toJson(value) );
        }

        json[ValuesKey] = jsonValues;

        return json;
    }

	std::optional<VectorT> fromJson(const QJsonValue& jsonValue)
    {
        SV_ASSERT(typeIsNamed<T>()); //not really needed
        SV_ASSERT(typeIsNamed<VectorT>()); //not really needed

        const QString err = "Error deserializing " + QString(typeName<VectorT>());

        auto json = convertJsonAndLogError<QJsonObject>(jsonValue, err);
        if (!json) return {};

        auto jsonValuesArr = getFromJsonAndLogError<QJsonArray>(*json, ValuesKey, err);
        if (!jsonValuesArr) return {};

        VectorT vector;
        vector.reserve(jsonValuesArr->size());

        for (auto jsonValue : *jsonValuesArr)
        {
            if (auto optT = Serializer<T>().fromJson(jsonValue) )
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

//Unlike in case with vector above, we are not adding type information
template<Serializable T>
class Serializer< std::set<T> >
{
public:
    using SetT = std::set<T>;

	QJsonValue toJson(const SetT& set)
    {
        QJsonArray jsonValues;
        for (auto value : set)
        {
            jsonValues.append( Serializer<T>().toJson(value) );
        }

        return jsonValues;
    }

	std::optional<SetT> fromJson(const QJsonValue& jsonValue)
    {
        const QString err = "Error deserializing " + QString(typeNameOrMangled<SetT>());

        auto jsonValuesArr = convertJsonAndLogError<QJsonArray>(jsonValue, err);
        if (!jsonValuesArr) return {};

        SetT set;

        for (auto jsonValue : *jsonValuesArr)
        {
            if (auto optT = Serializer<T>().fromJson(jsonValue) )
            {
                set.insert(*optT);
            }
            else
            {
                SV_ERROR((err + ": failed to deserialize set element").toStdString());
                return {};
            }
        }

        return set;
    }
};