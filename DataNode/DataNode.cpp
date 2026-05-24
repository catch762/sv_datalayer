#include "DataNode.h"
#include "SerializationLogic/SerializationSystem.h"

QJsonObjectOpt DataNode::toJSON(OnJsonCreatedFromNodeAction onJsonCreatedAction, int _level) const
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

        *result->tryGetLeafvalue() = SerializationSystem::instance().jsonToQVariant(leafValue);
    }
    else if(auto childrenArray = getFromJsonAndLogError<QJsonArray>(*json, childrenKey, err)) //Then its Composite node
    {
        result->initPayload(NodeType::Composite);
        CompositeData* resCompData = result->tryGetCompositeData();

        for (auto child : *childrenArray)
        {
            if (DataNodeShared loadedChild = fromJSON(child, onNodeCreatedAction, _level + 1))
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

    if(onNodeCreatedAction && !onNodeCreatedAction(result, *json, _level))
    {
        SV_ERROR(result->formatMsg("During deserializing, onNodeCreatedAction returned 'false'"));
        return {};
    }

    return result;
}

bool DataNode::structurallyEqual(const DataNode& first, const DataNode& second, std::string *outMismatchInfo, int _currentLevel)
{
    std::string mismatchBegin;
    if (outMismatchInfo)
    {
        mismatchBegin = std::format("[Level {}] NOT EQUAL: {} AND {}:",
                                    _currentLevel, first.basicInfo(), second.basicInfo());
    }

    if (first.getName() != second.getName())
    {
        if (outMismatchInfo)
        {
            *outMismatchInfo = std::format("{} names mismatch", mismatchBegin);
        }

        return false;
    }

    if (first.isLeaf() != second.isLeaf())
    {
        if (outMismatchInfo)
        {
            *outMismatchInfo = std::format("{} node type mismatch", mismatchBegin);
        }

        return false;
    }

    if (first.isLeaf())
    {
        if(first.tryGetLeafvalue()->typeId() != second.tryGetLeafvalue()->typeId())
        {
            if (outMismatchInfo)
            {
                *outMismatchInfo = std::format("{} leaf value type mismatch", mismatchBegin);
            }

            return false;
        }
    }
    else
    {
        const auto& firstChildren  = first.tryGetCompositeData()->children;
        const auto& secondChildren = second.tryGetCompositeData()->children;

        if (firstChildren.size() != secondChildren.size())
        {
            if (outMismatchInfo)
            {
                *outMismatchInfo = std::format("{} different children count", mismatchBegin);
            }

            return false;    
        }

        //Ok, we can compare pairs of children now, and its responsibility of nested 'structurallyEqual' call.
        //The only other check we do on this level is nullptr check, just in case.
        for (int i = 0; i < firstChildren.size(); ++i)
        {
            auto firstChild = firstChildren[i];
            auto secondChild = secondChildren[i];

            auto doLogNullptrMessage = [&](bool isFirst)
            {
                std::string msg = std::format("{} '{}' has nullptr child at [{}] idx",
                     mismatchBegin, isFirst ? "first" : "second", i);
                SV_ERROR(msg);

                if (outMismatchInfo)
                {
                    *outMismatchInfo = msg;
                }
            };

            if (!firstChild)
            {
                doLogNullptrMessage(true);
            }
            if (!secondChild)
            {
                doLogNullptrMessage(false);
            }
            if (!firstChild || !secondChild)
            {
                //error is already saved at this point
                return false; 
            }

            if (!DataNode::structurallyEqual(*firstChild, *secondChild, outMismatchInfo, _currentLevel + 1))
            {
                //So, the call that decided that children were not equal did save
                //error to 'outMismatchInfo' already - now we just silently return.
                return false;
            }
        }
    }

    //nothing mismatched - trees are structurally equal.
    return true;
}
