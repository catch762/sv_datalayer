#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"


//Returns success
template<typename ValueType>
bool setNodeValue(DataNodeWeak weakNode, ValueType value)
{
    auto nodeShared = weakNode.lock();
    if (!nodeShared)
    {
        SV_ERROR("Couldnt set DataNode value, DataNodeWeak ptr expired");
        return false;
    }

    auto leaf = nodeShared->tryGetLeafvalue();
    if (!leaf)
    {
        SV_ERROR("Couldnt set DataNode value, its composite node, leaf expected");
        return false;
    }

    if (leaf->typeId() != qtTypeId<ValueType>())
    {
        SV_ERROR(std::format("Couldnt set DataNode value, type mismatch: node leaf value type: {} and value to set: {}",
            qVariantInfo(*leaf), qtTypeInfo<ValueType>()));
        return false;
    }

    //SV_LOG(std::format("upd val {}", value));
    *leaf = QVariant::fromValue(value);
    return true;
}

//returns success
//todo requires
template<typename LeafValueType, typename WidgetSetValueMethod, typename Widget>
bool setWidgetValueFromNode(Widget* widgetPtr, ConstDataNodeWeak weakNode)
{
    auto nodeShared = weakNode.lock();
    if (!nodeShared)
    {
        SV_ERROR("Couldnt set widget value, DataNodeWeak ptr expired");
        return false;
    }

    auto valOpt = nodeShared->tryGetLeafValueContent<LeafValueType>();
    if (!valOpt)
    {
        SV_ERROR(std::format("Couldnt set widget value, expected leaf node of type [{}], got {}",
            qtTypeName<LeafValueType>(), nodeShared));
        return false;
    }

    (*widget.*WidgetSetValueMethod)(*valOpt);

    //todo: is it needed?
    widget->update();

    return true;
}

/*template<typename ValueType,
         typename WidgetType,
         typename WidgetChangedSignal,
         typename WidgetGetValueMethod,
         typename WidgetSetValueMethod
>
class NodeWidgetHelper
{
public:
    bool createAndInitContentWidgets(   DataNodeShared leafNode,
                                        const QJsonObjectWithWidgetOptionsOpt& options = {},
                                        QWidget* widgetParent = nullptr )
    {
        if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<ValueType>(leafNode))
        {
            return false;
        }
    }
public:
    WidgetType* widget = nullptr;
};*/

class BoolNodeWidget : public NodeWidget
{
public:
    BoolNodeWidget( DataNodeShared node,
                    bool isForCompositeNode,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, isForCompositeNode, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<bool>(node)) return;

        widget = new QCheckBox();
        setWidgetValueFromNodeValue();

        trackValueChanges(widget, &QCheckBox::stateChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->isChecked());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<bool, &QCheckBox::setChecked>(widget, getNode());
    }

private:
    QCheckBox* widget = nullptr;
};

class BoolVecNodeWidget : public NodeWidget
{
public:
    BoolVecNodeWidget( DataNodeShared node,
                    bool isForCompositeNode,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, isForCompositeNode, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<BoolVec>(node)) return;

        widget = new BoolVecWidget(node->tryGetLeafValueContent<BoolVec>().value());
        setWidgetValueFromNodeValue();

        trackValueChanges(widget, &QCheckBox::stateChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->isChecked());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<bool, &QCheckBox::setChecked>(widget, getNode());
    }

private:
    QCheckBox* widget = nullptr;
};

/*class BoolVecNodeWidget : public NodeWidget
{
public:
    bool createAndInitContentWidgets(DataNodeShared leafWithBoolVec, const QJsonObjectWithWidgetOptionsOpt& options = {}) override
    {
        if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<BoolVec>(leafWithBoolVec))
        {
            return false;
        }

        widget = new QCheckBox();

        if (!setWidgetValueFromNodeValue())
        {
            return false;
        }

        connect(widget, &QCheckBox::stateChanged, this, &NodeWidget::valueChanged);

        return true;
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->isChecked());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<bool, &QCheckBox::setChecked>(widget, getNode());
    }

private:
    QCheckBox* widget = nullptr;
};*/


template <class DerivedNodeWidgetType>
NodeWidget* widgetMaker(DataNodeShared leaf, const QJsonObjectWithWidgetOptionsOpt& options)
{
    return new DerivedNodeWidgetType(leaf, false, leaf->getName(), options);
}

template <typename Type, class DerivedNodeWidgetType>
void registerWidgetMakerForType()
{
    WidgetMakerSystem::instance().registerWidgetMaker<Type>(
        widgetMaker<DerivedNodeWidgetType>
    );
}

void DefaultWidgetMakers::RegisterEverything()
{
    registerWidgetMakerForType<bool, BoolNodeWidget>();

    /*
    auto& system = WidgetMakerSystem::instance();

    system.registerWidgetMaker<QString>         (widgetMakerForQString);
    system.registerWidgetMaker<bool>            (widgetMakerForBool);
    system.registerWidgetMaker<BoolVec>         (widgetMakerForBoolVec);
    system.registerWidgetMaker<LimitedDouble>   (widgetMakerForLimitedDouble);
    system.registerWidgetMaker<LimitedDoubleVec>(widgetMakerForLimitedDoubleVec);
    system.registerWidgetMaker<LimitedInt>      (widgetMakerForLimitedInt);
    system.registerWidgetMaker<LimitedIntVec>   (widgetMakerForLimitedIntVec);
    system.registerWidgetMaker<Enum>            (widgetMakerForEnum);
    system.registerWidgetMaker<EnumVec>         (widgetMakerForEnumVec);
    */
}