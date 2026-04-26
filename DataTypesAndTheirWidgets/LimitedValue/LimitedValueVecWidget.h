#pragma once
#include "LimitedValueWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include <QStackedLayout>
#include "WidgetLogic/WidgetDefs.h"

class LimitedValueVecSlidersWidget;
class XYPadWithPresetsWidget;
class DataNodeWrapperWidget;

//todo on value change check validation

class LimitedValueVecWidget : public QFrame
{
    Q_OBJECT
public:
    enum Mode
    {
        ShowJustLimitedValueWidgets,
        ShowXYPad
    };

    LimitedValueVecWidget(const LimitedIntOrDoubleVec& vec,
                           const QJsonObjectWithWidgetOptionsOpt& options = {},
                           QWidget *parent = nullptr);

    const LimitedIntOrDoubleVec& getValue() const;

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedIntOrDoubleVec& newValue);

    QJsonObjectWithWidgetOptionsOpt makeOptions() const;
    void setupButtonsOnWrapperParent(DataNodeWrapperWidget* wrapper, const QJsonObjectWithWidgetOptionsOpt& options = {} );

signals:
    void valueChanged(const LimitedIntOrDoubleVec &val);
    void intValueChanged(const LimitedIntVec& val);
    void doubleValueChanged(const LimitedDoubleVec& val);

private:
    //no signals will be emitted from views
    void setViewsStateFromValue(const LimitedIntOrDoubleVec& value);
    void setMode(Mode mode);

private:
    LimitedIntOrDoubleVec value;

    QVBoxLayout*            layout          = nullptr;
    LimitedValueVecSlidersWidget*       slidersView = nullptr;
    XYPadWithPresetsWidget*     xyPadView   = nullptr;

    // Button to select views, it will be created on a DataNodeWrapperWidget, but stored here.
    // default state: 'checked = false' which means slidersView is visible
    QPointer<QPushButton> viewSelectorWrapperButton;

    static inline const QString modeIsXYKey = "modeIsXY";
};

Q_DECLARE_METATYPE(LimitedValueVecWidget*);

template<>
class Serializer< LimitedValueVecWidget* >
{
public:
    using WidgetPtr = LimitedValueVecWidget*;

    QJsonValue toJson(const WidgetPtr& value)
    {
        if (auto options = value->makeOptions()) return *options;
        return {};
    }

    std::optional<WidgetPtr> fromJson(const QJsonValue& json)
    {
        SV_UNREACHABLE();
    }
};