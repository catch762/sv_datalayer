#include "DataLayerUtils.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "SerializationLogic/SerializationSystem.h"
#include <functional>



template<typename T>
void checkSerializeAndDeserialize(T val, std::function<QString(const T&)> valToString)
{
    SV_LOG( format("****** Testing type <{}>", qtTypeName<T>().toStdString()) );

    SV_LOG(std::format("Input: {}", val));

    auto jsonVal                = SerializationSystem::instance().qVariantToJson(QVariant::fromValue(val));
    auto reconstructedVariant   = SerializationSystem::instance().jsonToQVariant(jsonVal);

    if (reconstructedVariant.isValid())
    {
        auto reconstructedVal       = reconstructedVariant.template value<T>();

        SV_LOG(std::format("Output: {}", reconstructedVal));
    }
    else SV_LOG("Empty QVariant was returned by jsonToQVariant");
    
    SV_LOG("Was serialized as " + jsonValueToString(jsonVal).toStdString());
}

template <typename T>
QString baseToQString(const T& val)
{
    return QString("%1").arg(val);
}

void AdhocTesting::runTest()
{
    checkSerializeAndDeserialize(true,              baseToQString<bool>);
    checkSerializeAndDeserialize(QString("kek"),    baseToQString<QString>);
    checkSerializeAndDeserialize(5.0,               baseToQString<double>);

    LimitedDouble limd(0.5, 0, 1);
    checkSerializeAndDeserialize<LimitedDouble>(limd, [](const LimitedDouble& v){return v.toString();});


    LimitedDoubleVec limd_vec = { LimitedDouble{0, -1, 1}, LimitedDouble{}, LimitedDouble(99, 0, 100) };
    checkSerializeAndDeserialize<LimitedDoubleVec>(limd_vec, [](const LimitedDoubleVec& v){
        QString res = "LimitedDoubleVec{ ";
        for (auto val : v) res += val.toString() + ", ";
        res += "}";
        return res;
    });
}
