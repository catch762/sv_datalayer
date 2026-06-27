#include "WidgetsForNodeManager.h"
#include "NodeWidget.h"

void WidgetsForNodeManager::updateAllWidgetsFromNodeState(ConstDataNodeWeak node)
{
    if (auto container = getWidgetsForNode(node))
    {
        for (const auto &widgetEntry : *container)
        {
            if (widgetEntry.stillAlive())
            {
                if (auto* wrapperWidget = qobject_cast<NodeWidget*>(widgetEntry.qPointer.data()))
                {
                    wrapperWidget->setWidgetValueFromNodeValue();
                }
                else SV_WARN(std::format("WidgetsForNodeManager: couldnt cast widget to NodeWidget "
                                            "to update it from node {}", node));
            }
        }
    }
}