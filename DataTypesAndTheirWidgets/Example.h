#pragma once
#include "VectorOfAnyTypeWidgetCreator.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"

DEFINE_VECTOR_OF_T_WIDGET(  VectorWidget,
                            LimitedInt,
                            LimitedValueWidget,
                            []()->LimitedValueWidget*{ return new LimitedValueWidget(LimitedInt{}); },
                            [](const LimitedValueWidget* w)->const LimitedInt&{ return std::get<LimitedInt>(w->getValue()); },
                            [](LimitedValueWidget* w, const LimitedInt& v){ w->setValue(v); },
                            LimitedValueWidget::valueChanged )