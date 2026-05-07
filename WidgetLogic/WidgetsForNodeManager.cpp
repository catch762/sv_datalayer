#include "WidgetsForNodeManager.h"
#include "DataNodeWrapperWidget.h"

void WidgetsForNodeManager::updateAllWidgetsFromNodeState(ConstDataNodeWeak node)
{
    if (auto container = getWidgetsForNode(node))
    {
        for (const auto &widgetEntry : *container)
        {
            if (widgetEntry.stillAlive())
            {
                if (auto* wrapperWidget = qobject_cast<DataNodeWrapperWidget*>(widgetEntry.qPointer.data()))
                {
                    wrapperWidget->updateContentWidgetsFromDataNode(node);
                }
                else SV_WARN(std::format("WidgetsForNodeManager: couldnt cast widget to DataNodeWrapperWidget "
                                            "to update it from node {}", node));
            }
        }
    }
}