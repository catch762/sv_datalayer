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
    static T interpolate(const T &A, const T &B, double ratioAToB01)
    {
        static_assert(false, "Called unimplemented interpolator");
    }
};