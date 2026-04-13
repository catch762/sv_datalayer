#include "DataNode.h"
#include "SerializationLogic/SerializationSystem.h"

QJsonObjectOpt DataNode::toJSON(OnJsonCreatedFromNodeAction onJsonCreatedAction) const
{
    QJsonObject obj;
    obj[nameKey] = name;

    if (auto *leafValue = tryGetLeafvalue())
    {
        obj[valueKey] = SerializationSystem::instance().qVariantToJson(*leafValue);
    }
    else if(auto compData = tryGetCompositeData())
    {
        QJsonArray childrenArray;
        for (auto &child : compData->children)
        {
            if (!child)
            {
                SV_WARN(formatMsg("During serializing, encountered null child, ignoring it"));
                continue;
            }

            auto jsonObjOpt = child->toJSON(onJsonCreatedAction);
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
    
    if(onJsonCreatedAction && !onJsonCreatedAction(shared_from_this(), obj))
    {
        SV_ERROR(formatMsg("During serializing, onJsonCreatedAction returned 'false'"));
        return {};
    }

    return obj;
}

DataNodeShared DataNode::fromJSON(QJsonValue jsonValue, OnNodeCreatedFromJsonAction onNodeCreatedAction)
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

        *result->tryGetLeafvalue() = SerializationSystem::instance().jsonToQVariant(leafValue);
    }
    else if(auto childrenArray = getFromJsonAndLogError<QJsonArray>(*json, childrenKey, err)) //Then its Composite node
    {
        result->initPayload(NodeType::Composite);
        CompositeData* resCompData = result->tryGetCompositeData();

        for (auto child : *childrenArray)
        {
            if (DataNodeShared loadedChild = fromJSON(child, onNodeCreatedAction))
            {
                resCompData->children.push_back(loadedChild);
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
        return {};
    }

    if(onNodeCreatedAction && !onNodeCreatedAction(result, *json))
    {
        SV_ERROR(result->formatMsg("During deserializing, onNodeCreatedAction returned 'false'"));
        return {};
    }

    return result;
}