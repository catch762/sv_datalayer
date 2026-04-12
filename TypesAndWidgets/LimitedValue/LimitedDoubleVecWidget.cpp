#include "LimitedDoubleVecWidget.h"
#include "Internal/LimitedDoublesWidget.h"
#include "Internal/XYPadWithPresetsWidget.h"
#include "TypesAndWidgets/DataNodeWrapperWidget.h"
#include <QToolButton>


LimitedDoubleVecWidget::LimitedDoubleVecWidget(const LimitedDoubleVec& initialValue,
    const QJsonObjectWithWidgetOptionsOpt& options, QWidget *parent)
    : QFrame(parent)
{
    value = initialValue;

    layout = new QVBoxLayout(this);
    initLayoutSpacing(layout, 2, 0);

    slidersView = new LimitedDoublesWidget(value, this);
    {
        connect(slidersView, &LimitedDoublesWidget::valueChanged, this, [this](auto &val)
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

const LimitedDoubleVec& LimitedDoubleVecWidget::getValue() const
{
    return value;
}

void LimitedDoubleVecWidget::setValue(const LimitedDoubleVec& newValue)
{
    //static int i = 0;
    //SV_LOG("Master: setValue " + std::to_string(i++));

    value = newValue;

    setViewsStateFromValue(value);

    emit valueChanged(value);
}

QJsonObjectWithWidgetOptionsOpt LimitedDoubleVecWidget::makeOptions() const
{
    QJsonObjectWithWidgetOptions options = xyPadView->makeOptions().value_or(QJsonObjectWithWidgetOptions());

    if (viewSelectorWrapperButton)
    {
        options[modeIsXYKey] = viewSelectorWrapperButton->isChecked();
    }

    return options;
}

void LimitedDoubleVecWidget::setupButtonsOnWrapperParent(DataNodeWrapperWidget *wrapper, const QJsonObjectWithWidgetOptionsOpt& options)
{
    viewSelectorWrapperButton = makeTopStripeCheckableButtonWithIcon(QIcon::ThemeIcon::MediaPlaybackStop,
                                                                     QIcon::ThemeIcon::FormatJustifyLeft);
    viewSelectorWrapperButton->setChecked(false);

    connect(viewSelectorWrapperButton, &QPushButton::toggled, this, [this](bool checked)
    {
        setMode(checked ? Mode::ShowXYPad : Mode::ShowJustLimitedDoubleWidgets);
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

void LimitedDoubleVecWidget::setViewsStateFromValue(const LimitedDoubleVec& value)
{
    QSignalBlocker blockSliders(slidersView);
    slidersView->setValue(value);

    xyPadView->updateEverythingToMatchParentValue();
}

void LimitedDoubleVecWidget::setMode(Mode mode)
{
    if (mode == Mode::ShowJustLimitedDoubleWidgets)
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
