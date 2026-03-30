#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>
#include "DataLayerUtils.h"
#include "SerializerInterface.h"

//**************************************************************************************************************
//
// System for serializing/deserializing arbitrary types.
//
// Usage: 	
//
//	- to register your type simply call: 
//		SerializationSystem::instance().registerSerialization(serializerFunc, deserializerFunc)
//
// 	- then when variable of this type is wrapped in QVariant, you can call:
// 		qVariantToJson()
//		jsonToQVariant() for the opposite
//
// Requirements:
//	 
//	- the type must be registered and named, as checked by qtTypeIsRegisteredAndNamed() assert
//
// 	- the resulting JSON object must have its 'qtTypeName<T>()' saved by key 'TypeFieldKey' (defined in utils)
//	  Thats how SerializationSystem knows which deserializer to pick when it receives JSON.
//	  (just FYI raw 'double', 'bool' and 'QString' types dont save it, but its the only exception)
//
// SerializationSystem comes with some types already registered: thats because
// in its constructor it loads registrations from DefaultSerializers class.
// You may want to look into it, and load your own serializers collection in a simillar fashion.
//
//**************************************************************************************************************
class SerializationSystem
{
public:
    using QVariantToJsonFunc = std::function<QJsonValue(const QVariant&)>;
	using JsonToQVariantFunc = std::function<QVariant(const QJsonValue&)>;

    struct SerializerEntry
	{
		QVariantToJsonFunc serializer;
		JsonToQVariantFunc deserializer;
	};

	using TwoKeysOneValSerializersMap = boost::bimaps::bimap<
		boost::bimaps::tagged<QtTypeIndex, 	struct QtTypeIndexTag>,
		boost::bimaps::tagged<QString, 		struct QStringTag>,
		boost::bimaps::with_info<SerializerEntry>
	>;

	static SerializationSystem& instance();


	template <Serializable T>
	void registerSerialization();

    template<class T>
	void registerSerialization(QVariantToJsonFunc serializer, JsonToQVariantFunc deserializer);

	//convenience function, that takes serializer/deserializer with concrete type,
	//wraps them in QVariant and calls version above.
	template<class T>
	void registerSerialization(std::function<QJsonValue(const T&)> serializer, std::function<std::optional<T>(QJsonValue)> deserializer);

	//convenience function, wraps in QVariant and calls 'qVariantToJson'
	template<class T>
	QJsonValue toJson(const T& value);

	QJsonValue qVariantToJson(const QVariant& val);

	//todo write about type and how its not needed for double bool qstring
	QVariant jsonToQVariant(const QJsonValue& json);
	
	template<class T>
	std::optional<T> fromJson(const QJsonValue& json);

	const SerializerEntry* getSerializerByIndex(QtTypeIndex id);
	

	const SerializerEntry* getSerializerByTypeName(QString typeName);
	

private:
	SerializationSystem();

	TwoKeysOneValSerializersMap::map_by<QStringTag>::type& serializersAsQStringMap();
	TwoKeysOneValSerializersMap::map_by<QtTypeIndexTag>::type& serializersAsTypeindexMap();

private:
	TwoKeysOneValSerializersMap serializerEntries;
};



template<class T>
void SerializationSystem::registerSerialization(QVariantToJsonFunc serializer, JsonToQVariantFunc deserializer)
{
	SV_ASSERT(qtTypeIsRegisteredAndNamed<T>());

	serializerEntries.insert( {qtTypeId<T>(), qtTypeName<T>(), SerializerEntry{serializer, deserializer} } );
}

template<class T>
void SerializationSystem::registerSerialization(std::function<QJsonValue(const T&)> serializer, std::function<std::optional<T>(QJsonValue)> deserializer)
{
	auto wrappedSerializer = [serializer](const QVariant& val)->QJsonValue
	{
		SV_ASSERT(qtTypeId<T>() == val.typeId());
		return serializer(val.value<T>());
	};

	auto wrappedDeserializer = [deserializer](const QJsonValue& json)->QVariant
	{
		if (auto valueOpt = deserializer(json))
		{
			return QVariant::fromValue(*valueOpt);
		}
		else return QVariant();
	};

	registerSerialization<T>(wrappedSerializer, wrappedDeserializer);
}

template<class T>
QJsonValue SerializationSystem::toJson(const T& value)
{
	return qVariantToJson(QVariant::fromValue(value));
}

template<class T>
std::optional<T> SerializationSystem::fromJson(const QJsonValue& json)
{
	auto qvariant = jsonToQVariant(json);
	if (!qvariant.isValid()) return {};

	if (qvariant.typeId() != qtTypeId<T>())
	{
		SV_ERROR(("SerializationSystem::fromJson, while trying to deserialize type " + qtTypeName<T>()
			  		+ " received mismatching result " + qVariantInfo(qvariant)).toStdString());
		return {};
	}

	return qvariant.value<T>();
}

template <Serializable T>
void SerializationSystem::registerSerialization()
{
	auto wrappedSerializer = [](const QVariant& val)->QJsonValue
	{
		SV_ASSERT(qtTypeId<T>() == val.typeId());
		return Serializer<T>::toJson(val.value<T>());
	};

	auto wrappedDeserializer = [](const QJsonValue& json)->QVariant
	{
		if (auto valueOpt = Serializer<T>::fromJson(json))
		{
			return QVariant::fromValue(*valueOpt);
		}
		else return QVariant();
	};

	registerSerialization<T>(wrappedSerializer, wrappedDeserializer);
}