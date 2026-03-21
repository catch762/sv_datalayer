#include "XYPresetsWidget.h"



XYPresetsWidget::XYPresetsWidget(QWidget *parent) : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(2);

    presetsScrollAreaWidget = new HorizontalScrollAreaWidget(this);
    layout->addWidget(presetsScrollAreaWidget);
}