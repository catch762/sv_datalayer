#include "DataNodeSerializers.h"

#include "WidgetLogic/WidgetDefs.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"

bool SerializerForDataNodeTreeAndItsWidgets::onJsonCreatedFromNode_saveWidgetOptions(ConstDataNodeShared node, QJsonObject &jsonOfNode, int level)
{
    auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node);
    auto hasWidget = qVariantHasWidget(widgetVariant);

    if (hasWidget)
    {
        //if widget isnt supposed to save anything, we will receive empty value - its fine.
        QJsonObjectWithWidgetOptions widgetOptions = SerializationSystem::instance().qVariantToJson(widgetVariant).toObject();
        if (!widgetOptions.isEmpty())
        {
            SV_LOG("Did write widget data to json");
            jsonOfNode[widgetsKey] = widgetOptions;
        }
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

    if (!qVariantHasWidget(createdWidget))
    {
        SV_ERROR(std::format("restoring widget failed: Received null QVariantHoldingWidget for leaf node {}", node));
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

std::tuple<DataNodeShared, QVariantHoldingWidget> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndItsWidget(const QJsonValue& json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind(onNodeCreatedFromJson_restoreWidget,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3,
                                            true)); // <- will make widget for root

    if (!rootNode) return {};

    auto rootNodeWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(rootNode);

    if (!qVariantHasWidget(rootNodeWidget))
    {
        SV_ERROR("jsonToRootNodeAndItsWidget: didnt find widget for root");
    }

    return {rootNode, rootNodeWidget};
}

std::tuple<DataNodeShared, QVariantHoldingWidgetVec> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndTopLevelChildrenWidgets(const QJsonValue &json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind(onNodeCreatedFromJson_restoreWidget,
                                            std::placeholders::_1,
                                            std::placeholders::_2,
                                            std::placeholders::_3,
                                            false)); // <- will NOT make widget for root

    if (!rootNode) return {};

    QVariantHoldingWidgetVec topLevelChildrenWidgets;
    if (rootNode->isComposite())
    {
        for (auto child : rootNode->tryGetCompositeData()->children)
        {   
            auto childWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(child);

            if (!qVariantHasWidget(childWidget))
            {
                SV_ERROR(std::format("jsonToRootNodeAndTopLevelChildrenWidgets: didnt find widget for root's child {}", child));

                WidgetsForNodeManager::printAllEntries();
            }

            topLevelChildrenWidgets.push_back(childWidget);
        }
    }

    return {rootNode, topLevelChildrenWidgets};
}
