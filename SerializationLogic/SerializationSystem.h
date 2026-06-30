#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>
#include "WidgetLogic/WidgetDefs.h"
#include "SerializationLogic/SerializerInterface.h"
#include "DataForTypeMap.h"

#include "TypeNames.h"

//**************************************************************************************************************
//
// System for serializing/deserializing arbitrary types.
//
// Usage: 	
//
//	- to register your type simply call either variant of: 
//		SerializationSystem::instance().registerSerialization()
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
// SerializationSystem comes with some types already registered: thats because ---REMOVE
// in its constructor it loads registrations from DefaultSerializers class.
// You may want to look into it, and load your own serializers collection in a simillar fashion.
//
//**************************************************************************************************************

class SerializationSystem
{
public:
    using AnyToJsonFunc = std::function<QJsonValue(const std::any&)>;
	using JsonToAnyFunc = std::function<std::any(const QJsonValue&)>;

    struct SerializerEntry
	{
		AnyToJsonFunc 	serializer;
		JsonToAnyFunc 	deserializer;
	};

	//using SerializerMap = DataForTypeMap<SerializerEntry>;

	
	//If has defined Serializer<T>, just call this
	template <Serializable T>
	void registerSerialization();
	
	
	//todo its bad cause qvariant also fits
	//convenience function, wraps in QVariant and calls 'qVariantToJson'
	template<class T>
	QJsonValue toJson(const T& value);
	
	QJsonValue anyToJson(const QVariant& val, bool logOnError = false);
	
	//todo write about type and how its not needed for double bool qstring
	std::any jsonToAny(const QJsonValue& json);
	
	template<class T>
	std::optional<T> fromJson(const QJsonValue& json);
	
	static SerializationSystem& instance();

private:
	SerializationSystem() = default;
	DISABLE_COPY_AND_ASSIGNMENT(SerializationSystem);
	
	const SerializerEntry* getSerializerByIndex(std::type_index id);
	const SerializerEntry* getSerializerByTypeName(QString typeName);
	
	// This could ve been public method too, but i feel like leaving only one way to do things -
	// the other, and the only one, public 'registerSerialization()'
    template<class T>
	void registerSerialization(AnyToJsonFunc serializer, JsonToAnyFunc deserializer);

private:
	DataForTypeMap<SerializerEntry> serializerEntries;
};



template<class T>
void SerializationSystem::registerSerialization(AnyToJsonFunc serializer, JsonToAnyFunc deserializer)
{
	SV_ASSERT(typeIsNamed<T>());

	serializerEntries.addEntryForType( typeIndex<T>(), typeName<T>(), SerializerEntry{serializer, deserializer} );
}

template<class T>
QJsonValue SerializationSystem::toJson(const T& value)
{
	static_assert(!std::is_same_v<T, std::any>, "You dont pass std::any here, only concrete type");

	return anyToJson(std::any(value));
}

template<class T>
std::optional<T> SerializationSystem::fromJson(const QJsonValue& json)
{
	auto any = jsonToAny(json);
	if (!any.has_value()) return {};

	if (!anyHoldsType<T>())
	{
		SV_ERROR(std::format("SerializationSystem::fromJson, while trying to deserialize type {}"
			  				 " received mismatching result {}", typeName<T>(), any));
		return {};
	}

	return anyGetOpt<T>(any);
}

template <Serializable T>
void SerializationSystem::registerSerialization()
{
	auto wrappedSerializer = [](const std::any& any)->QJsonValue
	{
		SV_ASSERT(anyHoldsType<T>(any));
		return Serializer<T>().toJson(*anyGet<T>(any));
	};

	auto wrappedDeserializer = [](const QJsonValue& json)->std::any
	{
		if (auto valueOpt = Serializer<T>().fromJson(json))
		{
			return std::any(*valueOpt);
		}
		else return std::any();
	};

	registerSerialization<T>(wrappedSerializer, wrappedDeserializer);
}