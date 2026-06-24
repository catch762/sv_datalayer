#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

//Note that value type may be const reference, or may be raw value, depending on signal
template<typename ValueType, typename WidgetT, typename ValueChangedSignal>
void setupUpdatingNodeOnChanges(WidgetT* widget, ValueChangedSignal widgetChangedSignal, DataNodeWeak weakNode)
{
    QObject::connect(widget, widgetChangedSignal, [weakNode](ValueType value)
    {
        static const std::string basicErr = "Widget changed value, but couldnt update its weak DataNode: ";

        if (auto nodeShared = weakNode.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                if (leaf->typeId() != qtTypeId<ValueType>())
                {
                    SV_WARN(std::format("Changing node leaf value type: from {} to {}",
                                        qVariantInfo(*leaf), qtTypeInfo<ValueType>())); 
                }

                //SV_LOG(std::format("upd val {}", value));
                *leaf = QVariant::fromValue( value );

                //TODO: WidgetsForNodeManager.update all.
            }
            else SV_ERROR(basicErr + "node is not leaf");
        }
        else SV_WARN(basicErr + "node has expired");
    });
}

template<typename ValueType, typename WidgetT, auto WidgetSetValueMethod>
void updateWidgetFromNodeState(QWidget* qwidget, ConstDataNodeWeak nodeWeak)
{
    static const std::string basicErr = "Error while trying to update widget from DataNode: ";

    auto* widget = qobject_cast<WidgetT*>(qwidget);
    if (!widget)
    {
        SV_ERROR( std::format("{}couldnt convert QWidget to {}", basicErr, qtTypeInfo<WidgetT>()) );
        return;
    }

    auto nodeShared = nodeWeak.lock();
    if (!nodeShared)
    {
        SV_ERROR(basicErr + "node has expired");
        return;
    }

    auto leaf = nodeShared->tryGetLeafvalue();
    if (!leaf)
    {
        SV_ERROR(basicErr + "node is not leaf");
        return;
    }

    if (leaf->typeId() != qtTypeId<ValueType>())
    {
        SV_ERROR(std::format("{}expected leaf value type {} and got {}",
                            basicErr, qtTypeInfo<ValueType>(), qVariantInfo(*leaf) ));
        return;
    }

    SV_LOG(std::format("{} widget updated", qtTypeInfo<ValueType>()));
    (*widget.*WidgetSetValueMethod)(leaf->value<ValueType>());
    widget->update();
}

void DefaultWidgetMakers::RegisterEverything()
{
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
}

WidgetWrapper* DefaultWidgetMakers::widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<QString>(leafWithQString))
    {
        return {};
    }

    auto *widget = new QLineEdit(leafWithQString->tryGetLeafvalue()->toString());

    auto nodeWeak = DataNodeWeak(leafWithQString);

    setupUpdatingNodeOnChanges<const QString&>(widget, &QLineEdit::textChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   QString,
                                                QLineEdit,
                                                &QLineEdit::setText>;

    return new WidgetWrapper( widget, leafWithQString->getName(), options, updater);
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForBool(DataNodeShared leafWithBool, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<bool>(leafWithBool))
    {
        return {};
    }

    auto *widget = makeBoolWidget(leafWithBool->tryGetLeafvalue()->value<bool>());

    auto nodeWeak = DataNodeWeak(leafWithBool);

    setupUpdatingNodeOnChanges<bool>(widget, &QCheckBox::toggled, nodeWeak);

    auto updater = updateWidgetFromNodeState<   bool,
                                                QCheckBox,
                                                &QCheckBox::setChecked>;

    return new WidgetWrapper( widget, leafWithBool->getName(), options, updater);
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForBoolVec(DataNodeShared leafWithBoolVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<BoolVec>(leafWithBoolVec))
    {
        return {};
    }

    auto *widget = new BoolVecWidget(leafWithBoolVec->tryGetLeafvalue()->value<BoolVec>());

    auto nodeWeak = DataNodeWeak(leafWithBoolVec);

    setupUpdatingNodeOnChanges<const BoolVec&>(widget, &BoolVecWidget::valueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   BoolVec,
                                                BoolVecWidget,
                                                &BoolVecWidget::setValue>;

    return new WidgetWrapper( widget, leafWithBoolVec->getName(), options, updater);
}

WidgetWrapper* DefaultWidgetMakers::widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDouble>(leafWithLimitedDouble))
    {
        return {};
    }

    auto *widget = new LimitedValueWidget(leafWithLimitedDouble->tryGetLeafvalue()->value<LimitedDouble>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedDouble);

    setupUpdatingNodeOnChanges<const LimitedDouble&>(widget, &LimitedValueWidget::doubleValueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   LimitedDouble,
                                                LimitedValueWidget,
                                                &LimitedValueWidget::setValue>;

    return new WidgetWrapper( widget, leafWithLimitedDouble->getName(), options, updater);
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForLimitedInt(DataNodeShared leafWithLimitedInt, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedInt>(leafWithLimitedInt))
    {
        return {};
    }

    auto *widget = new LimitedValueWidget(leafWithLimitedInt->tryGetLeafvalue()->value<LimitedInt>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedInt);

    setupUpdatingNodeOnChanges<const LimitedInt&>(widget, &LimitedValueWidget::intValueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   LimitedInt,
                                                LimitedValueWidget,
                                                &LimitedValueWidget::setValue>;

    return new WidgetWrapper( widget, leafWithLimitedInt->getName(), options, updater);
}

WidgetWrapper* DefaultWidgetMakers::widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDoubleVec>(leafWithLimitedDoubleVec))
    {
        return {};
    }

    auto *widget = new LimitedValueVecWidget(leafWithLimitedDoubleVec->tryGetLeafvalue()->value<LimitedDoubleVec>(), options);

    auto nodeWeak = DataNodeWeak(leafWithLimitedDoubleVec);

    setupUpdatingNodeOnChanges<const LimitedDoubleVec&>(widget, &LimitedValueVecWidget::doubleValueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   LimitedDoubleVec,
                                                LimitedValueVecWidget,
                                                &LimitedValueVecWidget::setValue>;

    auto *wrapper = new WidgetWrapper( widget, leafWithLimitedDoubleVec->getName(), options, updater);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForLimitedIntVec(DataNodeShared leafWithLimitedIntVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedIntVec>(leafWithLimitedIntVec))
    {
        return {};
    }

    auto *widget = new LimitedValueVecWidget(leafWithLimitedIntVec->tryGetLeafvalue()->value<LimitedIntVec>(), options);

    auto nodeWeak = DataNodeWeak(leafWithLimitedIntVec);

    setupUpdatingNodeOnChanges<const LimitedIntVec&>(widget, &LimitedValueVecWidget::intValueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   LimitedIntVec,
                                                LimitedValueVecWidget,
                                                &LimitedValueVecWidget::setValue>;

    auto *wrapper = new WidgetWrapper( widget, leafWithLimitedIntVec->getName(), options, updater);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForEnum(DataNodeShared leafWithEnum, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<Enum>(leafWithEnum))
    {
        return {};
    }

    auto *widget = new EnumWidget();
    widget->setValue(leafWithEnum->tryGetLeafvalue()->value<Enum>());

    auto nodeWeak = DataNodeWeak(leafWithEnum);

    setupUpdatingNodeOnChanges<const Enum&>(widget, &EnumWidget::valueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   Enum,
                                                EnumWidget,
                                                &EnumWidget::setValue>;

    return new WidgetWrapper( widget, leafWithEnum->getName(), options, updater);
}

WidgetWrapper *DefaultWidgetMakers::widgetMakerForEnumVec(DataNodeShared leafWithEnumVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<EnumVec>(leafWithEnumVec))
    {
        return {};
    }

    auto *widget = new EnumVecWidget(leafWithEnumVec->tryGetLeafvalue()->value<EnumVec>());

    auto nodeWeak = DataNodeWeak(leafWithEnumVec);

    setupUpdatingNodeOnChanges<const EnumVec&>(widget, &EnumVecWidget::valueChanged, nodeWeak);

    auto updater = updateWidgetFromNodeState<   EnumVec,
                                                EnumVecWidget,
                                                &EnumVecWidget::setValue>;

    return new WidgetWrapper( widget, leafWithEnumVec->getName(), options, updater);
}
