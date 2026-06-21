#include "Interpolation/InterpolationSystem.h"
#include "Comparison/ComparisonSystem.h"
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "doctest.h"

using namespace datanode_helpers;


TEST_CASE("Interpolating trees")
{
    //all the mixing at this testcase will be at 50%:
    const double MixRatio = 0.5;

    auto checkVarsMixing = [MixRatio](const auto &a, const auto &b, const auto& expectedMixResult)
    {
        using ValueType = std::remove_cvref_t<decltype(a)>;

        auto varA               = QVariant::fromValue(a);
        auto varB               = QVariant::fromValue(b);
        auto actualMixResult    = QVariant::fromValue(a); // assigning 'a' just so it has same type
        auto success            = InterpolationSystem::interpolate( varA,
                                                                    varB,
                                                                    actualMixResult,
                                                                    MixRatio );
        if (!success)
        {
            FAIL_CHECK(std::format("{}: failed to interpolate, prob type mismatch, variants were A={} B={} RES={}",
                                   qtTypeName<ValueType>(), varA, varB, actualMixResult));
        }
        
        REQUIRE(holdsType<ValueType>(actualMixResult)); //just in case

        if(!ComparisonSystem::equals(QVariant::fromValue(expectedMixResult), actualMixResult))
        {
            FAIL_CHECK(std::format("{}: interpolation result doesnt match expected: \nA  ={} \nB  ={} \nEXP={} \nRES={}",
                                   qtTypeName<ValueType>(), a, b, expectedMixResult, actualMixResult.template value<ValueType>()));
        }
    };

    const auto limd_A                 = LimitedDouble{60, 50, 70};
    const auto limd_B                 = LimitedDouble{120, 100, 140};
    const auto limd_ExpectedMixed     = LimitedDouble{90, 75, 105};

    const auto limint_A               = LimitedInt{50,0,100};
    const auto limint_B               = LimitedInt{150,-10,300};
    const auto limint_ExpectedMixed   = LimitedInt{100,-5,200};

    const auto limdvec_A              = LimitedDoubleVec{LimitedDouble{10, 10, 50},     LimitedDouble{-10,100,-100}};
    const auto limdvec_B              = LimitedDoubleVec{LimitedDouble{20, 20, 100},    LimitedDouble{-20,200,-200}};
    const auto limdvec_ExpectedMixed  = LimitedDoubleVec{LimitedDouble{15, 15, 75},     LimitedDouble{-15,150,-150}};

    const auto limivec_A              = LimitedIntVec   {LimitedInt{10,   0, 20},   LimitedInt{-10, 0,-20}};
    const auto limivec_B              = LimitedIntVec   {LimitedInt{-100, 0, -200}, LimitedInt{-100,0,-200}};
    const auto limivec_ExpectedMixed  = LimitedIntVec   {LimitedInt{-45,  0, -90},  LimitedInt{-55, 0,-110}};

    checkVarsMixing(limd_A,     limd_B,     limd_ExpectedMixed);
    checkVarsMixing(limint_A,   limint_B,   limint_ExpectedMixed);
    checkVarsMixing(limdvec_A,  limdvec_B,  limdvec_ExpectedMixed);
    checkVarsMixing(limivec_A,  limivec_B,  limivec_ExpectedMixed);

    /*
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
    */
}