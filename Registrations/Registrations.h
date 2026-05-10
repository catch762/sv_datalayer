#pragma once
#include "DefaultWidgetMakers.h"
#include "DefaultSerializers.h"
#include "DefaultInterpolators.h"


class DatalayerDefaultTypesMetadata
{
public:
    static void registerEverything()
    {
        DefaultSerializers::RegisterEverything();
        DefaultWidgetMakers::RegisterEverything();
    }
};