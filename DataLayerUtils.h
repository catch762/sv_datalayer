#pragma once
#include "sv_qtcommon.h"

static constexpr auto TypeFieldKey = "_type";

static constexpr auto MinFloatInUI  = std::numeric_limits<float>::min() / 10.0f;
static constexpr auto MaxFloatInUI  = std::numeric_limits<float>::max() / 10.0f;
static constexpr auto MinIntInUI    = std::numeric_limits<int>::min() / 10.0f;
static constexpr auto MaxIntInUI    = std::numeric_limits<int>::max() / 10.0f;

template <typename T>
inline void addTypeFieldToJson(QJsonObject &obj)
{
    obj[TypeFieldKey] = qtTypeName<T>();
}

class AdhocTesting
{
public:
    static void runTest();
};