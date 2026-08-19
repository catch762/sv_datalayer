#include "DataNodeSerializers.h"

#include "WidgetLogic/WidgetDefs.h"
#include "WidgetLogic/WidgetsForNodeManager.h"
#include "SerializationLogic/SerializationSystem.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"

MapOfWidgetOptionsForNodes SerializerForDataNodeTreeAndItsWidgets::getOptionsFromWidgetsOfTree(DataNodeShared tree)
{
    SV_ASSERT(tree);

    MapOfWidgetOptionsForNodes result;

    tree->iterateRecoursively(tree, [&](const DataNodeShared& node)
    {
        if (auto* nodeWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(node))
        {
            auto widgetOptions = nodeWidget->makeOptions();
            if (!widgetOptions.isEmpty())
            {
                result[node] = widgetOptions;
            }
        }
        else SV_WARN(std::format("getWidgetOptionsForTree: Are you sure its ok that we cant find widget for {} ?", node));
    });

    return result;
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

    WidgetOptionsJsonOpt widgetOptionsOpt = getFromJson<QJsonObject>(jsonOfNode, widgetsKey);

    auto createdWidget = WidgetMakerSystem::instance().createAndRegisterWidgetForNode(node, widgetOptionsOpt);

    if (!createdWidget)
    {
        SV_ERROR(std::format("restoring widget failed: Received null NodeWidget for leaf node {}", node));
        return false;
    }

    return true;
}

bool SerializerForDataNodeTreeAndItsWidgets::onNodeCreatedFromJson_saveOptions( DataNodeShared              node,
                                                                                const QJsonObject&          jsonOfNode,
                                                                                int                         level,
                                                                                MapOfWidgetOptionsForNodes& savedOptions)
{
    if (WidgetOptionsJsonOpt widgetOptionsOpt = getFromJson<QJsonObject>(jsonOfNode, widgetsKey))
    {
        savedOptions[node] = *widgetOptionsOpt;
    }

    return true;
}

QJsonValueOpt SerializerForDataNodeTreeAndItsWidgets::toJson(const DataNodeShared& tree)
{
    MapOfWidgetOptionsForNodes optionsMap = getOptionsFromWidgetsOfTree(tree);
    return toJson(tree, optionsMap);
}

QJsonValueOpt SerializerForDataNodeTreeAndItsWidgets::toJson(const DataNodeShared& tree, const MapOfWidgetOptionsForNodes& optionsMap)
{
    auto onJsonCreatedFromNodeAlsoInjectOptionsFromMap = [optionsMap](ConstDataNodeShared node, QJsonObject& jsonOfNode, int level)
    {
        if (auto options = getValue(optionsMap, ConstDataNodeWeak(node)))
        {
            jsonOfNode[widgetsKey] = *options;
        }

        return true;
    };

    return tree->toJSON(onJsonCreatedFromNodeAlsoInjectOptionsFromMap);
}

std::tuple<DataNodeShared, NodeWidget*> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndItsWidget(const QJsonValue& json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind( onNodeCreatedFromJson_restoreWidget,
                                                            std::placeholders::_1,
                                                            std::placeholders::_2,
                                                            std::placeholders::_3,
                                                            true )); // <- will make widget for root

    if (!rootNode) return {};

    auto rootNodeWidget = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(rootNode);

    if (!rootNodeWidget)
    {
        SV_ERROR("jsonToRootNodeAndItsWidget: didnt find widget for root");
    }

    return {rootNode, rootNodeWidget};
}

std::tuple<DataNodeShared, NodeWidgetQPointerVec> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndTopLevelChildrenWidgets(const QJsonValue &json)
{
    auto rootNode = DataNode::fromJSON(json, std::bind( onNodeCreatedFromJson_restoreWidget,
                                                            std::placeholders::_1,
                                                            std::placeholders::_2,
                                                            std::placeholders::_3,
                                                            false )); // <- will NOT make widget for root

    if (!rootNode) return {};

    NodeWidgetQPointerVec topLevelChildrenWidgets;
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

std::tuple<DataNodeShared, MapOfWidgetOptionsForNodes> SerializerForDataNodeTreeAndItsWidgets::jsonToRootNodeAndWidgetOptions(const QJsonValue& json)
{
    MapOfWidgetOptionsForNodes options = {};

    auto rootNode = DataNode::fromJSON(json, std::bind( onNodeCreatedFromJson_saveOptions,
                                                            std::placeholders::_1,
                                                            std::placeholders::_2,
                                                            std::placeholders::_3,
                                                            std::ref(options) ));

    if (!rootNode) return {};

    return { rootNode, options };
}