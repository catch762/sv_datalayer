#pragma once
#include "sv_qtcommon.h"

//*************************************************************************************
//
//  Usage: in header file, after YourType definition, define Interpolator<YourType> impl.
//
//*************************************************************************************
template<typename T>
class Interpolator
{
public:
    //Not sure if i ll keep it, but i decided to leave default impl instead of assert.
    static void interpolate(const T &A, const T &B, T& Result, double ratioAToB01)
    {
        Result = A + (B - A) * ratioAToB01;
        //static_assert(false, "Called unimplemented interpolator");
    }

    //utils method, dont need to provide another impl for that
    static T interpolate(const T &A, const T &B, double ratioAToB01)
    {
        T Result{};
        interpolate(A, B, Result, ratioAToB01);
    }
};