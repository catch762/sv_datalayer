#pragma once
#include <QCheckBox>
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include "DataTypesAndTheirWidgets/VectorOfAnyTypeWidgetCreator.h"
#include "sv_qtcommon.h"

class VectorWidget : public QWidget
{
    Q_OBJECT

signals:
    void valueChanged(const std::vector<LimitedInt>& value);

    DEFINE_VECTOR_OF_T_WIDGET(  VectorWidget,
                                LimitedInt,
                                LimitedValueWidget,
                                [](const LimitedInt& v)->LimitedValueWidget*{ return new LimitedValueWidget(v); },
                                [](const LimitedValueWidget* w)->const LimitedInt&{ return std::get<LimitedInt>(w->getValue()); },
                                GetValReturnType::ConstRef,
                                [](LimitedValueWidget* w, const LimitedInt& v){ w->setValue(v); },
                                LimitedValueWidget::valueChanged )
};

using BoolVec = std::vector<bool>;

inline QCheckBox* makeBoolWidget(bool value)
{
    auto widget = new QCheckBox();
    widget->setChecked(value);
    return widget;
}

class BoolVecWidget : public QWidget
{
    Q_OBJECT

signals:
    void valueChanged(const BoolVec& value);

    DEFINE_VECTOR_OF_T_WIDGET(  BoolVecWidget,
                                bool,
                                QCheckBox,
                                [](const bool& v)->QCheckBox*{ return makeBoolWidget(v); },
                                [](const QCheckBox* w)->bool{ return w->isChecked(); },
                                GetValReturnType::Value,
                                [](QCheckBox* w, const bool& v){ w->setChecked(v); },
                                QCheckBox::checkStateChanged )
};
