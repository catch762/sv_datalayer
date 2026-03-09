#include "sv_qtcommon.h"

class SerializationSystem
{
public:
    using QVariantToJsonFunc = std::function<QJsonValue(const QVariant&)>;
	using JsonToQVariantFunc = std::function<QVariant(const QJsonValue&)>;

    struct Entry
	{
		QVariantToJsonFunc serializer;
		JsonToQVariantFunc deserializer;
	};

	static SerializationSystem& instance()
	{
		static SerializationSystem s;
		return s;
	}

    template<class T>
	void registerSerialization(QVariantToJsonFunc serializer, JsonToQVariantFunc deserializer)
	{
        static_assert(qtTypeIsRegistered<T>());

		entries[qtTypeId<T>()] = { s, d };
	}

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
		if (auto * entry = get(val.typeId()))
		{
			return entry->serializer(val);
		}

        SV_ERROR("Serialization", QString("Couldnt find serializers for QVariant with type [%1][%2]")
                                    .arg(val.typeId()).arg(val.typeName).toStdString());
		return QJsonValue();
	}

	QVariant jsonToQVariant(const QJsonValue& json)
	{
		QString type_name;
		if (json.isDouble())
		{
			type_name = "double";
		}
		else if (json.isBool())
		{
			type_name = "bool";
		}
		else if (json.isString())
		{
			type_name = "QString";
		}
		else if (json.isObject())
		{
			if (json["type"].isString())
			{
				type_name = json["type"].toString();
			}
			else type_name = "AnyMap";
		}
		else
		{
			OnError("json_to_any: something currently unsupported happened");
			return std::any();
		}

		if (auto * e = instance().get(type_name))
		{
			return e->deserializer(json);
		}
		else
		{
			OnError("json_to_any: no deserializer for " + type_name);
			return std::any();
		}
	}

	

	const Entry* get(QtTypeIndex id)
	{
		auto found = entries.find(id);
		if (found != entries.end()) return &found->second;
		else return nullptr;
	}

private:
	Serializers()
	{
		DefaultSerializers::Load(this);
		UserSerializers::Load(this);
	}
	Serializers(const Serializers&) {}

private:
	std::map<QtTypeIndex, Entry> entries;
};