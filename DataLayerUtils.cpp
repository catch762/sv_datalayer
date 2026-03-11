#include "DataLayerUtils.h"
#include "TypesAndWidgets/LimitedValue.h"
#include "SerializationSystem.h"
#include <functional>

template<typename T>
void checkSerializeAndDeserialize(T val, std::function<QString(T)> valToString)
{
    SV_LOG( format("****** Testing type <{}>", qtTypeName<T>().toStdString()) );

    SV_LOG("Input: " + valToString(val).toStdString());

    auto jsonVal                = SerializationSystem::instance().qVariantToJson(QVariant::fromValue(val));
    auto reconstructedVariant   = SerializationSystem::instance().jsonToQVariant(jsonVal);

    if (reconstructedVariant.isValid())
    {
        auto reconstructedVal       = reconstructedVariant.template value<T>();

        SV_LOG("Output: " + valToString(reconstructedVal).toStdString());
    }
    else SV_LOG("Empty QVariant was returned by jsonToQVariant");
    
    SV_LOG("Was serialized as " + jsonValueToString(jsonVal).toStdString());
}

template <typename T>
QString baseToQString(T val)
{
    return QString("%1").arg(val);
}

void AdhocTesting::runTest()
{
    checkSerializeAndDeserialize(true,              baseToQString<bool>);
    checkSerializeAndDeserialize(QString("kek"),    baseToQString<QString>);
    checkSerializeAndDeserialize(5.0,               baseToQString<double>);

    LimitedDouble limd(0.5, 0, 1);

    checkSerializeAndDeserialize<LimitedDouble>(limd, [](LimitedDouble v){return v.toString();});

    //sereializeAndDeserialize(QVariant::fromValue(limd), "limiteddouble");
}
