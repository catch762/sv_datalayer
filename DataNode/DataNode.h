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

#include <QPointer>

class DataNodeWrapperWidget;

class DataNode;
SV_DECL_ALIASES(DataNode);
using ConstDataNodeShared = std::shared_ptr<const DataNode>;

// 'Composite pattern' tree node class to hold arbitrary data / nested structures.
// 
// Any DataNode is either:
//  1) isLeaf()      - just holds one variable (wrapped in QVariant). Can not have children nodes.
//  2) isComposite() - the opposite: holds no data, but does have a list of children nodes.
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
        ConstDataNodeShared getChild(int idx) const
        {
            return hasChild(idx) ? children[idx] : ConstDataNodeShared();
        }
        int childrenCount() const
        {
            return children.size();
        }
        bool hasChild(int idx) const
        {
            return idx >= 0 && idx < childrenCount();
        }

        std::vector<DataNodeShared> children;
        //tsl::ordered_map<QString, QString> test;
    };

    using PayloadVariant = std::variant<LeafValue, CompositeData>;
    
    //DataNode() = default;
    DataNode(const QString &_name = QString(), NodeType _nodeType = NodeType::Leaf) : name(_name)
    {
        initPayload(_nodeType);
    }

    template<typename LeafValueT>
    static DataNodeShared makeLeaf(const QString &_name = QString(), const LeafValueT& value = {})
    {
        auto node = new DataNode(_name, NodeType::Leaf);

        //avoiding double wrapping qvariant in qvariant:
        if constexpr (std::is_same_v<LeafValueT, QVariant>)
        {
            *node->tryGetLeafvalue() = value;
        }
        else
        {
            *node->tryGetLeafvalue() = QVariant::fromValue(value);
        }

        return DataNodeShared(node);
    }
    
    static DataNodeShared makeComposite(const QString &_name = QString(), std::vector<DataNodeShared>&& children = {})
    {
        auto node = DataNodeShared(new DataNode(_name, NodeType::Composite));
        node->tryGetCompositeData()->children = std::move(children);
        return node;
    }

    //Returns added node.
    //If this node is not Composite, it will trigger assert.
    template<typename LeafValueT>
    DataNodeShared addLeaf(const QString &_name = QString(), const LeafValueT& value = {})
    {
        auto node = makeLeaf(_name, value);
        addChild(node);
        return node;
    }

    //Returns added node.
    //If this node is not Composite, it will trigger assert.
    DataNodeShared addComposite(const QString &_name = QString())
    {
        auto node = makeComposite(_name);
        addChild(node);
        return node;
    }

    bool isLeaf() const
    {
        return std::holds_alternative<LeafValue>(payload);
    }
    bool isComposite() const
    {
        return std::holds_alternative<CompositeData>(payload);
    }

    QString getName() const
    {
        return name;
    }
    QStringOpt tryGetLeafTypeName() const
    {
        if (auto leaf = tryGetLeafvalue())
        {
            return leaf->typeName();
        }
        
        return {};
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


    // When DataNode gets serialized or deserialized, you can use corresponding
    // action to do something else (inject or read additional data, for example.)
    // Return value in both: means 'success'. If false is returned, it means entire operation is failed.
    // '_level': 0 is root, 1 is child, 2 is granchild, etc
    using OnJsonCreatedFromNodeAction = std::function<bool(ConstDataNodeShared node,       QJsonObject &jsonOfNode, int _level)>;
    using OnNodeCreatedFromJsonAction = std::function<bool(     DataNodeShared node, const QJsonObject &jsonOfNode, int _level)>;

    QJsonObjectOpt toJSON(OnJsonCreatedFromNodeAction onJsonCreatedAction = nullptr, int _level = 0) const;
    static DataNodeShared fromJSON(QJsonValue jsonValue, OnNodeCreatedFromJsonAction onNodeCreatedAction = nullptr, int _level = 0);

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
        return std::const_pointer_cast<DataNode>(
            static_cast<const DataNode*>(this)->tryGetChild(idx)
        );
    }

    std::shared_ptr<const DataNode> tryGetChild(int idx) const
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

    //returns 0 for Leaf nodes
    int childrenCount() const
    {
        return isLeaf() ? 0 : tryGetCompositeData()->childrenCount();
    }

    QString basicInfo() const
    {
        return QString("DataNode{%1, name=%2, holds=%3}")
            .arg(isLeaf() ? "leaf":"comp")
            .arg(name)
            .arg(isLeaf() ? *tryGetLeafTypeName() : QString("%1 kids").arg(tryGetCompositeData()->childrenCount()));
    }
    std::string stdBasicInfo() const
    {
        return basicInfo().toStdString();
    }

    template<typename T>
    bool isLeafWithType()
    {
        if (auto leafValue = tryGetLeafvalue())
        {
            return qtTypeId<T>() == leafValue->typeId();
        }
        else return false;
    }

private:
    void initPayload(NodeType nodeType)
    {
        if (nodeType == NodeType::Leaf) payload = LeafValue();
        else payload = CompositeData();
    }

    std::string formatMsg(const std::string &msg) const
    {
        return basicInfo().toStdString() + ": " + msg;
    }

private:
    QString name;

    PayloadVariant payload;

    DataNodeWeak parent;

private:
    static inline const QString nameKey     = "name";       //mandatory for all
    static inline const QString valueKey    = "leafValue";  //mandatory for Leaf nodes
    static inline const QString childrenKey = "children";   //mandatory for Composite nodes

    static inline const std::string logCategory = "DataNode";
};

SV_DECL_STD_FORMATTER(DataNode,          obj.stdBasicInfo());
SV_DECL_STD_FORMATTER(DataNodeShared,    obj        ? obj->stdBasicInfo()        : "DataNode{nullptr}");
SV_DECL_STD_FORMATTER(ConstDataNodeWeak, obj.lock() ? obj.lock()->stdBasicInfo() : "DataNode{nullptr}");