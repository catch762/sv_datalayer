#pragma once
#include "sv_qtcommon.h"
#include "SerializationLogic/SerializerInterface.h"
struct Enum
{
public:
    struct EnumEntry
    {
        int enumValue = 0;
        QString name;

        const bool operator==(const EnumEntry& other) const;
        static QJsonValue toJson(const EnumEntry& e);
        static std::optional<EnumEntry> fromJson(const QJsonValue& json);
        std::string toString() const;
    };
    
    Enum() = default;

    //this will not check if you passed entries with duplicating values.
    Enum(std::vector<EnumEntry> _entries, int _currentIndex = 0);

    const std::vector<EnumEntry>& getEntries() const;

    int entriesCount() const;

    //its not guaranteed to be valid index, check it yourself
    int getCurrentIndex() const;

    void setCurrentIndex(int newCurrentIndex);

    //returns nullopt if current index doesnt point to valid EnumEntry
    intOpt getEnumValue() const;

    //this only does something if there is such EnumEntry with this enumValue
    void setEnumValue(int newEnumValue);

    const EnumEntry* getEntryForIndex(int index) const;

    using EntryWithIndex = std::pair<const EnumEntry*, int>;
    std::optional<EntryWithIndex>  getEntryForEnumValue(int enumValue) const;

    const bool operator==(const Enum& other) const;

    static QJsonValue toJSON(const Enum &e);
    static std::optional<Enum> fromJSON(const QJsonValue &jsonValue);

    //index points to valid data and there are no duplicate entries. Expensive check.
    bool isValid() const;

    std::string toString() const;
private:
    std::vector<EnumEntry> entries;
    int currentIndex = 0;
};
SV_DECL_STD_FORMATTER(Enum, obj.toString());
SV_DECL_STD_FORMATTER(Enum::EnumEntry, obj.toString());
SV_DECL_ALIASES(Enum);

using EnumVec = std::vector<Enum>;
SV_DECL_ALIASES(EnumVec);

template <>
class Serializer<Enum>
{
public:
    QJsonValue toJson(const Enum& value)
    {
        return Enum::toJSON(value);
    }
    
    std::optional<Enum> fromJson(const QJsonValue& json)
    {
        return Enum::fromJSON(json);
    }
};