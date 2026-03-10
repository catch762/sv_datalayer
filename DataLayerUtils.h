#pragma once

static constexpr auto TypeFieldKey = "_type";

/*
void sereializeAndDeserialize(QVariant val, QString testName)
{
    SV_LOG("DataLayerTest", "****** " + testName.toStdString() + "*****");
    SV_LOG("DataLayerTest", "Input: " + qVariantInfo(val).toStdString());

    auto jsonVal = SerializationSystem::instance().qVariantToJson(val);

    SV_LOG("DataLayerTest", "Serialized as " + jsonValueToString(jsonVal).toStdString());

    auto reconstructedVariant = SerializationSystem::instance().jsonToQVariant(jsonVal);

    SV_LOG("DataLayerTest", "Output: " + qVariantInfo(reconstructedVariant).toStdString() + "\n");
};

void dl_testing()
{
    sereializeAndDeserialize(QVariant(true),            "bool");
    sereializeAndDeserialize(QVariant(QString("kek")),  "string");
    sereializeAndDeserialize(QVariant(5.0),             "double");
}
*/