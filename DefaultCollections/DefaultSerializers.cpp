#include "DefaultSerializers.h"
#include "SerializationSystem.h"
#include "TypesAndWidgets/TypesAndWidgets.h"
#include "ContainerSerializers.h"



void DefaultSerializers::Register(SerializationSystem *systemInstance)
{
    systemInstance->registerSerialization<double> ();
    systemInstance->registerSerialization<bool>   ();
    systemInstance->registerSerialization<QString>();

    systemInstance->registerSerialization<LimitedDouble>();
    systemInstance->registerSerialization<LimitedDoubleVec>();
    
}