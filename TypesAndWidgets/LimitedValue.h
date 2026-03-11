#pragma once
#include "sv_qtcommon.h"
#include <algorithm>

//******************************************************************************
//
// Contains 'value' and two limits: 'left' and 'right'.
// 'left' can be smaller or bigger than 'right'.
// No matter what, 'value' will be between min(left, right) and max(left, right)
//
// Storing 'min' and 'max' directly would conflict with idea of corresponding
// UI element, which might be setting bigger value in 'min' than in 'max'
//
// The <T> is only expected to be int or double.
//
//******************************************************************************

template<typename T>
concept LimitedValueAllowedType = std::same_as<T, int> || std::same_as<T, double>;

template<LimitedValueAllowedType T>
class LimitedValue
{
public:
    LimitedValue(T theValue = 0, T theLeft = 0, T theRight = 1)
    : value(theValue), left(theLeft), right(theRight)
    {
        setValue(theValue);
    }

    //sets 'value' and applies limits to it
    void setValue(T theValue)
    {
        value = std::clamp(value, min(), max());
    }

    //sets and applies new limits to 'value'
    void setLeft(T theLeft)
    {
        left = theLeft;
        value = setValue(value);
    }

    //sets and applies new limits to 'value'
    void setRight(T theRight)
    {
        right = theRight;
        value = setValue(value);
    }

    T min() const
    {
        return std::min(left, right);
    }

    T max() const
    {
        return std::max(left, right);
    }

    //at 0.0 sets value to left, at 1.0 to right.
    void setValue01(double toRight01)
    {
        toRight01 = std::clamp(toRight01, 0.0, 1.0);
        setValue( mix(left, right, toRight01) );
    }

    //if value==left, returns 0.0, if value==right returns 1.0
    double getValue01()
    {
        T diffFromLeft = value - left;
        T span = right - left;

        //avoiding dividing by both 'int, zero' and 'double, practically zero'
        if (double(abs(span)) < std::numeric_limits<double>::epsilon()) return 0.0;

        return diffFromLeft/span;
    }

    static QJsonValue toJSON(const LimitedValue &param)
    {
        QJsonObject obj;
        obj[LeftKey]      = param.left;
        obj[RightKey]     = param.right;
        obj[ValKey]       = param.value;
        obj[TypeFieldKey] = thisTypeName();
        return obj;
    }

    static std::optional<LimitedValue> fromJSON(const QJsonValue &jsonValue)
    {
        const QString err = "LimitedParam::fromJSON failed";

        auto jsonObjOpt = jsonGetObjectOptAndLogError(jsonValue, err);
        if (!jsonObjOpt) return {};

        auto valueOpt   = jsonGetDoubleOptAndLogError (*jsonObjOpt, ValKey,   err);
        auto leftOpt    = jsonGetDoubleOptAndLogError (*jsonObjOpt, LeftKey,  err);
        auto rightOpt   = jsonGetDoubleOptAndLogError (*jsonObjOpt, RightKey, err);
        auto typeOpt    = jsonGetStringOptAndLogError (*jsonObjOpt, TypeFieldKey, err);

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

    QString toString()
    {
        return QString("%1[left=%2][right=%3][val=%4]").arg(thisTypeName()).arg(left).arg(right).arg(value);
    }

private:


private:
    T value;
    T left;
    T right;

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