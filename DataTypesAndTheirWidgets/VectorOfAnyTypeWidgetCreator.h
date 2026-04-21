#pragma once
#include "sv_qtcommon.h"

#include "LimitedValue/LimitedValueWidget.h"


template<typename ElemType, typename WidgetType>
class VectorWidgetHelper
{
public:
    using VectorOfElements          = std::vector<ElemType>;

    using CreateWidgetFunc          = std::function<WidgetType*()>;
    using SetupWidgetFunc           = std::function<void(WidgetType*)>; //called after CreateWidgetFunc for signal connections
    using GetWidgetValueFunc        = std::function<const ElemType& (const WidgetType*)>;
    using SetWidgetValueFunc        = std::function<void            (WidgetType*, const ElemType&)>;

    VectorWidgetHelper() = default;

    VectorWidgetHelper( QWidget*            _actualWidget,
                        CreateWidgetFunc    _createWidgetFunc,
                        SetupWidgetFunc     _setupWidgetFunc,
                        GetWidgetValueFunc  _getValueFunc,
                        SetWidgetValueFunc  _setValueFunc)
    {
        actualWidget         = _actualWidget;
        createWidgetFunc     = _createWidgetFunc;
        setupWidgetFunc      = _setupWidgetFunc;
        getWidgetValueFunc   = _getValueFunc;
        setWidgetValueFunc   = _setValueFunc;

        elemWidgetsLayout = new QVBoxLayout(actualWidget);
        initLayoutSpacing(elemWidgetsLayout);

        SV_ASSERT(actualWidget);
        SV_ASSERT(createWidgetFunc);
        SV_ASSERT(setupWidgetFunc);
        SV_ASSERT(getWidgetValueFunc);
        SV_ASSERT(setWidgetValueFunc);
    }

    const VectorOfElements& getValue()
    {
        return value;
    }

    void setValue(const VectorOfElements& newValue)
    {
        value = newValue;
        setWidgetsStateFromValue(value);
    }

public:
    void setCurrentValueFromWidgetsState()
    {
        auto valueSize = value.size();
        auto widgetsSize = elemWidgets.size();

        if (valueSize != widgetsSize)
        {
            SV_LOG(std::format("VectorWidgetHelper, set val from widgets: valueSize[{}] but widgetsSize[{}], will resize",
                    valueSize, widgetsSize));
            value.resize(widgetsSize);
        }

        SV_ASSERT(value.size() == widgetsSize);
        for (int i = 0; i < widgetsSize; ++i)
        {
            value[i] = getWidgetValueFunc( elemWidgets[i] );
        }
    }

private:
    void setElemWidgetsCount(int requiredElemWidgetsCount)
    {
        auto existingElemWidgets = elemWidgets.size();

        if (existingElemWidgets > requiredElemWidgetsCount)
        {
            int widgetsToDelete = existingElemWidgets - requiredElemWidgetsCount;
            //SV_LOG(std::format("setElemWidgetsCount will delete [{}] widgets", widgetsToDelete));

            deleteElemWidgets(widgetsToDelete);
        }
        else if (requiredElemWidgetsCount > existingElemWidgets)
        {
            int widgetsToAdd = requiredElemWidgetsCount - existingElemWidgets;
            //SV_LOG(std::format("setElemWidgetsCount will add [{}] widgets", widgetsToAdd));

            addElemWidgets(widgetsToAdd);
        }
        else
        {
            //SV_LOG("setElemWidgetsCount will do nothing");
        }
    }

    void addElemWidgets(int widgetsToAdd)
    {
        for (int i = 0; i < widgetsToAdd; ++i)
        {
            WidgetType* widget = createWidgetFunc();
            setupWidgetFunc(widget);

            elemWidgets.push_back(widget);
            elemWidgetsLayout->addWidget(widget);
        }
    }

    void deleteElemWidgets(int widgetsToDelete)
    {
        for (int i = 0; i < widgetsToDelete; ++i)
        {
            delete elemWidgets.back();
            elemWidgets.pop_back();
        }
    }

    void setWidgetsStateFromValue(const VectorOfElements& valueVec)
    {
        setElemWidgetsCount(valueVec.size());
        SV_ASSERT(valueVec.size() == elemWidgets.size());

        for (int i = 0; i < elemWidgets.size(); ++i)
        {
            auto widget = elemWidgets[i];

            QSignalBlocker blocker(widget);

            setWidgetValueFunc(widget, value[i]);
        }
    }

private:
    CreateWidgetFunc         createWidgetFunc;
    SetupWidgetFunc          setupWidgetFunc;
    GetWidgetValueFunc       getWidgetValueFunc;
    SetWidgetValueFunc       setWidgetValueFunc;

    QWidget*                 actualWidget = nullptr;
    VectorOfElements         value;

    QVBoxLayout*             elemWidgetsLayout = nullptr;
    std::vector<WidgetType*> elemWidgets;
};

#define ELEM_TYPE LimitedInt
#define ELEM_WIDGET LimitedValueWidget
#define CREATE_WIDGET_FUNC []()->LimitedValueWidget*{ return new LimitedValueWidget(LimitedInt{}); }
#define GETVAL_WIDGET_FUNC [](const LimitedValueWidget* w)->const LimitedInt&{ return std::get<LimitedInt>(w->getValue()); }
#define SETVAL_WIDGET_FUNC [](LimitedValueWidget* w, const LimitedInt& v){ w->setValue(v); }
#define WIDGET_VALCHANGED_SIGNAL LimitedValueWidget::valueChanged

class VectorWidget : public QWidget
{
    Q_OBJECT
public:
    using VectorOfElements       = std::vector<ELEM_TYPE>;
    using VectorWidgetHelperType = VectorWidgetHelper<ELEM_TYPE, ELEM_WIDGET>;

    VectorWidget(QWidget *parent = nullptr)
    :   QWidget(parent),
        helper( this,
                CREATE_WIDGET_FUNC,
                std::bind(setupElementWidget, this, std::placeholders::_1),
                GETVAL_WIDGET_FUNC,
                SETVAL_WIDGET_FUNC)
    {
    }

    const VectorOfElements& getValue()
    {
        return helper.getValue();
    }

    void setValue(const VectorOfElements& newValue)
    {
        helper.setValue(newValue);
        emit valueChanged(getValue());
    }
    
signals:
    void valueChanged(const VectorOfElements& value);

private:
    void onElementWidgetValueChanged()
    {
        helper.setCurrentValueFromWidgetsState();
        emit valueChanged(getValue());
    }

    void setupElementWidget(ELEM_WIDGET* widget)
    {
        connect(widget, &WIDGET_VALCHANGED_SIGNAL, this, onElementWidgetValueChanged);
    }

private:
    VectorWidgetHelperType helper;
};

