#pragma once
#include "sv_qtcommon.h"
#include "Interpolation/InterpolationInterface.h"

//*****************************************************************************************
//
// The idea is simple: if we have type T registered in InterpolationSystem,
// surely we can have templated interpolator for container
// types such as std::vector<T> so we dont have to write them every time.
//
// (Same idea as in ContainerSerializers.h)
//
// However, for every ConcreteType you will use with the container (lets say its a vector):
//  a) you still need to Q_DECLARE_METATYPE(std::vector<ConcreteType>)
//  b) you still need to register 'std::vector<ConcreteType>' in InterpolationSystem
//     passing interpolator from this ContainerSerializers class.
//
//*****************************************************************************************


template<typename T>
class Interpolator< std::vector<T> >
{
public:
    using VectorT = std::vector<T>;

	static void interpolate(const VectorT &A, const VectorT &B, VectorT &Result, double ratioAToB01)
    {
        if (A.size() != B.size())
        {
            SV_ERROR(std::format("Error interpolating two vector<T>: size {} and {}", A.size(), B.size()));
            return;
        }

        Result.resize(A.size());
        for(int i = 0; i < Result.size(); ++i)
        {
            Interpolator<T>::interpolate(A[i], B[i], Result[i], ratioAToB01);
        }
    }
};
