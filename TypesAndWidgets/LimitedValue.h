#include "sv_common.h"


// Note: i deliberately did not sort 'min' and 'max', because
// if you mix them up, its not this class job to correct the mistake.

template<typename T>
class LimitedValue
{
public:
    LimitedParam(T theMin, T theMax, T theValue)
    : value(theValue), min(theMin), max(theMax)
    {
        setValue(theValue);
    }

    //sets 'value' and limits it to min and max.
    void setValue(T theValue)
    {
        value = std::min(std::max(min, theValue), max);
    }

    //sets and applies new limits to 'value'
    void setMin(T theMin)
    {
        min = theMin;
        value = setValue(value);
    }

    //sets and applies new limits to 'value'
    void setMax(T theMax)
    {
        max = theMax;
        value = setValue(value);
    }



    static QJsonValue toJSON(const LimitedParam &param)
    {
        QJsonObject obj;
        obj[MinKey] = min;
        obj[MaxKey] = max;
        obj[ValKey] = value;
        obj[TypeFieldKey] = thisTypeName();
        
    }

    static std::optional<LimitedParam> fromJSON(const QJsonValue &jsonValue)
    {
        
    }

    static QString thisTypeName()
    {
        return qtTypeName<LimitedValue<T>>();
    }

private:
    T min;
    T max;
    T value;

    static inline const QString MinKey = QString("min");
    static inline const QString MaxKey = QString("max");
    static inline const QString ValKey = QString("val");
};
