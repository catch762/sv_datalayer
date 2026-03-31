#pragma once
#include <optional>
#include <QJsonValue>

//*************************************************************************************
//
//  Usage: in header file, after YourType definition, define Serializer<YourType> impl.
//
//*************************************************************************************
template <typename T>
class Serializer
{
public:
    static QJsonValue toJson(const T& value)
    {
        static_assert(false, "you must implement Serializer::toJson for this type");
        SV_UNREACHABLE();
    }
    static std::optional<T> fromJson(const QJsonValue& json)
    {
        static_assert(false, "you must implement Serializer::fromJson for this type");
        SV_UNREACHABLE();
    }
};


// Concept to check if Serializer exists for T.
// Currently doesnt always trigger, because i have default impl in Serializer already, which
// triggers static_assert instead anyway, so its ok. But at least it conveys the message.
template<typename T>
concept Serializable = requires(const T& value)
{
    { Serializer<T>::toJson(value) } -> std::same_as<QJsonValue>;
    { Serializer<T>::fromJson(QJsonValue()) } -> std::same_as<std::optional<T>>;
};