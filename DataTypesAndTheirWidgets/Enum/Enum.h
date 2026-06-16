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

    Enum(std::vector<EnumEntry> _entries, int _currentEnumValue)
    {
        entries = std::move(_entries);
        currentEnumValue = _currentEnumValue;
    }

    const std::vector<EnumEntry>& getEntries()
    {
        return entries;
    }

    int entriesCount()
    {
        return entries.size();
    }

    int getEnumValue()
    {
        return currentEnumValue;
    }

    void setEnumValue(int newValue)
    {
        currentEnumValue = newValue;
    }

    const EnumEntry* getEntryForIndex(int index)
    {
        if (!isValidIndex(index, entries.size())) return nullptr;
        return &entries[index];
    }

private:
    std::vector<EnumEntry> entries;
    int currentEnumValue = 0;
};