#include "DefaultWidgetMakers.h"
#include "WidgetMakerSystem.h"

#include "TypesAndWidgets/TypesAndWidgets.h"

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>(widgetMakerForQString);
    system->registerWidgetMaker<LimitedDouble>(widgetMakerForLimitedDouble);
}

QWidget *DefaultWidgetMakers::widgetMakerForQString(DataNodeShared leafWithQString)
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

QWidget *DefaultWidgetMakers::widgetMakerForLimitedDouble(DataNodeShared leafWithLimitedDouble)
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
