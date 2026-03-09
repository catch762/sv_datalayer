#pragma once
#include "DataNode.h"
#include <typeindex>
#include <map>

/*
template<typename T>
class LimitedParam
{
public:
    LimitedParam(T _value, T _min = {}, T _max = {})
    : value(_value), min(_min), max(_max)
    {}

    static QJsonValue toJSON(const LimitedParam &param)
    {
        QJsonObject obj;
        
    }

    static std::optional<LimitedParam> fromJSON(const QJsonValue &jsonValue)
    {
        
    }

private:
    T value;
    T min;
    T max;

    static inline const QString minKey = QString("min");
    static inline const QString maxKey = QString("max");
    static inline const QString valKey = QString("val");
};
*/

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

    // Checks that node is a leaf containing fully registered type T,
    // prints verbose logs if something's wrong
    template<typename T>
    bool checkIsProperNodeForCreatingWidgetOfType(DataNodeShared node)
    {
        //todo: write about how exactly to register

        int typeId = qtTypeId<T>();
        if (typeId == 0)
        {
            SV_ERROR("DataLayerSystem", "Can not create widget: the type has typeid=0, so its not registered");
            return false;
        }

        QString typeName = qtTypeName<T>();
        if(typeName.isEmpty())
        {
            SV_ERROR("DataLayerSystem", std::format("Can not create widget: type has typeid=[{}], but empty typeName, so is not FULLY registered", typeId));
            return false;
        }

        auto errMsgHeader = std::format("Can not create widget of type[{}]: ", typeName.toStdString());

        if (!node)
        {
            SV_ERROR("DataLayerSystem", errMsgHeader + "null node passed in");
            return false;
        }

        if (!node->isLeaf())
        {
            SV_ERROR("DataLayerSystem", errMsgHeader + "node isnt even leaf, its: " + node->stdBasicInfo());
            return false;
        }

        if (!node->isLeafWithType<T>())
        {
            //todo better log
            SV_ERROR("DataLayerSystem", errMsgHeader + "its a leaf but types mismatch: " + node->stdBasicInfo());
            return false;
        }

        return true;
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