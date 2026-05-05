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
    // So, both A and B are QVariant's holding value of T, and so is return value (A mixed to B).
    //
    // When providing impl, dont check if A/B really holds T (just put assert for that).
    // Code which calls this (InterpolationSystem) does all the checks and error printing
    // and will not call this if types do mismatch.
    static QVariant interpolate(const QVariant &A, QVariant &B, double ratioAToB01)
    {
        static_assert(false, "Unimplemented interpolator");
    }
};