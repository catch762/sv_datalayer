#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>

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
// 	- the resulting JSON object must have its 'qtTypeName<T>()' saved by key 'SerializationSystem::TypeFieldKey'
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

	static inline const QString TypeFieldKey = "_type";

	static SerializationSystem& instance();


    template<class T>
	void registerSerialization(QVariantToJsonFunc serializer, JsonToQVariantFunc deserializer);

	//convenience function, that takes serializer/deserializer with concrete type,
	//wraps them in QVariant and calls version above.
	template<class T>
	void registerSerialization(std::function<QJsonValue(const T&)> serializer, std::function<T(QJsonValue)> deserializer);


	QJsonValue qVariantToJson(const QVariant& val);

	//todo write about type and how its not needed for double bool qstring
	QVariant jsonToQVariant(const QJsonValue& json);
	

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
void SerializationSystem::registerSerialization(std::function<QJsonValue(const T&)> serializer, std::function<T(QJsonValue)> deserializer)
{
	auto wrappedSerializer = [serializer](const QVariant& val)->QJsonValue
	{
		SV_ASSERT(qtTypeId<T>() == val.typeId());
		return serializer(val.value<T>());
	};

	auto wrappedDeserializer = [deserializer](const QJsonValue& json)->QVariant
	{
		return deserializer(json);
	};

	registerSerialization<T>(wrappedSerializer, wrappedDeserializer);
}