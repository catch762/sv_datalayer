#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"

#include "WidgetLogic/WidgetsForNodeManager.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

NodeWidget* WidgetMakerSystem::createAndRegisterWidgetForNode(  DataNodeShared              node, 
                                                                const WidgetOptionsJsonOpt& options,
                                                                const MapOfWidgetOptionsForNodes* optionsForChildren )
{
    if (!node)
    {
        return {};
    }

    NodeWidget* resultWidget = createWidgetForNode (node, options);

    if (resultWidget)
    {
        WidgetsForNodeManager::registerWidgetForNode(node, resultWidget);
    }


    return resultWidget;
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollection(std::type_index typeIndex)
{
    auto found = widgetMakerCollections.find(typeIndex);
    if (found != widgetMakerCollections.end())
    {
        return &found->second;
    }

    //SV_ERROR("No widget maker collection registered for " + qtTypeInfo(typeIndex).toStdString());
    return nullptr;
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollectionAndCreateIfNotFound(std::type_index typeIndex)
{
    if (auto existing = getCollection(typeIndex))
    {
        return existing;
    }

    widgetMakerCollections[typeIndex] = WidgetMakerCollection{};

    auto* res = getCollection(typeIndex);
    SV_ASSERT(res);
    return res;
}

const WidgetMakerSystem::WidgetMakerForTypeT *WidgetMakerSystem::getWidgetMakerForContentType(const std::any &any,
                                                                                              QStringOpt widgetMakerNameOpt)
{
    if (auto* collection = getCollection(typeIndex(any)))
    {
        QString widgetMakerName = widgetMakerNameOpt.value_or(collection->defaultWidgetMakerName);

        auto foundWidgetMaker = collection->widgetMakers.find(widgetMakerName);
        if (foundWidgetMaker != collection->widgetMakers.end())
        {
            return &foundWidgetMaker->second;
        }
        else
        {
            SV_ERROR(std::format("Requested widgetMakerName=[%1] for %2, but no such widgetMaker in collection",
                widgetMakerName, any));
        }
    }
    else
    {
        SV_ERROR(std::format("Requested widgetMaker for %1, but nothing is registered for this type",
                any));
    }

    return nullptr;
}

NodeWidget* WidgetMakerSystem::createWidgetForNode( DataNodeShared              node, 
                                                    const WidgetOptionsJsonOpt& options,
                                                    MapOfWidgetOptionsForNodes* optionsForChildren )
{
    SV_ASSERT(node);

    SV_LOG(std::format("createWidgetForNode {} with options: {}", node, options ? jsonValueToString(*options) : QString("none")));

    NodeWidget* res = node->isLeaf() ? createWidgetForLeafNode                 (node, options) :
                                       recursivelyCreateWidgetsForCompositeNode(node, options);
    if (!res)
    {
        SV_ERROR(std::format("createWidgetForNode failed for {}", node));
    }

    return res;
}

//todo rename
NodeWidget* WidgetMakerSystem::recursivelyCreateWidgetsForCompositeNode(DataNodeShared              node, 
                                                                        const WidgetOptionsJsonOpt& options,
                                                                        MapOfWidgetOptionsForNodes* optionsForChildren)
{
    SV_ASSERT(node);
    SV_ASSERT(node->isComposite())

    std::vector<NodeWidget*> widgetsOfChildren;

    for (auto childNode : node->tryGetCompositeData()->getChildren())
    {
        if (auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(childNode))
        {
            widgetsOfChildren.push_back(widgetVariant);
        }
        else
        {
            //Apparently, we didnt go depth-first. So we are creating child widgets now

            auto childWidgetOptionsOpt = optionsForChildren ? getValueOpt(*optionsForChildren, ConstDataNodeWeak(childNode)) :
                                                                                               WidgetOptionsJsonOpt{};

            if (auto createdChildWidget = createAndRegisterWidgetForNode(childNode, childWidgetOptionsOpt, optionsForChildren))
            {
                widgetsOfChildren.push_back(createdChildWidget);
            }
        }
    }

    if (widgetsOfChildren.empty())
    {
        SV_ERROR(std::format("Could not createAndRegisterWidgetisForCompositeNode for {} "
                             "because didnt find a single child widget. Not doing anything then.", node));
        return {};
    }

    auto *wrapper = NodeWidget::makeNodeWidgetForCompositeNode (widgetsOfChildren, node, node->getName(), options);
    return wrapper;
}

NodeWidget* WidgetMakerSystem::createWidgetForLeafNode(DataNodeShared node, const WidgetOptionsJsonOpt &options)
{
    SV_ASSERT(node);
    SV_ASSERT(node->isLeaf())

    auto widgetMakerNameOpt = getWidgetMakerNameOpt(options);

    auto *leafValue = node->tryGetLeafvalue();

    if (auto widgetmaker = getWidgetMakerForContentType(*leafValue, widgetMakerNameOpt))
    {
        if (auto *widget = (*widgetmaker)(node, options))
        {
            return widget;
        }
        else
        {
            SV_ERROR(std::format("widget maker returned null for {}", node));
            return {};
        }
    }
    else
    {
        //todo bad inspection
        SV_ERROR(std::format("No widget maker exist for {}", node)); 
        return {};
    }
}