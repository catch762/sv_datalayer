#pragma once
#include "sv_qtcommon.h"
#include "Interpolation/InterpolationInterface.h"

//*****************************************************************************************
//
// The idea is simple: if we have type T registered in InterpolationSystem,
// surely we can have templated interpolator for container
// types such as std::vector<T> so we dont have to write them every time.
//
// (Same idea as in ContainerSerializers.h)
//
// However, for every ConcreteType you will use with the container (lets say its a vector):
//  a) you still need to Q_DECLARE_METATYPE(std::vector<ConcreteType>)
//  b) you still need to register 'std::vector<ConcreteType>' in InterpolationSystem
//     passing interpolator from this ContainerSerializers class.
//
//*****************************************************************************************

/*
template<Serializable T>
class Serializer< std::vector<T> >
{
public:
    using VectorT = std::vector<T>;

	QJsonValue toJson(const VectorT& vec)
    {
        SV_ASSERT(qtTypeIsRegisteredAndNamed<T>()); //not really needed
        SV_ASSERT(qtTypeIsRegisteredAndNamed<VectorT>()); //this IS needed, because im adding typeName in next line

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
        SV_ASSERT(qtTypeIsRegisteredAndNamed<T>()); //not really needed
        SV_ASSERT(qtTypeIsRegisteredAndNamed<VectorT>()); //not really needed

        const QString err = "Error deserializing " + qtTypeName<VectorT>();

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

*/