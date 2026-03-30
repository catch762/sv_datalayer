#include "DefaultWidgetMakers.h"
#include "WidgetMakerSystem.h"

#include "TypesAndWidgets/TypesAndWidgets.h"

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>(widgetMakerForQString, "std");
    system->registerWidgetMaker<LimitedDouble>(widgetMakerForLimitedDouble, QString("std"));
    system->registerWidgetMaker<LimitedDoubleVec>(widgetMakerForLimitedDoubleVec, QString("std"));
}

QWidget *DefaultWidgetMakers::widgetMakerForQString(DataNodeShared leafWithQString, const WidgetOptionsJsonOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<QString>(leafWithQString))
    {
        return nullptr;
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

    return widget;
}

QWidget *DefaultWidgetMakers::widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble, const WidgetOptionsJsonOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDouble>(leafWithLimitedDouble))
    {
        return nullptr;
    }

    auto *widget = new LimitedDoubleWidget(leafWithLimitedDouble->tryGetLeafvalue()->value<LimitedDouble>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedDouble);

    QObject::connect(widget, &LimitedDoubleWidget::valueChanged, widget, [nodeWeak](const LimitedDouble &v)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                *leaf = QVariant::fromValue(v);
            }
        }
    });

    return widget;
}


QWidget *DefaultWidgetMakers::widgetMakerForLimitedDoubleVec(DataNodeShared leafWithLimitedDoubleVec, const WidgetOptionsJsonOpt &options)
{
    if (!WidgetMakerSystem::checkIsProperLeafNodeForCreatingWidgetOfType<LimitedDoubleVec>(leafWithLimitedDoubleVec))
    {
        return nullptr;
    }

    auto *widget = new LimitedDoubleVecWidget(leafWithLimitedDoubleVec->tryGetLeafvalue()->value<LimitedDoubleVec>());

    auto nodeWeak = DataNodeWeak(leafWithLimitedDoubleVec);

    QObject::connect(widget, &LimitedDoubleVecWidget::valueChanged, widget, [nodeWeak](const LimitedDoubleVec &v)
    {
        if (auto nodeShared = nodeWeak.lock())
        {
            if (auto leaf = nodeShared->tryGetLeafvalue())
            {
                SV_LOG("Saving LimitedDoubleVec to node...");
                *leaf = QVariant::fromValue(v);
            }
        }
    });

    return widget;
}
