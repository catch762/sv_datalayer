#pragma once
#include <optional>
#include <QJsonValue>

template <typename T>
class Serializer
{
public:
    //You must implement these:

    //static QJsonValue       toJson(const T& value);
    //static std::optional<T> fromJson(const QJsonValue& json);
};


/***************************************************************************

Usage: in header file, after YourType definition, make an impl like that:

template <>
class Serializer<YourType>
{
public:
    static QJsonValue               toJson(const YourType& value) {...}
    static std::optional<YourType>  fromJson(const QJsonValue& json) {...}
}; 

***************************************************************************/


// Concept to check if Serializer is defined for T.
// Currently doesnt ever trigger, because i have default impl already, that
// triggers static_assert.
template<typename T>
concept Serializable = requires(const T& value)
{
    { Serializer<T>::toJson(value) } -> std::same_as<QJsonValue>;
    { Serializer<T>::fromJson(QJsonValue()) } -> std::same_as<std::optional<T>>;
};