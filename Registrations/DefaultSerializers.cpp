#include "DefaultSerializers.h"
#include "SerializationLogic/SerializationSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "ContainerSerializers.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"


void DefaultSerializers::Register(SerializationSystem *systemInstance)
{
    systemInstance->registerSerialization<double> ();
    systemInstance->registerSerialization<bool>   ();
    systemInstance->registerSerialization<QString>();

    systemInstance->registerSerialization<LimitedDouble>();
    systemInstance->registerSerialization<LimitedDoubleVec>();

    systemInstance->registerSerialization<LimitedInt>();
    systemInstance->registerSerialization<LimitedIntVec>();

    systemInstance->registerSerialization<DataNodeWrapperWidget*>();
    systemInstance->registerSerialization<LimitedValueVecWidget*>();
    
}