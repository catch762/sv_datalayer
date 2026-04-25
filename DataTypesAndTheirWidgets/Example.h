#pragma once
#include "VectorOfAnyTypeWidgetCreator.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

class VectorWidget : public QWidget
{
    Q_OBJECT

signals:
    void valueChanged(const std::vector<LimitedInt>& value);

    DEFINE_VECTOR_OF_T_WIDGET(  LimitedInt,
                                LimitedValueWidget,
                                [](const LimitedInt&)->LimitedValueWidget*{ return new LimitedValueWidget(LimitedInt{}); },
                                [](const LimitedValueWidget* w)->const LimitedInt&{ return std::get<LimitedInt>(w->getValue()); },
                                [](LimitedValueWidget* w, const LimitedInt& v){ w->setValue(v); },
                                LimitedValueWidget::valueChanged )
};