#include "DefaultSerializers.h"
#include "SerializationLogic/SerializationSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "Utils/ContainerSerializers.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"


void DefaultSerializers::RegisterEverything()
{
    auto& system = SerializationSystem::instance();

    system.registerSerialization<double> ();
    system.registerSerialization<bool>   ();
    system.registerSerialization<QString>();
    
    system.registerSerialization<BoolVec>   ();

    system.registerSerialization<LimitedDouble>();
    system.registerSerialization<LimitedDoubleVec>();

    system.registerSerialization<LimitedInt>();
    system.registerSerialization<LimitedIntVec>();

    system.registerSerialization<DataNodeWrapperWidget*>();
    system.registerSerialization<LimitedValueVecWidget*>();
    
}