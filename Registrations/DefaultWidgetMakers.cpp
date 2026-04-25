#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

//FOR FUTURE
template<typename ValueChangedSignal, typename ValueType>
void setupUpdatingNodeOnChanges(QWidget* widget, ValueChangedSignal widgetChangedSignal, DataNodeWeak weakNode)
{
    QObject::connect(widget, widgetChangedSignal, [weakNode](const ValueType& value)
    {
        constexpr std::string basicErr = "Widget changed value, but couldnt update its weak DataNode: ";

        if (auto nodeShared = weakNode.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                if (leaf->typeId() != qtTypeId<ValueType>(value))
                {
                    SV_WARN(std::format("Changing node leaf value type: from {} to {}",
                                        qVariantInfo(*leaf), qtTypeInfo<ValueType>())); 
                }

                *leaf = QVariant::fromValue( value );
            }
            else SV_ERROR(basicErr + "node is not leaf");
        }
        else SV_WARN(basicErr + "node has expired");
    });
}

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>(widgetMakerForQString, "std");
    system->registerWidgetMaker<LimitedDouble>(widgetMakerForLimitedDouble, QString("std"));
    system->registerWidgetMaker<LimitedDoubleVec>(widgetMakerForLimitedDoubleVec, QString("std"));
    system->registerWidgetMaker<LimitedInt>(widgetMakerForLimitedInt, QString("std"));
    system->registerWidgetMaker<LimitedIntVec>(widgetMakerForLimitedIntVec, QString("std"));
}

DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForQString(DataNodeShared leafWithQString, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<QString>(leafWithQString))
    {
        return {};
    }

    auto *widget = new QLineEdit(leafWithQString->tryGetLeafvalue()->toString());

    auto nodeWeak = DataNodeWeak(leafWithQString);

    //todo check if 3rd param...
    QObject::connect(widget, &QLineEdit::textChanged, widget, [nodeWeak](const QString &s)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue(s);
            }
        }
    });

    return new DataNodeWrapperWidget( widget, leafWithQString->getName(), options);
}

DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDouble>(leafWithLimitedDouble))
    {
        return {};
    }

    auto *widget = new LimitedValueWidget(leafWithLimitedDouble->tryGetLeafvalue()->value<LimitedDouble>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedDouble);

    QObject::connect(widget, &LimitedValueWidget::doubleValueChanged, widget, [nodeWeak](const LimitedDouble &v)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue(v);
            }
        }
    });

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

    QObject::connect(widget, &LimitedValueWidget::intValueChanged, widget, [nodeWeak](const LimitedInt &v)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue(v);
            }
        }
    });

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

    QObject::connect(widget, &LimitedValueVecWidget::valueChanged, widget, [nodeWeak](const LimitedIntOrDoubleVec &v)
    {
        if (!std::holds_alternative<LimitedDoubleVec>(v))
        {
            SV_ERROR("Widget for LimitedDoubleVec did emit LimitedIntVec, ignoring");
            return;
        }

        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue( std::get<LimitedDoubleVec>(v) );
            }
        }
    });

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

    QObject::connect(widget, &LimitedValueVecWidget::valueChanged, widget, [nodeWeak](const LimitedIntOrDoubleVec &v)
    {
        if (!std::holds_alternative<LimitedIntVec>(v))
        {
            SV_ERROR("Widget for LimitedIntVec did emit LimitedDoubleVec, ignoring");
            return;
        }

        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue( std::get<LimitedIntVec>(v) );
            }
        }
    });

    auto *wrapper = new DataNodeWrapperWidget( widget, leafWithLimitedIntVec->getName(), options);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}
