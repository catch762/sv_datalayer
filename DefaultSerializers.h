#pragma once
#include <QJsonValue>

class SerializationSystem;

class DefaultSerializers
{
public:
	static void RegisterAllDefaultTypes(SerializationSystem *systemInstance);

	static QJsonValue double_ser(const double& v);
	static double double_deser(const QJsonValue& json);

	static QJsonValue bool_ser(const bool& v);
	static bool bool_deser(const QJsonValue& json);

	static QJsonValue qstring_ser(const QString& v);
	static QString qstring_deser(const QJsonValue& json);
};