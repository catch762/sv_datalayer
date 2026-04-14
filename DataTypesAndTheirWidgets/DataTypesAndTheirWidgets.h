#pragma once
#include "LimitedValue/LimitedValue.h"



using LimitedIntVec     = std::vector<LimitedInt>;
using LimitedDoubleVec  = std::vector<LimitedDouble>;

Q_DECLARE_METATYPE(LimitedIntVec)
Q_DECLARE_METATYPE(LimitedDoubleVec)

SV_DECL_ALIASES(LimitedIntVec)
SV_DECL_ALIASES(LimitedDoubleVec)


using LimitedIntOrDouble    = std::variant<LimitedInt, LimitedDouble>;
using LimitedIntOrDoubleVec = std::variant<LimitedIntVec, LimitedDoubleVec>;

using doubleOrInt = std::variant<double, int>;


#include "LimitedValue/LimitedDoubleWidget.h"
#include "LimitedValue/LimitedIntWidget.h"

#include "LimitedValue/LimitedDoubleVecWidget.h"

#include "WidgetLogic/WidgetDefs.h"