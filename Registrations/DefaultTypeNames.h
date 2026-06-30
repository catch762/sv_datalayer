#pragma once
#include "sv_qtcommon.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

SV_REGTYPENAME(bool);
SV_REGTYPENAME(BoolVec);
SV_REGTYPENAME(int);
SV_REGTYPENAME(double);
SV_REGTYPENAME(QString);
SV_REGTYPENAME(LimitedInt);
SV_REGTYPENAME(LimitedIntVec);
SV_REGTYPENAME(LimitedDouble);
SV_REGTYPENAME(LimitedDoubleVec);
SV_REGTYPENAME(Enum);
SV_REGTYPENAME(EnumVec);

class DefaultTypeNames
{
public:
    static void registerEverything();
};