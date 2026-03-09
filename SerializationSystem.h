#include "sv_qtcommon.h"
#include <boost/bimap.hpp>

//**************************************************************************************************************
//
// System for serializing/deserializing arbitrary types.
//
// Usage: 	
//	- to register your type simply call: 
//		SerializationSystem::instance().registerSerialization(serializerFunc, deserializerFunc)
// 	- then when variable of this type is wrapped in QVariant, you can call:
// 		qVariantToJson()
//		jsonToQVariant() for the opposite
//
// Requirements:	 
//	- the type must be registered and named, as checked by qtTypeIsRegisteredAndNamed() assert
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

	static SerializationSystem& instance()
	{
		static SerializationSystem s;
		return s;
	}

    template<class T>
	void registerSerialization(QVariantToJsonFunc serializer, JsonToQVariantFunc deserializer)
	{
        static_assert(qtTypeIsRegisteredAndNamed<T>());

		serializerEntries.insert( {qtTypeId<T>(), qtTypeName<T>(), SerializerEntry{serializer, deserializer} } );
	}

	//convenience function, that takes serializer/deserializer with concrete type,
	//wraps them in QVariant and calls version above.
	template<class T>
	void registerSerialization(std::function<QJsonValue(const T&)> serializer, std::function<T(QJsonValue)> deserializer)
	{
		auto wrappedSerializer = [serializer](const QVariant& val)->QJsonValue
		{
            SV_ASSERT(qtTypeId<T> == val.typeId());
			return serializer(val.value<T>());
		};

		auto wrappedDeserializer = [deserializer](const QJsonValue& json)->QVariant
		{
			return deserializer(json);
		};

		registerSerialization<T>(wrappedSerializer, wrappedDeserializer);
	}


	QJsonValue qVariantToJson(const QVariant& val)
	{
		if (auto * entry = getSerializerByIndex(val.typeId()))
		{
			return entry->serializer(val);
		}

        SV_ERROR("Serialization", QString("Couldnt find serializers for QVariant with type [%1][%2]")
                                    .arg(val.typeId()).arg(val.typeName()).toStdString());
		return QJsonValue();
	}

	//todo write about type and how its not needed for double bool qstring
	QVariant jsonToQVariant(const QJsonValue& json)
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
			logJsonErr(QString("its something unsupported").arg(TypeFieldKey));
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

	

	const SerializerEntry* getSerializerByIndex(QtTypeIndex id)
	{
		auto found = serializersAsTypeindexMap().find(id);
		if (found != serializersAsTypeindexMap().end()) return &found->info;
		else return nullptr;
	}

	const SerializerEntry* getSerializerByTypeName(QString typeName)
	{
		auto found = serializersAsQStringMap().find(typeName);
		if (found != serializersAsQStringMap().end()) return &found->info;
		else return nullptr;
	}

private:
	/*Serializers()
	{
		DefaultSerializers::Load(this);
		UserSerializers::Load(this);
	}
	Serializers(const Serializers&) {}*/

	TwoKeysOneValSerializersMap::map_by<QStringTag>::type& serializersAsQStringMap()
	{
		return serializerEntries.by<QStringTag>();
	}
	TwoKeysOneValSerializersMap::map_by<QtTypeIndexTag>::type& serializersAsTypeindexMap()
	{
		return serializerEntries.by<QtTypeIndexTag>();
	}  

private:
	TwoKeysOneValSerializersMap serializerEntries;
};