#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/NodeWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
//#include "WidgetLogic/WidgetUtils.h"

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

    (widget->*WidgetSetValueMethod)(*valOpt);

    //todo: is it needed?
    widget->update();

    return true;
}

template<typename ValueType,
         typename WidgetType,
         auto WidgetChangedSignal,
         auto WidgetGetValueMethod,
         auto WidgetSetValueMethod
>
class NodeWidgetHelper
{
public:
    NodeWidgetHelper(NodeWidget* _widgetWrapper)
    {
        widgetWrapper = _widgetWrapper;
        SV_ASSERT(widgetWrapper);
        SV_ASSERT(nodeHasProperContent());
    }

    bool nodeHasProperContent()
    {
        return nodeSuitableForWidgetOfType<ValueType>(node().lock());
    }

    void trackWidgetValueChanges()
    {
        widgetWrapper->trackValueChanges(widget, WidgetChangedSignal);
    }

    bool setNodeValueFromWidgetValue()
    {
        return setNodeValue(node(), (widget->*WidgetGetValueMethod)());
    }

    bool setWidgetValueFromNodeValue()
    {
        return setWidgetValueFromNode<ValueType, WidgetSetValueMethod>(widget, node());
    }

    //You call either of 2 variants below, or init widget manually:
    //1) When widget doesnt have constructor which lets you pass value in it.
    void initWidgetAndSetValOnIt()
    {
        widget = new WidgetType();
        setWidgetValueFromNodeValue();

        widgetWrapper->addContentWidget(widget);
        trackWidgetValueChanges();
    }
    //2) When widget does have constructor WidgetType(ValueType initialVal)
    void initWidgetWithInitialVal()
    {
        SV_ASSERT(nodeHasProperContent());

        widget = new WidgetType(node().lock()->tryGetLeafValueContent<ValueType>().value());

        widgetWrapper->addContentWidget(widget);
        trackWidgetValueChanges();
    }

private:
    DataNodeWeak node()
    {
        return widgetWrapper->getNode();
    }
private:
    NodeWidget* widgetWrapper = nullptr;
public:
    WidgetType* widget = nullptr;
};

class BoolNodeWidget : public NodeWidget
{
public:
    BoolNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent), helper(this)
    {
        helper.initWidgetAndSetValOnIt();
    }

    bool setNodeValueFromWidgetValue() override
    {
        return helper.setNodeValueFromWidgetValue();
    }

    bool setWidgetValueFromNodeValue() override
    {
        return helper.setWidgetValueFromNodeValue();
    }

private:
    NodeWidgetHelper<bool, 
                     QCheckBox, 
                     &QCheckBox::stateChanged, 
                     &QCheckBox::isChecked, 
                     &QCheckBox::setChecked> helper;
};

class BoolVecNodeWidget : public NodeWidget
{
public:
    BoolVecNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent), helper(this)
    {
        helper.initWidgetWithInitialVal();
    }

    bool setNodeValueFromWidgetValue() override
    {
        return helper.setNodeValueFromWidgetValue();
    }

    bool setWidgetValueFromNodeValue() override
    {
        return helper.setWidgetValueFromNodeValue();
    }

private:
    NodeWidgetHelper<BoolVec, 
                     BoolVecWidget, 
                     &BoolVecWidget::valueChanged, 
                     &BoolVecWidget::getValue, 
                     &BoolVecWidget::setValue> helper;
};

class QStringNodeWidget : public NodeWidget
{
public:
    QStringNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent), helper(this)
    {
        helper.initWidgetWithInitialVal();
    }

    bool setNodeValueFromWidgetValue() override
    {
        return helper.setNodeValueFromWidgetValue();
    }

    bool setWidgetValueFromNodeValue() override
    {
        return helper.setWidgetValueFromNodeValue();
    }

private:
    NodeWidgetHelper<QString, 
                     QLineEdit, 
                     &QLineEdit::textChanged, 
                     &QLineEdit::text, 
                     &QLineEdit::setText> helper;
};

class LimitedIntNodeWidget : public NodeWidget
{
public:
    LimitedIntNodeWidget( DataNodeShared node,
                    const QString& name = {},
                    const QJsonObjectWithWidgetOptionsOpt& options = {}, 
                    QWidget* parent = nullptr )
        : NodeWidget(node, name, options, parent), helper(this)
    {
        helper.initWidgetWithInitialVal();
    }

    bool setNodeValueFromWidgetValue() override
    {
        //widget returns variant double/int, so have to check it first

        if (auto* intValue = std::get_if<LimitedInt>(&helper.widget->getValue()))
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
        return helper.setWidgetValueFromNodeValue();
    }

private:
    NodeWidgetHelper<LimitedInt,
                     LimitedValueWidget,
                     &LimitedValueWidget::valueChanged,
                     &LimitedValueWidget::getValue,
                     &LimitedValueWidget::setValue> helper;
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