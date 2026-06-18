#pragma once
#include "DefaultWidgetMakers.h"
#include "DefaultSerializers.h"
#include "DefaultInterpolators.h"
#include "DefaultComparators.h"

class DatalayerDefaultTypesMetadata
{
public:
    static void registerEverything()
    {
        DefaultSerializers::RegisterEverything();
        DefaultWidgetMakers::RegisterEverything();
        DefaultInterpolators::registerEverything();
        DefaultComparators::registerEverything();
    }
};