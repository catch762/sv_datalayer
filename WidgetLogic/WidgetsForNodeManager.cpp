#include "WidgetsForNodeManager.h"
#include "NodeWidget.h"

void WidgetsForNodeManager::updateAllWidgetsFromNodeState(ConstDataNodeWeak node)
{
    if (auto container = getWidgetsForNode(node))
    {
        for (const auto &widgetEntry : *container)
        {
            if (widgetEntry)
            {
                widgetEntry->setWidgetValueFromNodeValue();
            }
        }
    }
}