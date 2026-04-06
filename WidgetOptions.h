#pragma once
#include "sv_qtcommon.h"

using QJsonObjectWithWidgetOptions = QJsonObject;
SV_DECL_OPT(QJsonObjectWithWidgetOptions);

constexpr auto WidgetMakerNameKey = "_maker";

inline QStringOpt getWidgetMakerNameOpt(const QJsonObjectWithWidgetOptions &obj)
{
    //return getFromJsonAndLogError<QString>(obj, WidgetMakerNameKey);

    auto value = obj[WidgetMakerNameKey];
    if (value.isString()) return value.toString();

    return {};
}

inline QStringOpt getWidgetMakerNameOpt(const QJsonObjectWithWidgetOptionsOpt &objOpt)
{
    return objOpt ? getWidgetMakerNameOpt(*objOpt) : QStringOpt();
}

inline void setWidgetMakerName(const QJsonObjectWithWidgetOptions &obj, QString widgetMakerName)
{
    obj[WidgetMakerNameKey] = widgetMakerName;
}