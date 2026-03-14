#include "WidgetMakerSystem.h"
#include "DefaultCollections/DefaultWidgetMakers.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

QWidget* WidgetMakerSystem::makeWidgetForNode(DataNodeShared node)
{
    if (!node)
    {
        return nullptr;
    }

    if (auto leafValue = node->tryGetLeafvalue())
    {
        if (auto widgetmaker = getWidgetMakerForContentType(*leafValue))
        {
            if (auto createdInnerWidget = (*widgetmaker)(node))
            {
                return new DataNodeWrapperWidget({createdInnerWidget}, node->getName());
            }
            else
            {
                SV_ERROR("WidgetMakerSystem", "widget maker returned nullptr for: " + node->stdBasicInfo());
                return nullptr;
            }
        }
        else
        {
            //todo bad inspection
            SV_ERROR("WidgetMakerSystem", "No widget maker exist for: " + node->stdBasicInfo());
            return nullptr;
        }
    }
    else if (auto compData = node->tryGetCompositeData())
    {
        //This node doesnt contain QVariant value, it does contain list of other DataNode's -
        //this is a special case handled here.
        SV_ASSERT(false && "not impl yet");
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

WidgetMakerSystem::WidgetMakerSystem()
{
    DefaultWidgetMakers::RegisterEverything(this);
}
