#pragma once
#include "LimitedDoubleWidget.h"
#include "TypesAndWidgets/TypesAndWidgets.h"

//as of now, no change of vector size is possible, once widget is created

class LimitedDoubleVecWidget : public QFrame
{
    Q_OBJECT
public:
    LimitedDoubleVecWidget(const LimitedDoubleVec& vec, QWidget *parent = nullptr)
    : QFrame(parent)
    {
        layout = new QVBoxLayout(this);

        setValue(vec);
    }

    const LimitedDoubleVec& getValue()
    {
        return value;
    }

    //may add or remove widgets based on difference between current value and new value
    void setValue(const LimitedDoubleVec& newValue)
    {
        value = newValue;

        setWidgetsStateFromValue(value);

        emit valueChanged(value);
    }

signals:
    void valueChanged(const LimitedDoubleVec &val);

private slots:
    void onSomethingChanged()
    {
        setCurrentValueFromWidgetsState();

        emit valueChanged(value);
    }

private:
    //May resize value, if widgets count is different from value size
    void setCurrentValueFromWidgetsState()
    {
        auto valueSize = value.size();
        auto widgetsSize = subWidgets.size();

        if (valueSize != widgetsSize)
        {
            SV_LOG(std::format("LimitedDoubleVecWidget, set val from widgets: valueSize[{}] but widgetsSize[{}], will resize",
                 valueSize, widgetsSize));
            value.resize(widgetsSize);
        }

        SV_ASSERT(value.size() == subWidgets.size());
        for (int i = 0; i < widgetsSize; ++i)
        {
            value[i] = subWidgets[i]->currentValue();
        }
    }

    //Simply makes sure there are N subwidgets now: deletes unneeded widgets or adds new ones, if needed.
    //Added subwidgets remain with default value.
    void setSubWidgetsCount(int requiredSubwidgetsCount)
    {
        auto existingSubwidgets = subWidgets.size();

        if (existingSubwidgets > requiredSubwidgetsCount)
        {
            int widgetsToDelete = existingSubwidgets - requiredSubwidgetsCount;

            SV_LOG(std::format("setSubWidgetsCount will delete [{}] widgets", widgetsToDelete));

            for (int i = 0; i < widgetsToDelete; ++i)
            {
                delete subWidgets.back();
                subWidgets.pop_back();
            }
        }
        else if (requiredSubwidgetsCount > existingSubwidgets)
        {
            int widgetsToAdd = requiredSubwidgetsCount - existingSubwidgets;

            SV_LOG(std::format("setSubWidgetsCount will add [{}] widgets", widgetsToAdd));

            for (int i = 0; i < widgetsToAdd; ++i)
            {
                //not even setting value, well do it later
                auto widget = new LimitedDoubleWidget(LimitedDouble{}, this);

                connect(widget, &LimitedDoubleWidget::valueChanged, this, &LimitedDoubleVecWidget::onSomethingChanged);

                subWidgets.push_back(widget);
                layout->addWidget(widget);
            }
        }
        else
        {
            SV_LOG("setSubWidgetsCount will do nothing");
        }
    }

    void setWidgetsStateFromValue(const LimitedDoubleVec& value)
    {
        setSubWidgetsCount(value.size());

        SV_ASSERT(value.size() == subWidgets.size());

        for (int i = 0; i < subWidgets.size(); ++i)
        {
            auto widget = subWidgets[i];

            QSignalBlocker blocker(widget);
            widget->setValue(value[i]);
        }
    }

private:
    LimitedDoubleVec value;
    QVBoxLayout* layout = nullptr;
    std::vector<LimitedDoubleWidget*> subWidgets;
};