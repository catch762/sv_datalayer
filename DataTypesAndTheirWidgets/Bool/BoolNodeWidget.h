#pragma once
#include "WidgetLogic/NodeWidget.h"
#include <QCheckBox>

#include "WidgetLogic/WidgetMakerSystem.h"

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

class BoolNodeWidget : public NodeWidget
{
public:
    void createAndInitContentWidgets(DataNodeShared leafWithBool, const QJsonObjectWithWidgetOptionsOpt& options = {}) override
    {
        if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<bool>(leafWithBool))
        {
            //return {};
            return;
        }

        bool state = leafWithBool->tryGetLeafvalue()->value<bool>();

        widget = new QCheckBox();
        widget->setChecked(state);
        
        connect(widget, &QCheckBox::stateChanged, this, &NodeWidget::valueChanged);
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