#include "DataNodeEquality.h"
#include "Comparison/ComparisonSystem.h"
#include "doctest/doctest.h"

bool leafValuesEqual(const std::any& a, const std::any& b)
{
    return ComparisonSystem::equals(a,b);
}

