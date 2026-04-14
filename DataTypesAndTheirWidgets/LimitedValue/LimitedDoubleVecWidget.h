#pragma once
#include "LimitedValueWidget.h"
#include "DataTypesAndTheirWidgets/DataTypesAndTheirWidgets.h"
#include <QStackedLayout>
#include "WidgetLogic/WidgetDefs.h"

class LimitedDoublesWidget;
class XYPadWithPresetsWidget;
class DataNodeWrapperWidget;

//todo on value change check validation

class LimitedDoubleVecWidget : public QFrame
{
    Q_OBJECT
public:
    enum Mode
    {
        ShowJustLimitedValueWidgets,
        ShowXYPad
    };

    LimitedDoubleVecWidget(const LimitedDoubleVec& vec,
                           const QJsonObjectWithWidgetOptionsOpt& options = {},
                           QWidget *parent = nullptr);

    const LimitedDoubleVec& getValue() const;

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedDoubleVec& newValue);

    QJsonObjectWithWidgetOptionsOpt makeOptions() const;
    void setupButtonsOnWrapperParent(DataNodeWrapperWidget* wrapper, const QJsonObjectWithWidgetOptionsOpt& options = {} );

signals:
    void valueChanged(const LimitedDoubleVec &val);

private:
    //no signals will be emitted from views
    void setViewsStateFromValue(const LimitedDoubleVec& value);
    void setMode(Mode mode);

private:
    LimitedDoubleVec value;
    QVBoxLayout*            layout          = nullptr;
    LimitedDoublesWidget*       slidersView = nullptr;
    XYPadWithPresetsWidget*     xyPadView   = nullptr;

    // Button to select views, it will be created on a DataNodeWrapperWidget, but stored here.
    // default state: 'checked = false' which means slidersView is visible
    QPointer<QPushButton> viewSelectorWrapperButton;

    static inline const QString modeIsXYKey = "modeIsXY";
};

Q_DECLARE_METATYPE(LimitedDoubleVecWidget*);

template<>
class Serializer< LimitedDoubleVecWidget* >
{
public:
    using WidgetPtr = LimitedDoubleVecWidget*;

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