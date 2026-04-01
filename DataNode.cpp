#include "DataNode.h"

QJsonValue DataNode::toJSON() const
{
    QJsonObject obj;
    obj[nameKey] = name;

    if (isLeaf())
    {
        obj[valueKey] = "todo";// Serialization::any to json(getLeafvalue); ALSO TEMPLATED version without any
    }
    else if(auto compData = tryGetCompositeData())
    {
        QJsonArray childrenArray;
        for (auto &child : compData->children)
        {
            if (child)
            {
                childrenArray.append( child->toJSON() );
            }
        }

        obj[childrenKey] = childrenArray;
    }
    //else unreachable

    return obj;
}

DataNodeShared DataNode::fromJSON(QJsonValue jsonValue)
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

        //todo.

        return result;
    }
    else if(auto childrenArray = getFromJsonAndLogError<QJsonArray>(*json, childrenKey, err)) //Then its Composite node
    {
        result->initPayload(NodeType::Composite);
        CompositeData* resCompData = result->tryGetCompositeData();

        for (auto child : *childrenArray)
        {
            if (DataNodeShared loadedChild = fromJSON(child))
            {
                resCompData->children.push_back(loadedChild);
            }
            else
            {
                //error
                return {};
            }
        }
        
        return result;
    }
    else return {};
}