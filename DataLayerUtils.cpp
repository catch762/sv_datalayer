#include "DataLayerUtils.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "SerializationLogic/SerializationSystem.h"
#include <functional>



template<typename T, typename Func>
void checkSerializeAndDeserialize(T val, Func valToString)
{
    SV_LOG( std::format("****** Testing type <{}>", typeName<T>()) );

    SV_LOG(std::format("Input: {}", val));

    auto jsonVal                = SerializationSystem::instance().anyToJson(std::any(val));
    auto reconstructedAny       = SerializationSystem::instance().jsonToAny(jsonVal);

    if (!reconstructedAny)
    {
        SV_LOG("Empty std::any was returned by jsonToAny");
    }
    else
    {
        if (auto reconstructedVal = anyGet<T>(*reconstructedAny))
        {
            SV_LOG(std::format("Output: {}", *reconstructedVal));
        }
        else SV_LOG("std::any that doesnt match type was returned by jsonToAny");
    }

    SV_LOG("Was serialized as " + jsonValueToString(jsonVal).toStdString());
}

template <typename T>
QString baseToQString(const T& val)
{
    return QString("%1").arg(val);
}
