#include "Enum.h"
#include "WidgetLogic/WidgetDefs.h"
#include "doctest.h"

namespace
{
const QString CurrentIndexKey = "index";
const QString EntriesKey = "entries";
}

const bool Enum::EnumEntry::operator==(const Enum::EnumEntry& other) const
{
    return enumValue == other.enumValue && name == other.name;
}

QJsonValue Enum::EnumEntry::toJson(const EnumEntry &e)
{
    QJsonArray arr;
    arr.append(e.enumValue);
    arr.append(e.name);
    return arr;
}

std::optional<Enum::EnumEntry> Enum::EnumEntry::fromJson(const QJsonValue &json)
{
    auto jsonArr = convertJsonAndLogError<QJsonArray>(json, "EnumEntry");
    if (!jsonArr) return {};

    if (jsonArr->size() != 2)
    {
        SV_ERROR(std::format("Bad EnumEntry arr size: {}", jsonArr->size()));
        return {};
    }

    auto enumVal = convertJsonAndLogError<int>(jsonArr->at(0), "EnumEntry");
    if (!enumVal) return {};

    auto name = convertJsonAndLogError<QString>(jsonArr->at(1), "EnumEntry");
    if (!name) return {};

    return EnumEntry{*enumVal, *name};
}

Enum::Enum(std::vector<EnumEntry> _entries, int _currentIndex)
{
    entries = std::move(_entries);
    currentIndex = _currentIndex;
}

const std::vector<Enum::EnumEntry>& Enum::getEntries() const
{
    return entries;
}

int Enum::entriesCount() const
{
    return entries.size();
}

//its not guaranteed to be valid
int Enum::getCurrentIndex() const
{
    return currentIndex;
}

void Enum::setCurrentIndex(int newCurrentIndex)
{
    currentIndex = newCurrentIndex;
}

intOpt Enum::getEnumValue() const
{
    if (auto *entry = getEntryForIndex(currentIndex))
    {
        return entry->enumValue;
    }
    else return {};
}

void Enum::setEnumValue(int newEnumValue)
{
    if (auto entryWithIndex = getEntryForEnumValue(newEnumValue))
    {
        currentIndex = entryWithIndex->second;
    }
    else SV_ERROR("Unable to set enumValue " + std::to_string(newEnumValue));
}

const Enum::EnumEntry* Enum::getEntryForIndex(int index) const
{
    if (!isValidIndex(index, entries.size())) return nullptr;
    return &entries[index];
}

std::optional<Enum::EntryWithIndex>  Enum::getEntryForEnumValue(int enumValue) const
{
    for (int i = 0; i < entries.size(); ++i)
    {
        if (entries[i].enumValue == enumValue) return EntryWithIndex{&entries[i], i};
    }
    return {};
}

const bool Enum::operator==(const Enum& other) const
{
    return currentIndex == other.currentIndex && vectorContentEquals(entries, other.entries);
}

QJsonValue Enum::toJSON(const Enum &e)
{
    QJsonObject obj;
    obj[CurrentIndexKey]      = e.currentIndex;
    
    QJsonArray entriesArray;
    for(const auto &entry : e.entries)
    {
        entriesArray.append(EnumEntry::toJson(entry));
    }
    obj[EntriesKey] = entriesArray;
    obj[TypeFieldKey] = qtTypeName<Enum>();

    return obj;
}

std::optional<Enum> Enum::fromJSON(const QJsonValue &jsonValue)
{
    auto jsonObj = convertJsonAndLogError<QJsonObject>(jsonValue, "Enum");
    if (!jsonObj) return {};

    auto index = getFromJsonAndLogError<int>(*jsonObj, CurrentIndexKey, "Enum");
    if (!index) return {};

    auto entriesArr = getFromJsonAndLogError<QJsonArray>(*jsonObj, EntriesKey, "Enum");
    if (!entriesArr) return {};

    std::vector<EnumEntry> decodedEntries;
    for (auto jsonVal : *entriesArr)
    {
        auto entryOpt = EnumEntry::fromJson(jsonVal);
        if (!entryOpt) return {};
        else decodedEntries.push_back(std::move(*entryOpt));
    }

    return Enum(std::move(decodedEntries), *index);
}

TEST_CASE("Enum to and from json round trip")
{
    Enum base = {
        {{10, "Aaa"}, {20, "Bbb"}, {30, ""}},
        1
    };

    SUBCASE("First check that operator== works")
    {
        Enum same = {
            {{10, "Aaa"}, {20, "Bbb"}, {30, ""}},
            1
        };

        Enum diff = {
            {{10, "Aaa"}, {20, "Bbb"}, {30, "diff"}},
            1
        };

        CHECK(base == same);
        CHECK(base != diff);
    }

    SUBCASE("Serializing and deserializing and check that nothing changed")
    {
        auto json = Enum::toJSON(base);

        auto reconstructedBase = Enum::fromJSON(json);

        REQUIRE(reconstructedBase);

        CHECK(base == *reconstructedBase);
    }
}