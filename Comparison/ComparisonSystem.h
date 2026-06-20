#pragma once
#include "sv_qtcommon.h"
#include "DataForTypeMap.h"

//**********************************************************************************
// So, we want to compare arbitrary types.
//
// We are gonna use std::equal_to: it uses default (a==b) comparison,
// but we can override this impl for any type if we wanted to.
//
// The problem this class solves is: both values we want to compare
// are inside QVariant. So we have to explicitly register this std::equal_to() func.
//
// Then we can call: ComparisonSystem::equals(QVariant(a), QVariant(b))
//**********************************************************************************

//Note: tests that test this system are in DataNode/DataNodeEqualityTest.cpp

class ComparisonSystem
{
public:
    using ComparatorFunc = std::function<bool(const QVariant& a, const QVariant& b)>;

    template<typename T>
    static void registerDefaultEqualsForType()
    {
        SV_ASSERT(!instance().comparators.entryExists(qtTypeId<T>()));

        instance().comparators.addEntryForType<T>(
            [](const QVariant& a, const QVariant& b)
            {
                if (!holdsType<T>(a) || !holdsType<T>(b)) return false;

                auto cmp = std::equal_to<T>();
                return cmp(a.value<T>(), b.value<T>());
            }
        );
    }

    template<typename T, typename FuncT>
    requires std::invocable<FuncT, const T&, const T&> && 
             std::same_as<bool, std::invoke_result_t<FuncT, const T&, const T&>>
    static void registerEqualsFuncForType(FuncT func)
    {
        SV_ASSERT(!instance().comparators.entryExists(qtTypeId<T>()));

        instance().comparators.addEntryForType<T>(
            [func](const QVariant& a, const QVariant& b)
            {
                if (!holdsType<T>(a) || !holdsType<T>(b)) return false;

                return func(a.value<T>(), b.value<T>());
            }
        );
    }

    static bool equals(const QVariant& a, const QVariant& b)
    {
        if (a.typeId() != b.typeId())
        {
            return false;
        }

        if (auto cmp = instance().comparators.getEntry(a.typeId()))
        {
            return (*cmp)(a,b);
        }
        else
        {
            SV_ERROR(std::format("No comparator found for {}", qVariantInfo(a)));
            return false;
        }
    }

private:
    static ComparisonSystem& instance()
    {
        static ComparisonSystem s;
        return s;
    }

private:
    DataForTypeMap<ComparatorFunc> comparators;
};