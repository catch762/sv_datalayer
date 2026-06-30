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
    using ComparatorFunc = std::function<bool(const std::any& a, const std::any& b)>;

    template<typename T>
    static void registerDefaultEqualsForType()
    {
        SV_ASSERT(!instance().comparators.entryExists(typeIndex<T>()));

        instance().comparators.addEntryForType<T>(
            [](const std::any& a, const std::any& b)
            {
                if (!anyHoldsType<T>(a) || !anyHoldsType<T>(b)) return false;

                auto cmp = std::equal_to<T>();
                return cmp(*anyGet<T>(a), *anyGet<T>(b));
            }
        );
    }

    template<typename T, typename FuncT>
    requires std::invocable<FuncT, const T&, const T&> && 
             std::same_as<bool, std::invoke_result_t<FuncT, const T&, const T&>>
    static void registerEqualsFuncForType(FuncT func)
    {
        SV_ASSERT(!instance().comparators.entryExists(typeIndex<T>()));

        instance().comparators.addEntryForType<T>(
            [func](const std::any& a, const std::any& b)
            {
                if (!anyHoldsType<T>(a) || !anyHoldsType<T>(b)) return false;

                return func(*anyGet<T>(a), *anyGet<T>(b));
            }
        );
    }

    static bool equals(const std::any& a, const std::any& b)
    {
        if (typeIndex(a) != typeIndex(b))
        {
            return false;
        }

        if (auto cmp = instance().comparators.getEntry(typeIndex(a)))
        {
            return (*cmp)(a,b);
        }
        else
        {
            SV_ERROR(std::format("No comparator found for {}", a));
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