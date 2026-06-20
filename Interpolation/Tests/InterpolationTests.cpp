#include "Interpolation/InterpolationSystem.h"
#include "Comparison/ComparisonSystem.h"
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "doctest.h"

using namespace datanode_helpers;


TEST_CASE("Interpolating trees")
{
    //all the mixing at this testcase will be at 50% 
    SV_WARN("interrr");

    auto limd_A                 = LimitedDouble{60, 50, 70};
    auto limd_B                 = LimitedDouble{120, 100, 140};
    auto limd_ExpectedMixed     = LimitedDouble{90, 75, 105};

    auto limint_A               = LimitedInt{50,0,100};
    auto limint_B               = LimitedInt{150,0,300};
    auto limint_ExpectedMixed   = LimitedInt{100,0,200};

    auto limdvec_A              = LimitedDoubleVec{LimitedDouble{10, 10, 50}, LimitedDouble{-10,100,-100}};
    auto limdvec_B              = LimitedDoubleVec{LimitedDouble{20, 20, 100}, LimitedDouble{-20,200,-200}};
    auto limdvec_ExpectedMixed  = LimitedDoubleVec{LimitedDouble{20, 20, 100}, LimitedDouble{-20,200,-200}};

    auto limivec_A              = LimitedIntVec   {LimitedInt{10, 0, 20},     LimitedInt{-10,0,-20}};
    auto limivec_B              = LimitedIntVec   {LimitedInt{-100, 0, -200}, LimitedInt{-100,0,-200}};
    auto limivec_ExpectedMixed  = LimitedIntVec   {LimitedInt{-100, 0, -200}, LimitedInt{-100,0,-200}};

    auto limd_ActualMixed = QVariant::fromValue(LimitedDouble{});
    auto opRes = InterpolationSystem::interpolate(QVariant::fromValue(limd_A), QVariant::fromValue(limd_A), limd_ActualMixed, 0.5);
    CHECK(opRes);
    CHECK(ComparisonSystem::equals(QVariant::fromValue(limd_ExpectedMixed), limd_ActualMixed));

    auto makeSubTreeForDefaultStrat_A = []()
    {
        return  dncomp("types without interp that will use DefaultMixingStrategy specified", {
                    dnleaf("bool",      true),
                    dnleaf("boolvec",   BoolVec{true,false,true}),
                    dnleaf("qstring",   QString("hi")),
                    dnleaf("enum",      Enum({{10, "ten"}, {20, "twenty"}, {100, "hundred"}}, 1))
                });
    };
    auto makeSubTreeForDefaultStrat_B = []()
    {
        return  dncomp("types without interp that will use DefaultMixingStrategy specified", {
                    dnleaf("bool",      false),
                    dnleaf("boolvec",   BoolVec{false,false,false}),
                    dnleaf("qstring",   QString("dont hi")),
                    dnleaf("enum",      Enum())
                });
    };

    auto makeSubTreeForActualInterpolation_A = [&]()
    {
        return  dncomp("types with interp rgistered in DefaultInterpolators.cpp", {
                    dnleaf("limiteddouble",     limd_A),
                    dnleaf("limitedint",        limint_A),
                    dnleaf("limiteddoublevec",  limdvec_A),
                    dnleaf("limitedintvec",     limivec_A)
                });
    };
    auto makeSubTreeForActualInterpolation_B = [&]()
    {
        return  dncomp("types with interp rgistered in DefaultInterpolators.cpp", {
                    dnleaf("limiteddouble",     limd_B),
                    dnleaf("limitedint",        limint_B),
                    dnleaf("limiteddoublevec",  limdvec_B),
                    dnleaf("limitedintvec",     limivec_B)
                });
    };
    auto makeSubTreeForActualInterpolation_ExpectedMixed = [&]()
    {
        return  dncomp("types with interp rgistered in DefaultInterpolators.cpp", {
                    dnleaf("limiteddouble",     limd_ExpectedMixed),
                    dnleaf("limitedint",        limint_ExpectedMixed),
                    dnleaf("limiteddoublevec",  limdvec_ExpectedMixed),
                    dnleaf("limitedintvec",     limivec_ExpectedMixed)
                });
    };
}