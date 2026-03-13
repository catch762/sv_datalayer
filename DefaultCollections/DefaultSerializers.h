#pragma once
#include <QJsonValue>
#include "sv_qtcommon.h"

class SerializationSystem;

class DefaultSerializers
{
public:
	static void RegisterEverything(SerializationSystem *systemInstance);

	static QJsonValue double_ser(const double& v);
	static doubleOpt double_deser(const QJsonValue& json);

	static QJsonValue bool_ser(const bool& v);
	static boolOpt bool_deser(const QJsonValue& json);

	static QJsonValue qstring_ser(const QString& v);
	static QStringOpt qstring_deser(const QJsonValue& json);
};