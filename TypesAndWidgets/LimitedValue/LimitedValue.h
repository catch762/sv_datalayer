#pragma once
#include "sv_qtcommon.h"
#include <algorithm>
#include "DataLayerUtils.h"

//todo rewrite for if float, if integer, not type names

//******************************************************************************
//
// Contains 'value' and two limits: 'left' and 'right'.
// 'left' can be smaller or bigger than 'right'.
// No matter what, 'value' will be between min(left, right) and max(left, right)
//
// Storing 'min' and 'max' directly would conflict with idea of corresponding
// UI element, which might be setting bigger value in 'min' than in 'max'
//
// The <T> is only expected to be integer or floating point.
//
//******************************************************************************

template<typename T>
concept LimitedValueAllowedType = std::is_arithmetic_v<T>; //only integer or floating point

template<LimitedValueAllowedType T>
class LimitedValue
{
public:
    LimitedValue(T theValue = 0, T theLeft = 0, T theRight = 1)
    : _value(theValue), _left(theLeft), _right(theRight)
    {
        setValue(theValue);
    }

    T value() const
    {
        return _value;
    }
    //sets 'value' and applies limits to it
    void setValue(T theValue)
    {
        _value = std::clamp(theValue, min(), max());
    }

    T left() const
    {
        return _left;
    }
    //sets and applies new limits to 'value'
    void setLeft(T theLeft)
    {
        left = theLeft;
        setValue(_value);
    }

    T right() const
    {
        return _right;
    }
    //sets and applies new limits to 'value'
    void setRight(T theRight)
    {
        _right = theRight;
        setValue(_value);
    }

    T min() const
    {
        return std::min(_left, _right);
    }

    T max() const
    {
        return std::max(_left, _right);
    }

    //at 0.0 sets value to left, at 1.0 to right.
    void setValue01(double toRight01)
    {
        toRight01 = std::clamp(toRight01, 0.0, 1.0);
        setValue( mix(_left, _right, toRight01) );
    }

    //if value==left, returns 0.0, if value==right returns 1.0
    double getValue01() const
    {
        T diffFromLeft = _value - _left;
        T span = _right - _left;

        //avoiding dividing by both 'int, zero' and 'double, practically zero'
        if (double(abs(span)) < std::numeric_limits<double>::epsilon()) return 0.0;

        return diffFromLeft/span;
    }

    static QJsonValue toJSON(const LimitedValue &param)
    {
        QJsonObject obj;
        obj[LeftKey]      = param.left();
        obj[RightKey]     = param.right();
        obj[ValKey]       = param.value();
        obj[TypeFieldKey] = thisTypeName();
        return obj;
    }

    static std::optional<LimitedValue> fromJSON(const QJsonValue &jsonValue)
    {
        const QString err = "LimitedParam::fromJSON failed";

        auto jsonObjOpt = convertJsonAndLogError<QJsonObject>(jsonValue, err);
        if (!jsonObjOpt) return {};

        auto valueOpt   = getFromJsonAndLogError<double> (*jsonObjOpt, ValKey,   err);
        auto leftOpt    = getFromJsonAndLogError<double> (*jsonObjOpt, LeftKey,  err);
        auto rightOpt   = getFromJsonAndLogError<double> (*jsonObjOpt, RightKey, err);
        auto typeOpt    = getFromJsonAndLogError<QString>(*jsonObjOpt, TypeFieldKey, err);

        if (typeOpt && *typeOpt != thisTypeName())
        {
            SV_ERROR(QString("'%1' tried to deserialize from json with type '%2'")
                .arg(thisTypeName()).arg(*typeOpt).toStdString());
            return {};
        }

        if (leftOpt && rightOpt && valueOpt && typeOpt)
        {
            return LimitedValue(*valueOpt, *leftOpt, *rightOpt);
        }

        return {};
    }

    static QString thisTypeName()
    {
        return qtTypeName<LimitedValue<T>>();
    }

    QString toString() const
    {
        return QString("%1[left=%2][right=%3][val=%4]").arg(thisTypeName()).arg(_left).arg(_right).arg(_value);
    }

private:


private:
    T _value;
    T _left;
    T _right;

    static inline const QString LeftKey = QString("left");
    static inline const QString RightKey = QString("right");
    static inline const QString ValKey = QString("val");
};

using LimitedInt    = LimitedValue<int>;
using LimitedDouble = LimitedValue<double>;

Q_DECLARE_METATYPE(LimitedInt)
Q_DECLARE_METATYPE(LimitedDouble)

SV_DECL_ALIASES(LimitedInt)
SV_DECL_ALIASES(LimitedDouble)