#pragma once
#include "sv_qtcommon.h"
#include "DataNode/DataNodeHeader.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "WidgetLogic/WidgetDefs.h"

class WidgetMakerSystem;
class DataNodeWrapperWidget;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything(WidgetMakerSystem *system);

    static DataNodeWrapperWidget* widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options);

    static DataNodeWrapperWidget* widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options);
    static DataNodeWrapperWidget* widgetMakerForLimitedInt   (DataNodeShared leafWithLimitedInt,    const QJsonObjectWithWidgetOptionsOpt &options);

    static DataNodeWrapperWidget* widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options);
    static DataNodeWrapperWidget* widgetMakerForLimitedIntVec   (DataNodeShared leafWithLimitedIntVec,    const QJsonObjectWithWidgetOptionsOpt &options);
};