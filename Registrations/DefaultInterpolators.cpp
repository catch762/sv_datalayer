#include "DefaultInterpolators.h"
#include "Interpolation/InterpolationSystem.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "Utils/ContainerInterpolators.h"

void DefaultInterpolators::registerEverything()
{
    InterpolationSystem::registerTypeInterpolator<double>();

    InterpolationSystem::registerTypeInterpolator<LimitedInt>();
    InterpolationSystem::registerTypeInterpolator<LimitedDouble>();

    InterpolationSystem::registerTypeInterpolator<LimitedIntVec>();
    InterpolationSystem::registerTypeInterpolator<LimitedDoubleVec>();
}
