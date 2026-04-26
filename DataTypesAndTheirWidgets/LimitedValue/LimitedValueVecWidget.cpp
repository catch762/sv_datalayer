#include "LimitedValueVecWidget.h"
#include "Internal/LimitedValueVecSlidersWidget.h"
#include "Internal/XYPadWithPresetsWidget.h"
#include "WidgetLogic/DataNodeWrapperWidget.h"
#include <QToolButton>


LimitedValueVecWidget::LimitedValueVecWidget(const LimitedIntOrDoubleVec& initialValue,
    const QJsonObjectWithWidgetOptionsOpt& options, QWidget *parent)
    : QFrame(parent)
{
    value = initialValue;

    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout, 2, 0);

    slidersView = new LimitedValueVecSlidersWidget(value, this);
    {
        connect(slidersView, &LimitedValueVecSlidersWidget::valueChanged, this, [this](auto &val)
        {
            setValue(val);
        });

        layout->addWidget(slidersView);
    }

    xyPadView = new XYPadWithPresetsWidget(this);
    {
        xyPadView->setVisible(false);
        if (options)
        {
            xyPadView->restoreFromOptions(*options);
        }
        layout->addWidget(xyPadView);
    }    
}

const LimitedIntOrDoubleVec& LimitedValueVecWidget::getValue() const
{
    return value;
}

void LimitedValueVecWidget::setValue(const LimitedIntOrDoubleVec& newValue)
{
    //static int i = 0;
    //SV_LOG("Master: setValue " + std::to_string(i++));

    value = newValue;

    setViewsStateFromValue(value);

    emit valueChanged(value);

    if (std::holds_alternative<LimitedIntVec>(value))
    {
        emit intValueChanged(std::get<LimitedIntVec>(value));
    }
    else if (std::holds_alternative<LimitedDoubleVec>(value))
    {
        emit doubleValueChanged(std::get<LimitedDoubleVec>(value));
    }
    else
    {
        SV_UNREACHABLE();
    }
}

QJsonObjectWithWidgetOptionsOpt LimitedValueVecWidget::makeOptions() const
{
    QJsonObjectWithWidgetOptions options = xyPadView->makeOptions().value_or(QJsonObjectWithWidgetOptions());

    if (viewSelectorWrapperButton)
    {
        options[modeIsXYKey] = viewSelectorWrapperButton->isChecked();
    }

    return options;
}

void LimitedValueVecWidget::setupButtonsOnWrapperParent(DataNodeWrapperWidget *wrapper, const QJsonObjectWithWidgetOptionsOpt& options)
{
    viewSelectorWrapperButton = makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                                                                     QIcon::ThemeIcon::FormatJustifyLeft);
    viewSelectorWrapperButton->setChecked(false);

    connect(viewSelectorWrapperButton, &QPushButton::toggled, this, [this](bool checked)
    {
        setMode(checked ? Mode::ShowXYPad : Mode::ShowJustLimitedValueWidgets);
    });

    wrapper->getStripeButtonsLayout()->addWidget(viewSelectorWrapperButton);

    if (options)
    {
        if (auto modeIsXY = getFromJson<bool>(*options, modeIsXYKey))
        {
            viewSelectorWrapperButton->setChecked(*modeIsXY);
        }
    }
}

void LimitedValueVecWidget::setViewsStateFromValue(const LimitedIntOrDoubleVec& value)
{
    QSignalBlocker blockSliders(slidersView);
    slidersView->setValue(value);

    xyPadView->updateEverythingToMatchParentValue();
}

void LimitedValueVecWidget::setMode(Mode mode)
{
    if (mode == Mode::ShowJustLimitedValueWidgets)
    {
        slidersView->setVisible(true);
        xyPadView->setVisible(false);
    }
    else
    {
        slidersView->setVisible(false);
        xyPadView->setVisible(true);
    }
}
