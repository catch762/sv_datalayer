#pragma once
#include "DataNode.h"
#include <typeindex>
#include <map>

class DataLayerSystem
{
public:
    using QtTypeId = int;
    // Must create appropriate widget, initialize it with node's value,
    // and setup updating node on changes, if necessary
    using WidgetMaker = std::function<QWidget*(DataNodeShared)>;

    static DataLayerSystem& instance()
    {
        static DataLayerSystem system;
        return system;
    }

    static QWidget* getWidgetForValue(QVariant value)
    {

    }

    QWidget* getWidgetForNode(DataNodeShared node)
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
                SV_ERROR("DataLayerSystem", QString("No widget maker exist for:%1").arg(leafValue->toString()).toStdString());
                return nullptr;
            }
        }
        else if (auto compData = node->tryGetCompositeData())
        {
            return nullptr;
        }

        SV_UNREACHABLE();
    }

    template<class T>
    void registerWidgetMaker(WidgetMaker maker)
    {
        //todo check overwrite?
        widgetMakers[qMetaTypeId<T>()] = maker;
    }

private:
    const WidgetMaker* getWidgetMakerForContentType(const QVariant &var)
    {
        auto found = widgetMakers.find(var.typeId());
        if (found == widgetMakers.end()) return nullptr;

        return &found->second;
    }

private:
    std::map<QtTypeId, WidgetMaker> widgetMakers;
};