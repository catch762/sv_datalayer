#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"


//Returns success
template<typename ValueType>
bool setNodeValue(DataNodeWeak weakNode, const ValueType& value)
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
template<typename LeafValueType, auto WidgetSetValueMethod, typename Widget>
bool setWidgetValueFromNode(Widget* widget, ConstDataNodeWeak weakNode)
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
    using ValueT = bool;
public:
    BoolNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<ValueT>(node)) return;

        widget = new QCheckBox(this);
        setWidgetValueFromNodeValue();

        trackValueChanges(widget, &QCheckBox::stateChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->isChecked());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<ValueT, &QCheckBox::setChecked>(widget, getNode());
    }

private:
    QCheckBox* widget = nullptr;
};

class BoolVecNodeWidget : public NodeWidget
{
    using ValueT = BoolVec;
public:
    BoolVecNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<ValueT>(node)) return;

        widget = new BoolVecWidget(node->tryGetLeafValueContent<ValueT>().value(), this);
        //setWidgetValueFromNodeValue();

        trackValueChanges(widget, &BoolVecWidget::valueChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->getValue());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<ValueT, &BoolVecWidget::setValue>(widget, getNode());
    }

private:
    BoolVecWidget* widget = nullptr;
};

class QStringNodeWidget : public NodeWidget
{
    using ValueT = QString;
public:
    QStringNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<ValueT>(node)) return;

        widget = new QLineEdit(node->tryGetLeafValueContent<ValueT>().value(), this);
        //setWidgetValueFromNodeValue();

        trackValueChanges(widget, &QLineEdit::textChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        return setNodeValue(getNode(), widget->text());
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<ValueT, &QLineEdit::setText>(widget, getNode());
    }

private:
    QLineEdit* widget = nullptr;
};

class LimitedIntNodeWidget : public NodeWidget
{
    using ValueT    = LimitedInt;
    using WidgetT   = LimitedValueWidget;

public:
    LimitedIntNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent)
    {
        if (!nodeSuitableForWidgetOfType<ValueT>(node)) return;

        widget = new WidgetT(node->tryGetLeafValueContent<ValueT>().value(), this);
        //setWidgetValueFromNodeValue();

        trackValueChanges(widget, &WidgetT::valueChanged);
    }

    bool setNodeValueFromWidgetValue() override
    {
        if (auto* intValue = std::get_if<ValueT>(&widget->getValue()))
        {
            return setNodeValue(getNode(), *intValue);
        }
        else
        {
            SV_ERROR("Cant update node: LimitedIntNodeWidget didnt expect to find LimitedDouble in a widget");
            return false;
        }
    }

    bool setWidgetValueFromNodeValue() override
    {
        return setWidgetValueFromNode<ValueT, &WidgetT::setValue>(widget, getNode());
    }

private:
    WidgetT* widget = nullptr;
};











template <class DerivedNodeWidgetType>
NodeWidget* widgetMaker(DataNodeShared leaf, const QJsonObjectWithWidgetOptionsOpt& options)
{
    return new DerivedNodeWidgetType(leaf, leaf->getName(), options, nullptr);
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
    registerWidgetMakerForType<bool,        BoolNodeWidget>();
    registerWidgetMakerForType<BoolVec,     BoolVecNodeWidget>();
    registerWidgetMakerForType<QString,     QStringNodeWidget>();
    registerWidgetMakerForType<LimitedInt,  LimitedIntNodeWidget>();

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