#include "WidgetMakerSystem.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

QWidget* WidgetMakerSystem::getWidgetForNode(DataNodeShared node)
{
    if (!node)
    {
        return nullptr;
    }

    if (auto leafValue = node->tryGetLeafvalue())
    {
        if (auto widgetmaker = getWidgetMakerForContentType(*leafValue))
        {
            return (*widgetmaker)(node);
        }
        else
        {
            //todo bad inspection
            SV_ERROR("WidgetMakerSystem", QString("No widget maker exist for:%1").arg(leafValue->toString()).toStdString());
            return nullptr;
        }
    }
    else if (auto compData = node->tryGetCompositeData())
    {
        //This node doesnt contain QVariant value, it does contain list of other DataNode's -
        //this is a special case handled here.
        return nullptr;
    }

    SV_UNREACHABLE();
}

const WidgetMakerSystem::WidgetMakerForTypeT* WidgetMakerSystem::getWidgetMakerForContentType(const QVariant &var)
{
    auto found = widgetMakers.find(var.typeId());
    if (found == widgetMakers.end()) return nullptr;

    return &found->second;
}