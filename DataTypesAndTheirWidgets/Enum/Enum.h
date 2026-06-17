#pragma once
#include "sv_qtcommon.h"

struct Enum
{
public:
    struct EnumEntry
    {
        int enumValue = 0;
        QString name;
    };
    
    Enum() = default;

    Enum(std::vector<EnumEntry> _entries, int _currentIndex = 0)
    {
        entries = std::move(_entries);
        currentIndex = _currentIndex;
    }

    const std::vector<EnumEntry>& getEntries()
    {
        return entries;
    }

    int entriesCount()
    {
        return entries.size();
    }

    //its not guaranteed to be valid
    int getCurrentIndex()
    {
        return currentIndex;
    }

    void setCurrentIndex(int newCurrentIndex)
    {
        currentIndex = newCurrentIndex;
    }

    intOpt getEnumValue()
    {
        if (auto *entry = getEntryForIndex(currentIndex))
        {
            return entry->enumValue;
        }
        else return {};
    }

    void setEnumValue(int newEnumValue)
    {
        if (auto entryWithIndex = getEntryForEnumValue(newEnumValue))
        {
            currentIndex = entryWithIndex->second;
        }
        else SV_ERROR("Unable to set enumValue " + std::to_string(newEnumValue));
    }

    const EnumEntry* getEntryForIndex(int index)
    {
        if (!isValidIndex(index, entries.size())) return nullptr;
        return &entries[index];
    }

    using EntryWithIndex = std::pair<const EnumEntry*, int>;
    std::optional<EntryWithIndex>  getEntryForEnumValue(int enumValue)
    {
        for (int i = 0; i < entries.size(); ++i)
        {
            if (entries[i].enumValue == enumValue) return EntryWithIndex{&entries[i], i};
        }
        return {};
    }

private:
    std::vector<EnumEntry> entries;
    int currentIndex = 0;
};