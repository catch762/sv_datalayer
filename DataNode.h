#pragma once

#include <memory>
#include <QString>
#include <any>
#include <vector>
#include <QJsonObject>
#include <QJsonArray>
#include <format>
#include "sv_qtcommon.h"
#include "tsl/ordered_map.h"

class DataNode;
SV_DECL_ALIASES(DataNode);

// 'Composite pattern' tree node class to hold arbitrary data / nested structures.
// 
// Any DataNode is either:
//  1) isLeaf()      - just holds one variable (wrapped in QVariant). Can not have children nodes.
//  2) isComposite() - the opposite: holds no data, but does have a list of children nodes.
//
// Both these types can have 'type name' though: either type name of variable inside Leaf node, or
// essentially a schema name represnting data layout of Composite node. See 'getTypeName()'
//
// The class is written with Qt Model's in mind

class DataNode : public std::enable_shared_from_this<DataNode> 
{
public:
    enum class NodeType
    {
        Leaf,
        Composite
    };

    using LeafValue = QVariant;
    struct CompositeData
    {
        DataNodeShared getChild(int idx)
        {
            return hasChild(idx) ? children[idx] : DataNodeShared();
        }
        int childrenCount()
        {
            return children.size();
        }
        bool hasChild(int idx)
        {
            return idx >= 0 && idx < childrenCount();
        }

        QString structTypeName;
        std::vector<DataNodeShared> children;
        //tsl::ordered_map<QString, QString> test;
    };

    using PayloadVariant = std::variant<LeafValue, CompositeData>;

    
    //DataNode() = default;
    DataNode(const QString &_name = QString(), NodeType _nodeType = NodeType::Leaf) : name(_name)
    {
        initPayload(_nodeType);
    }

    bool isLeaf() const
    {
        return std::holds_alternative<LeafValue>(payload);
    }
    bool isComposite() const
    {
        return std::holds_alternative<CompositeData>(payload);
    }

    QString getName()
    {
        return name;
    }
    QString getTypeName() const
    {
        if (auto leaf = tryGetLeafvalue())
        {
            return "todoGetTypeNameFromVariant";
        }
        else if (auto compData = tryGetCompositeData())
        {
            return compData->structTypeName;
        }

        SV_UNREACHABLE();
    };

    DataNodeShared tryGetParent()
    {
        return parent.lock();
    }

    
    const LeafValue* tryGetLeafvalue() const
    {
        return isLeaf() ? &std::get<LeafValue>(payload) : nullptr;
    }
    LeafValue* tryGetLeafvalue()
    {
        return removeConst( asConst(this)->tryGetLeafvalue() );
    }
    
    const CompositeData* tryGetCompositeData() const
    {
        return isComposite() ? &std::get<CompositeData>(payload) : nullptr;
    }
    CompositeData* tryGetCompositeData()
    {
        return removeConst( asConst(this)->tryGetCompositeData() );
    }

    //conforms to Qt models meaning of row: 
    //  - returns 0 when theres no parent (like there is imaginary root parent)
    //  - returns -1 if cant find 'this' in parent's children
    int rowIndexOfThisInParent()
    {
        if (auto parentPtr = parent.lock())
        {
            if (auto compdata = parentPtr->tryGetCompositeData())
            {
                const auto foundThis = std::find_if(compdata->children.cbegin(), compdata->children.cend(),
                                 [this](const auto &treeItem) {
                                     return treeItem.get() == this;
                                 });

                if (foundThis != compdata->children.cend())
                {
                    return std::distance(compdata->children.cbegin(), foundThis);
                }
            }
            
            return -1;
        }
        else return 0;
    }

    static QJsonValue toJSON(const DataNode &node)
    {
        QJsonObject obj;
        obj[nameKey] = node.name;

        if (node.isLeaf())
        {
            obj[valueKey] = "todo";// Serialization::any to json(getLeafvalue); ALSO TEMPLATED version without any
        }
        else if(auto compData = node.tryGetCompositeData())
        {
            QJsonArray childrenArray;
            for (auto &child : compData->children)
            {
                if (child)
                {
                    childrenArray.append( toJSON(*child) );
                }
            }

            obj[childrenKey] = childrenArray;
            obj[typeNameKey] = node.getTypeName();
        }
        //else unreachable

        return obj;
    }

    static DataNodeOpt fromJSON(QJsonValue jsonValue)
    {
        DataNode result;
        if (!jsonValue.isObject()) return {};

        auto json = jsonValue.toObject();

        if (auto name = jsonGetStringOpt(json, nameKey))
        {
            result.name = *name;
        }
        else return {};

        auto leafValue = json[valueKey];
        if (!leafValue.isUndefined()) //Then its Leaf node
        {
            result.initPayload(NodeType::Leaf);

            //todo.

            return result;
        }
        else if(auto childrenArray = jsonGetArrayOpt(json, childrenKey)) //Then its Composite node
        {
            result.initPayload(NodeType::Composite);
            CompositeData* resCompData = result.tryGetCompositeData();

            for (auto child : *childrenArray)
            {
                if (DataNodeOpt loadedChild = fromJSON(child))
                {
                    DataNodeShared childNode = std::make_shared<DataNode>();
                    *childNode = std::move(*loadedChild);

                    resCompData->children.push_back(childNode);
                }
                else
                {
                    //error
                    return {};
                }
            }
            
            //typename for Composite nodes is optional, i guess.
            if (auto typeName = jsonGetStringOpt(json, typeNameKey))
            {
                resCompData->structTypeName = *typeName;
            }

            return result;
        }
        else return {};
    }

    //These methods can not operate on wrong type, so will assert in case of mismatch:
    void addChild(DataNodeShared child)
    {
        SV_ASSERT(isComposite());

        if(auto compData = tryGetCompositeData())
        {
            compData->children.push_back(child);
            child->parent = weak_from_this();
        }
    }

    DataNodeShared tryGetChild(int idx)
    {
        auto compData = tryGetCompositeData();
        if (!compData)
        {
            //its a leaf, but its perfectly fine
            return {};
        }

        if (!compData->hasChild(idx))
        {
            SV_ERROR(logCategory, formatMsg(std::format("Child index {} out of bounds!", idx)));
            return {};
        }

        return compData->getChild(idx);
    }

    QString basicInfo()
    {
        return QString("DataNode{name=%1, type=%2, holds=%3}")
            .arg(name)
            .arg(isLeaf() ? "leaf":"comp")
            .arg(isLeaf() ? "leafvalue" : QString("%1 kids").arg(tryGetCompositeData()->childrenCount()));
    }
    std::string stdBasicInfo()
    {
        return basicInfo().toStdString();
    }

private:
    void initPayload(NodeType nodeType)
    {
        if (nodeType == NodeType::Leaf) payload = LeafValue();
        else payload = CompositeData();
    }

    std::string formatMsg(const std::string &msg)
    {
        return basicInfo().toStdString() + ": " + msg;
    }

private:
    QString name;

    PayloadVariant payload;

    DataNodeWeak parent;

private:
    static inline const QString nameKey = "name"; //mandatory
    static inline const QString valueKey = "value"; //mandatory for Leaf nodes
    static inline const QString typeNameKey = "typeName"; //optional for Composite nodes
    static inline const QString childrenKey = "children"; //mandatory for Composite nodes

    static inline const std::string logCategory = "DataNode";
};