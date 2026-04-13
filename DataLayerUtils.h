#pragma once
#include "sv_qtcommon.h"
#include <QPointer>



//UI logic often calculates 'span=max-min' or something, so we have to tone down values to avoid overflow.
static constexpr auto MinFloatInUI  = std::numeric_limits<float>::lowest() / 10.0f;
static constexpr auto MaxFloatInUI  = std::numeric_limits<float>::max() / 10.0f;
static constexpr auto MinIntInUI    = std::numeric_limits<int>::lowest() / 10;
static constexpr auto MaxIntInUI    = std::numeric_limits<int>::max() / 10;



class AdhocTesting
{
public:
    static void runTest();
};
