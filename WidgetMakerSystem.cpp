#include "WidgetMakerSystem.h"
#include "DefaultCollections/DefaultWidgetMakers.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"

#include "WidgetsForNodeManager.h"

WidgetMakerSystem& WidgetMakerSystem::instance()
{
    static WidgetMakerSystem system;
    return system;
}

QVariantHoldingWidget WidgetMakerSystem::createAndRegisterWidgetForNode(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!node)
    {
        return {};
    }

    QVariantHoldingWidget resultWidget = createWidgetForNode (node, options);

    if (qVariantHasWidget(resultWidget))
    {
        WidgetsForNodeManager::registerWidgetForNode(node, resultWidget);
    }

    return resultWidget;
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollection(QtTypeIndex typeIndex)
{
    auto found = widgetMakerCollections.find(typeIndex);
    if (found != widgetMakerCollections.end())
    {
        return &found->second;
    }

    //SV_ERROR("No widget maker collection registered for " + qtTypeInfo(typeIndex).toStdString());
    return nullptr;
}

WidgetMakerSystem::WidgetMakerCollection *WidgetMakerSystem::getCollectionAndCreateIfNotFound(QtTypeIndex typeIndex)
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

const WidgetMakerSystem::WidgetMakerForTypeT *WidgetMakerSystem::getWidgetMakerForContentType(const QVariant &var,
                                                                                              QStringOpt widgetMakerNameOpt)
{
    if (auto* collection = getCollection(var.typeId()))
    {
        QString widgetMakerName = widgetMakerNameOpt.value_or(collection->defaultWidgetMakerName);

        auto foundWidgetMaker = collection->widgetMakers.find(widgetMakerName);
        if (foundWidgetMaker != collection->widgetMakers.end())
        {
            return &foundWidgetMaker->second;
        }
        else
        {
            SV_ERROR(QString("Requested widgetMakerName=[%1] for %2, but no such widgetMaker in collection")
                    .arg(widgetMakerName).arg(qVariantInfo(var)).toStdString());
        }
    }
    else
    {
        SV_ERROR(QString("Requested widgetMaker for %1, but nothing is registered for this type")
                .arg(qVariantInfo(var)).toStdString());
    }

    return nullptr;
}

QVariantHoldingWidget WidgetMakerSystem::createWidgetForNode(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt &options)
{
    SV_ASSERT(node);

    if (options)
    {
        SV_LOG(std::format("createWidgetForNode {} with options {}", node, jsonValueToString(*options)));
    }

    return node->isLeaf() ? createWidgetForLeafNode     (node, options) :
                            createWidgetForCompositeNode(node, options);
}

QVariantHoldingWidget WidgetMakerSystem::createWidgetForCompositeNode(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt &options)
{
    SV_ASSERT(node);
    SV_ASSERT(node->isComposite())

    std::vector<QVariantHoldingWidget> widgetsOfChildren;

    for (auto childNode : node->tryGetCompositeData()->children)
    {
        auto widgetVariant = WidgetsForNodeManager::getSaveablePrimaryWidgetForNode(childNode);

        if (qVariantHasWidget(widgetVariant))
        {
            widgetsOfChildren.push_back(widgetVariant);
        }
        else
        {
            //Apparently, we didnt go depth-first. So we are creating widgets now, and at the moment we dont have options for them
            auto createdChildWidget = createAndRegisterWidgetForNode(childNode, QJsonObjectWithWidgetOptionsOpt{});
            if (qVariantHasWidget(createdChildWidget))
            {
                widgetsOfChildren.push_back(createdChildWidget);
            }
            //is it fine? maybe it is
            //SV_WARN(std::format("Could not obtain widget for node {} which is child of {}", childNode, node));
        }
    }

    if (widgetsOfChildren.empty())
    {
        SV_ERROR(std::format("Could not createAndRegisterWidgetForCompositeNode for {} "
                             "because didnt find a single child widget. Not doing anything then.", node));
        return {};
    }

    //todo use the options

    auto *wrapper = new DataNodeWrapperWidget(widgetsOfChildren, node->getName());
    return QVariant::fromValue( wrapper );
}

QVariantHoldingWidget WidgetMakerSystem::createWidgetForLeafNode(DataNodeShared node, const QJsonObjectWithWidgetOptionsOpt &options)
{
    SV_ASSERT(node);
    SV_ASSERT(node->isLeaf())

    auto widgetMakerNameOpt = getWidgetMakerNameOpt(options);

    auto *leafValue = node->tryGetLeafvalue();

    if (auto widgetmaker = getWidgetMakerForContentType(*leafValue, widgetMakerNameOpt))
    {
        auto createdInnerWidget = (*widgetmaker)(node, options);
        if (qVariantHasWidget(createdInnerWidget))
        {
            auto *wrapperWidget = new DataNodeWrapperWidget({createdInnerWidget}, node->getName());

            return QVariant::fromValue( wrapperWidget );
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

WidgetMakerSystem::WidgetMakerSystem()
{
    DefaultWidgetMakers::RegisterEverything(this);
}
