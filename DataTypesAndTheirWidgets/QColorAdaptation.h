#pragma once
#include <QColor>

SV_REGTYPENAME(QColor);

template <>
class Serializer<QColor>
{
public:
    QJsonValue toJson(const QColor& color)
    {
        QJsonObject obj;
        obj[TypeFieldKey] = typeName<QColor>();
        obj["r"] = color.redF();
        obj["g"] = color.greenF();
        obj["b"] = color.blueF();
        obj["a"] = color.alphaF();

        return obj;
    }

    QColorOpt fromJson(const QJsonValue& json)
    {
        auto obj = convertJson<QJsonObject>(json);
        if (!obj) return std::nullopt;

        auto r = getFromJson<double>(*obj, "r");
        auto g = getFromJson<double>(*obj, "g");
        auto b = getFromJson<double>(*obj, "b");
        auto a = getFromJson<double>(*obj, "a");

        if (!r || !g || !b || !a) return std::nullopt;

        QColor res;
        res.setRedF(*r);
        res.setGreenF(*g);
        res.setBlueF(*b);
        res.setAlphaF(*a);

        return res;
    }
};

template<>
class Interpolator<QColor>
{
public:
    static void interpolate(const QColor&   A,
                            const QColor&   B,
                            QColor&         Result,
                            double          ratioAToB01)
    {
        Result = mixColors(A, B, ratioAToB01);
    }
};