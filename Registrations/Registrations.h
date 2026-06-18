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

        everythingRegisteredFlagRef() = true;
    }

    static bool everythingWasRegistered()
    {
        return everythingRegisteredFlagRef();
    }

private:
    static bool& everythingRegisteredFlagRef()
    {
        static bool flag = false;
        return flag;
    }
};