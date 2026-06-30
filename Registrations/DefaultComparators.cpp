#include "DefaultComparators.h"
#include "Comparison/ComparisonSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

void DefaultComparators::registerEverything()
{
    //fuzzy:
    ComparisonSystem::registerEqualsFuncForType<double>(arithmeticEquals<double, double>);
    //ComparisonSystem::registerEqualsFuncForType<float>(arithmeticEquals<float, float>);

    ComparisonSystem::registerDefaultEqualsForType<bool>();
    ComparisonSystem::registerDefaultEqualsForType<int>();
    ComparisonSystem::registerDefaultEqualsForType<QString>();
    ComparisonSystem::registerDefaultEqualsForType<BoolVec>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedInt>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedIntVec>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedDouble>();
    ComparisonSystem::registerDefaultEqualsForType<LimitedDoubleVec>();
    ComparisonSystem::registerDefaultEqualsForType<Enum>();
    ComparisonSystem::registerDefaultEqualsForType<EnumVec>();

}