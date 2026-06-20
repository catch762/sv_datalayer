#pragma once
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

using namespace datanode_helpers;

inline DataNodeShared makeTreeWithAllDefaultTypes()
{
    return  dncomp("root",{
                dncomp("mostbasictypes", {
                    dnleaf("double",    double(5.0)),
                    dnleaf("bool",      true),
                    dnleaf("qstring",   QString("hi"))
                }),
                dncomp("mytypes", {
                    dnleaf("limiteddouble", LimitedDouble{6, 5, 7}),
                    dnleaf("limitedint",    LimitedInt{60,50,70}),
                    dnleaf("enum",          Enum({{10, "ten"}, {20, "twenty"}, {100, "hundred"}}, 1))
                }),
                dncomp("vectypes", {
                    dnleaf("limiteddoublevec",  LimitedDoubleVec{LimitedDouble{6, 5, 7}, LimitedDouble{0,0,1}} ),
                    dnleaf("limitedint",        LimitedIntVec{LimitedInt{10, 9, 11}, LimitedInt{0,0,1}}),
                    dnleaf("boolvec",           BoolVec{true,false,true})
                }),
            });
}