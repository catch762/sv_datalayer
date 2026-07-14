#include "DataNode.h"
#include "SerializationLogic/SerializationSystem.h"

QJsonObjectOpt DataNode::toJSON(OnJsonCreatedFromNodeAction onJsonCreatedAction, int _level) const
{
    QJsonObject obj;
    obj[nameKey] = name;

    if (auto *leafValue = tryGetLeafvalue())
    {
        obj[valueKey] = SerializationSystem::instance().anyToJson(*leafValue, true);
    }
    else if(auto compData = tryGetCompositeData())
    {
        QJsonArray childrenArray;
        for (auto &child : compData->getChildren())
        {
            if (!child)
            {
                SV_WARN(formatMsg("During serializing, encountered null child, ignoring it"));
                continue;
            }

            auto jsonObjOpt = child->toJSON(onJsonCreatedAction, _level + 1);
            if (!jsonObjOpt)
            {
                SV_ERROR(formatMsg("During serializing, child->toJSON returned 'false'"));
                return {};
            }

            childrenArray.append( *jsonObjOpt );
        }

        obj[childrenKey] = childrenArray;
    }
    else
    {
        SV_UNREACHABLE();
    }
    
    if(onJsonCreatedAction && !onJsonCreatedAction(shared_from_this(), obj, _level))
    {
        SV_ERROR(formatMsg("During serializing, onJsonCreatedAction returned 'false'"));
        return {};
    }

    return obj;
}

DataNodeShared DataNode::fromJSON(QJsonValue jsonValue, OnNodeCreatedFromJsonAction onNodeCreatedAction, int _level)
{
    const QString err("DataNode deserialize error");
    DataNodeShared result = std::make_shared<DataNode>();

    auto json = convertJsonAndLogError<QJsonObject>(jsonValue, err);
    if (!json) return {};

    if (auto name = getFromJsonAndLogError<QString>(*json, nameKey, err))
    {
        result->name = *name;
    }
    else return {};

    auto leafValue = json->value(valueKey);
    if (!leafValue.isUndefined()) //Then its Leaf node
    {
        result->initPayload(NodeType::Leaf);

        if (anyOpt leafAny = SerializationSystem::instance().jsonToAny(leafValue))
        {
            *result->tryGetLeafvalue() = *leafAny;
        }
        else
        {
            SV_ERROR(result->formatMsg("During deserializing, received nullopt from SerializationSystem for leaf"));
            return {};
        }
    }
    else if(auto childrenArray = getFromJsonAndLogError<QJsonArray>(*json, childrenKey, err)) //Then its Composite node
    {
        result->initPayload(NodeType::Composite);

        for (auto child : *childrenArray)
        {
            if (DataNodeShared loadedChild = fromJSON(child, onNodeCreatedAction, _level + 1))
            {
                result->addChild(loadedChild);
            }
            else
            {
                SV_ERROR(result->formatMsg("During deserializing, deserializing a child returned 'nullptr'"));
                return {};
            }
        }
    }
    else
    {
        SV_ERROR(result->formatMsg(std::format("During deserializing, on level {}, found ivalid json "
                                               "which hasnt leaf content and hasnt chidlren content", _level)));
        return {};
    }

    if(onNodeCreatedAction && !onNodeCreatedAction(result, *json, _level))
    {
        SV_ERROR(result->formatMsg("During deserializing, onNodeCreatedAction returned 'false'"));
        return {};
    }

    return result;
}
