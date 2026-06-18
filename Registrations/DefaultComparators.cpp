#include "DefaultComparators.h"
#include "Comparison/ComparisonSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

void DefaultComparators::registerEverything()
{
    ComparisonSystem::registerDefaultEqualsForType<bool>();
    ComparisonSystem::registerDefaultEqualsForType<BoolVec>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedInt>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedIntVec>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedDouble>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedDoubleVec>();
    ComparisonSystem::registerDefaultEqualsForType<Enum>();
    ComparisonSystem::registerDefaultEqualsForType<EnumVec>();

}