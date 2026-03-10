#pragma once
#include "sv_qtcommon.h"
#include "DataNode.h"

class WidgetMakerSystem;

class DefaultWidgetMakers
{
public:
    static void RegisterEverything(WidgetMakerSystem *system);

    static QWidget* widgetMakerForQString(DataNodeShared leafWithQString);
};