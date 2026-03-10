#include "DefaultWidgetMakers.h"
#include "WidgetMakerSystem.h"

void DefaultWidgetMakers::RegisterEverything(WidgetMakerSystem *system)
{
    system->registerWidgetMaker<QString>(widgetMakerForQString);
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
                *leaf = s;
            }
        }
    });

    return widget;
}
