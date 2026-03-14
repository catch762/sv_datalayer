#pragma once
#include "LimitedValue/LimitedValue.h"

#include "LimitedValue/LimitedDoubleWidget.h"
#include "LimitedValue/LimitedIntWidget.h"

using LimitedIntVec     = std::vector<LimitedInt>;
using LimitedDoubleVec  = std::vector<LimitedDouble>;

Q_DECLARE_METATYPE(LimitedIntVec)
Q_DECLARE_METATYPE(LimitedDoubleVec)

SV_DECL_ALIASES(LimitedIntVec)
SV_DECL_ALIASES(LimitedDoubleVec)

#include "LimitedValue/LimitedDoubleVecWidget.h"