#pragma once
#include "EnumWidget.h"
#include "DataTypesAndTheirWidgets\VectorOfAnyTypeWidgetCreator.h"

class EnumVecWidget : public QWidget
{
    Q_OBJECT

signals:
    void valueChanged(const EnumVec& value);

    DEFINE_VECTOR_OF_T_WIDGET(  EnumVecWidget,
                                Enum,
                                EnumWidget,
                                [](const Enum& v)->EnumWidget*{ return new EnumWidget(v); },
                                [](const EnumWidget* w)->const Enum&{ return w->getValue(); },
                                GetValReturnType::ConstRef,
                                [](EnumWidget* w, const Enum& v){ w->setValue(v); },
                                &EnumWidget::valueChanged )
};