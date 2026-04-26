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

                SV_LOG(std::format("upd val {}", qVariantInfo(*leaf)));

                *leaf = QVariant::fromValue( value );
            }
            else SV_ERROR(basicErr + "node is not leaf");
        }
        else SV_WARN(basicErr + "node has expired");
    });
}

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>         (widgetMakerForQString,            "std");
    system->registerWidgetMaker<bool>            (widgetMakerForBool,               "std");
    system->registerWidgetMaker<BoolVec>         (widgetMakerForBoolVec,            "std");
    system->registerWidgetMaker<LimitedDouble>   (widgetMakerForLimitedDouble,      "std");
    system->registerWidgetMaker<LimitedDoubleVec>(widgetMakerForLimitedDoubleVec,   "std");
    system->registerWidgetMaker<LimitedInt>      (widgetMakerForLimitedInt,         "std");
    system->registerWidgetMaker<LimitedIntVec>   (widgetMakerForLimitedIntVec,      "std");
}

DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<QString>(leafWithQString))
    {
        return {};
    }

    auto *widget = new QLineEdit(leafWithQString->tryGetLeafvalue()->toString());

    auto nodeWeak = DataNodeWeak(leafWithQString);

    setupUpdatingNodeOnChanges<const QString&>(widget, &QLineEdit::textChanged, nodeWeak);

    return new DataNodeWrapperWidget( widget, leafWithQString->getName(), options);
}

DataNodeWrapperWidget *DefaultWidgetMakers::widgetMakerForBool(DataNodeShared leafWithBool, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<bool>(leafWithBool))
    {
        return {};
    }

    auto *widget = makeBoolWidget(leafWithBool->tryGetLeafvalue()->value<bool>());

    auto nodeWeak = DataNodeWeak(leafWithBool);

    setupUpdatingNodeOnChanges<bool>(widget, &QCheckBox::toggled, nodeWeak);

    return new DataNodeWrapperWidget( widget, leafWithBool->getName(), options);
}

DataNodeWrapperWidget *DefaultWidgetMakers::widgetMakerForBoolVec(DataNodeShared leafWithBoolVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<BoolVec>(leafWithBoolVec))
    {
        return {};
    }

    auto *widget = new BoolVecWidget(leafWithBoolVec->tryGetLeafvalue()->value<BoolVec>());

    auto nodeWeak = DataNodeWeak(leafWithBoolVec);

    setupUpdatingNodeOnChanges<const BoolVec&>(widget, &BoolVecWidget::valueChanged, nodeWeak);

    return new DataNodeWrapperWidget( widget, leafWithBoolVec->getName(), options);
}

DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDouble>(leafWithLimitedDouble))
    {
        return {};
    }

    auto *widget = new LimitedValueWidget(leafWithLimitedDouble->tryGetLeafvalue()->value<LimitedDouble>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedDouble);

    setupUpdatingNodeOnChanges<const LimitedDouble&>(widget, &LimitedValueWidget::doubleValueChanged, nodeWeak);

    return new DataNodeWrapperWidget( widget, leafWithLimitedDouble->getName(), options);
}

DataNodeWrapperWidget *DefaultWidgetMakers::widgetMakerForLimitedInt(DataNodeShared leafWithLimitedInt, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedInt>(leafWithLimitedInt))
    {
        return {};
    }

    auto *widget = new LimitedValueWidget(leafWithLimitedInt->tryGetLeafvalue()->value<LimitedInt>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedInt);

    setupUpdatingNodeOnChanges<const LimitedInt&>(widget, &LimitedValueWidget::intValueChanged, nodeWeak);

    return new DataNodeWrapperWidget( widget, leafWithLimitedInt->getName(), options);
}

DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDoubleVec>(leafWithLimitedDoubleVec))
    {
        return {};
    }

    auto *widget = new LimitedValueVecWidget(leafWithLimitedDoubleVec->tryGetLeafvalue()->value<LimitedDoubleVec>(), options);

    auto nodeWeak = DataNodeWeak(leafWithLimitedDoubleVec);

    setupUpdatingNodeOnChanges<const LimitedDoubleVec&>(widget, &LimitedValueVecWidget::doubleValueChanged, nodeWeak);

    auto *wrapper = new DataNodeWrapperWidget( widget, leafWithLimitedDoubleVec->getName(), options);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}

DataNodeWrapperWidget *DefaultWidgetMakers::widgetMakerForLimitedIntVec(DataNodeShared leafWithLimitedIntVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedIntVec>(leafWithLimitedIntVec))
    {
        return {};
    }

    auto *widget = new LimitedValueVecWidget(leafWithLimitedIntVec->tryGetLeafvalue()->value<LimitedIntVec>(), options);

    auto nodeWeak = DataNodeWeak(leafWithLimitedIntVec);

    setupUpdatingNodeOnChanges<const LimitedIntVec&>(widget, &LimitedValueVecWidget::intValueChanged, nodeWeak);

    auto *wrapper = new DataNodeWrapperWidget( widget, leafWithLimitedIntVec->getName(), options);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}
