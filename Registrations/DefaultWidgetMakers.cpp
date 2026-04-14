#include "DefaultWidgetMakers.h"
#include "WidgetLogic/WidgetMakerSystem.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>(widgetMakerForQString, "std");
    system->registerWidgetMaker<LimitedDouble>(widgetMakerForLimitedDouble, QString("std"));
    system->registerWidgetMaker<LimitedDoubleVec>(widgetMakerForLimitedDoubleVec, QString("std"));
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


DataNodeWrapperWidget* DefaultWidgetMakers::widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const QJsonObjectWithWidgetOptionsOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDoubleVec>(leafWithLimitedDoubleVec))
    {
        return {};
    }

    auto *widget = new LimitedDoubleVecWidget(leafWithLimitedDoubleVec->tryGetLeafvalue()->value<LimitedDoubleVec>(), options);

    auto nodeWeak = DataNodeWeak(leafWithLimitedDoubleVec);

    QObject::connect(widget, &LimitedDoubleVecWidget::valueChanged, widget, [nodeWeak](const LimitedDoubleVec &v)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                //SV_LOG("Saving LimitedDoubleVec to node...");
                *leaf = QVariant::fromValue(v);
            }
        }
    });

    auto *wrapper = new DataNodeWrapperWidget( widget, leafWithLimitedDoubleVec->getName(), options);
    widget->setupButtonsOnWrapperParent(wrapper, options);
    return wrapper;
}
