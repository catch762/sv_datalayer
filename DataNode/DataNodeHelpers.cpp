#include "DataNodeHelpers.h"
#include "Comparison/ComparisonSystem.h"
#include "doctest/doctest.h"

bool leafValuesEqual(const QVariant &a, const QVariant &b)
{
    return ComparisonSystem::equals(a,b);
}

