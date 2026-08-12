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

class NodeWidget;

class DataNode;
SV_DECL_ALIASES(DataNode);
using ConstDataNodeShared = std::shared_ptr<const DataNode>;

// 'Composite pattern' tree node class to hold arbitrary data / nested structures.
// 
// Any DataNode is either:
//  1) isLeaf()      - just holds one variable (wrapped in std::any). Can not have children nodes.
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

    using LeafValue = std::any;
    struct CompositeData
    {
    public:
        DataNodeShared getChild(int idx)
        {
            return hasChild(idx) ? children[idx] : DataNodeShared();
        }
        ConstDataNodeShared getChild(int idx) const
        {
            return hasChild(idx) ? children[idx] : ConstDataNodeShared();
        }

        DataNodeShared getChild(const QString& name) 
        {
            for (auto& child : children)
            {
                if (child->getName() == name) return child;
            }

            return {};
        }

        ConstDataNodeShared getChild(const QString& name) const
        {
            for (auto& child : children)
            {
                if (child->getName() == name) return child;
            }

            return {};
        }

        int childrenCount() const
        {
            return children.size();
        }
        bool hasChild(int idx) const
        {
            return idx >= 0 && idx < childrenCount();
        }

        void setChildren(std::vector<DataNodeShared> theChildren, DataNodeWeak parent)
        {
            children = std::move(theChildren);
            for (auto& child : children)
            {
                SV_ASSERT(child);
                child->setParent(parent);
            }
        }

        //insertIndex must be [from 0 to childrenCount], inclusive
        void addChild(DataNodeShared child, DataNodeWeak parent, intOpt insertIndex = {})
        {
            SV_ASSERT(child);
            child->setParent(parent);

            if (insertIndex)
            {
                SV_ASSERT(*insertIndex >= 0 && *insertIndex <= children.size());
                children.insert(children.begin() + *insertIndex, child);
            }
            else
            {
                //dont care, push it to the end
                children.push_back(child);
            }
        }

        const std::vector<DataNodeShared>& getChildren() const
        {
            return children;
        }

    private:
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

        //avoiding double wrapping any in any:
        if constexpr (std::is_same_v<LeafValueT, std::any>)
        {
            *node->tryGetLeafvalue() = value;
        }
        else
        {
            *node->tryGetLeafvalue() = std::any(value);
        }

        return DataNodeShared(node);
    }
    
    static DataNodeShared makeComposite(const QString &_name = QString(), std::vector<DataNodeShared>&& children = {})
    {
        auto node = DataNodeShared(new DataNode(_name, NodeType::Composite));
        node->tryGetCompositeData()->setChildren( std::move(children), node );
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
            return QString::fromStdString(anyTypeNameOrMangled(*leaf));
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
                const auto& children = compdata->getChildren();

                const auto foundThis = std::find_if(children.cbegin(), children.cend(),
                                 [this](const auto &treeItem) {
                                     return treeItem.get() == this;
                                 });

                if (foundThis != children.cend())
                {
                    return std::distance(children.cbegin(), foundThis);
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
    void addChild(DataNodeShared child, intOpt insertIndex = {})
    {
        SV_ASSERT(isComposite());

        if(auto compData = tryGetCompositeData())
        {
            compData->addChild(child, shared_from_this(), insertIndex);
        }
    }

    DataNodeShared tryGetChild(int idx)
    {
        return std::const_pointer_cast<DataNode>(
            static_cast<const DataNode*>(this)->tryGetChild(idx)
        );
    }
    ConstDataNodeShared tryGetChild(int idx) const
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

    ConstDataNodeShared tryGetChild(const QString& name) const
    {
        auto compData = tryGetCompositeData();
        if (!compData)
        {
            //its a leaf, but its perfectly fine
            return {};
        }

        return compData->getChild(name);
    }
    DataNodeShared tryGetChild(const QString& name)
    {
        return std::const_pointer_cast<DataNode>(
            static_cast<const DataNode*>(this)->tryGetChild(name)
        );
    }

    //returns 0 for Leaf nodes
    int childrenCount() const
    {
        return isLeaf() ? 0 : tryGetCompositeData()->childrenCount();
    }

    std::string basicInfo(bool withAddress = false) const
    {
        return std::format("'{}': {}{}",
                name,
                isLeaf() ? tryGetLeafTypeName().value_or("error-no-leaf-type-name").toStdString() : std::format("{} kids", childrenCount()),
                withAddress ? std::format(", {:#X}", reinterpret_cast<std::uintptr_t>(this)) : ""
        );
    }

    std::string toString(bool withAddress = false, int currentLevel = 0) const
    {
        auto offset = std::string(currentLevel * 3, ' ');

        if (isLeaf())
        {
            return offset + basicInfo(withAddress);
        }
        else
        {
            auto result = offset + basicInfo(withAddress) + " {\n";

            for (auto child : tryGetCompositeData()->getChildren())
            {
                result = std::format("{}{}\n",
                    result,
                    child->toString(withAddress, currentLevel + 1)
                );
            }

            result = result + offset + "}";

            return result;
        }
    }

    template<typename T>
    bool isLeafWithType() const
    {
        if (auto leafValue = tryGetLeafvalue())
        {
            return anyHoldsType<T>(*leafValue);
        }
        else return false;
    }

    template<typename T>
    std::optional<T> tryGetLeafValueContent() const
    {
        if (!isLeafWithType<T>()) return {};
        return anyGetOpt<T>( *tryGetLeafvalue() );
    }

    //visitor is called on this and all inner subnodes
    static void iterateRecoursively(const DataNodeShared& node, const std::function<void(const DataNodeShared& node)>& visitor)
    {
        if (!node) return;

        visitor(node);

        if (auto *compData = node->tryGetCompositeData())
        {
            for (auto& child : compData->getChildren())
            {
                iterateRecoursively(child, visitor);
            }
        }
    }

    //DataNode returned has empty parent, but other than that its deep copy with identical content
    static DataNodeShared makeCopy(const DataNodeShared& node)
    {
        SV_ASSERT(node);
        if (node->isLeaf())
        {
            return DataNode::makeLeaf(node->getName(), *node->tryGetLeafvalue());
        }
        else if (auto *compdata = node->tryGetCompositeData())
        {
            std::vector<DataNodeShared> childrenCopies;
            childrenCopies.reserve(compdata->childrenCount());

            for (const auto& child : compdata->getChildren())
            {
                childrenCopies.push_back(DataNode::makeCopy(child));
            }

            return DataNode::makeComposite(node->getName(), std::move(childrenCopies));
        }
        else SV_UNREACHABLE();
    }

private:
    void initPayload(NodeType nodeType)
    {
        if (nodeType == NodeType::Leaf) payload = LeafValue();
        else payload = CompositeData();
    }

    std::string formatMsg(const std::string &msg) const
    {
        return basicInfo() + ": " + msg;
    }

    void setParent(DataNodeWeak theParent)
    {
        parent = theParent;
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

SV_DECL_STD_FORMATTER(DataNode,             obj.basicInfo());
SV_DECL_STD_FORMATTER(DataNodeShared,       obj             ? obj->basicInfo()        : "DataNode{nullptr}");
SV_DECL_STD_FORMATTER(ConstDataNodeShared,  obj             ? obj->basicInfo()        : "DataNode{nullptr}");
SV_DECL_STD_FORMATTER(ConstDataNodeWeak,    !obj.expired()  ? obj.lock()->basicInfo() : "DataNode{nullptr}");

namespace datanode_helpers
{
template<typename LeafValueT>
DataNodeShared dnleaf(const QString& name, const LeafValueT& val)
{
    return DataNode::makeLeaf(name, val);
}
inline DataNodeShared dncomp(const QString &name, std::vector<DataNodeShared>&& children)
{
    return DataNode::makeComposite(name, std::move(children));
}

};