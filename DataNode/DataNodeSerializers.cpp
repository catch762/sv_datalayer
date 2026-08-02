#include "DataNodeSerializers.h"

#include "WidgetLogic/WidgetDefs.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"

bool SerializerForDataNodeTreeAndItsWidgets::onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode, int level)
{
    //its fine if there is one, its fine if there are none
    if (auto* nodeWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
    {
        auto widgetOptions = nodeWidget->makeOptions();

        //its never empty tho
        if (!widgetOptions.isEmpty())
        {
            SV_LOG("Did write widget data to json");
            jsonOfNode[widgetsKey] = widgetOptions;
        }
    }
    else if (level != 0) //root never has a widget
    {
        SV_WARN(std::format("Serializer: Couldnt find associated widget in WidgetsForNodeManager for {}", node));
    }

    return true;
}

bool SerializerForDataNodeTreeAndItsWidgets::onNodeCreatedFromJson_restoreWidget(   DataNodeShared      node,
                                                                                    const QJsonObject&  jsonOfNode,
                                                                                    int                 level,
                                                                                    bool                makeWidgetForRootNode)
{
    if (level == 0 && !makeWidgetForRootNode)
    {
        return true;
    }

    QJsonObjectWithWidgetOptionsOpt widgetOptionsOpt = getFromJson<QJsonObject>(jsonOfNode, widgetsKey);

    auto createdWidget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, widgetOptionsOpt);

    if (!createdWidget)
    {
        SV_ERROR(std::format("restoring widget failed: Received null NodeWidget for leaf node {}", node));
        return false;
    }

    return true;
}


QJsonValueOpt SerializerForDataNodeTreeAndItsWidgets::toJson(const DataNodeShared& value)
{
    if (!value)
    {
        SV_LOG("Error: trying to serialize null DataNodeShared value");
        return QJsonValue();
    }

    return value->toJSON(onJsonCreatedFromNode_saveWidgetOptions);
}

std::tuple<DataNodeShared, NodeWidget*> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndItsWidget(const QJsonValue& json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind(onNodeCreatedFromJson_restoreWidget,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3,
                                            true)); // <- will make widget for root

    if (!rootNode) return {};

    auto rootNodeWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(rootNode);

    if (!rootNodeWidget)
    {
        SV_ERROR("jsonToRootNodeAndItsWidget: didnt find widget for root");
    }

    return {rootNode, rootNodeWidget};
}

std::tuple<DataNodeShared, NodeWidgetVec> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndTopLevelChildrenWidgets(const QJsonValue &json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind(onNodeCreatedFromJson_restoreWidget,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3,
                                            false)); // <- will NOT make widget for root

    if (!rootNode) return {};

    NodeWidgetVec topLevelChildrenWidgets;
    if (rootNode->isComposite())
    {
        for (auto child : rootNode->tryGetCompositeData()->getChildren())
        {   
            auto childWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(child);

            if (!childWidget)
            {
                SV_ERROR(std::format("jsonToRootNodeAndTopLevelChildrenWidgets: didnt find widget for root's child {}", child));

                WidgetsForNodeManager::printAllEntries();
            }

            topLevelChildrenWidgets.push_back(childWidget);
        }
    }

    return {rootNode, topLevelChildrenWidgets};
}
