#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>

// This is used when i need to save some metadata for a type, AND access it by both type index and type name.

template<typename DataEntry>
class DataForTypeMap
{
public:
    //Its like std::map but with two keys
    using DataMap = boost::bimaps::bimap<
		boost::bimaps::tagged<QtTypeIndex, 	struct TypeIndexKey_Tag>,   // key 1: type index
		boost::bimaps::tagged<QString, 		struct TypeNameKey_Tag>,    // key 2: type name
		boost::bimaps::with_info<DataEntry>                             // value: DataEntry
	>;
    //DataMap as if it was simple map with just QtTypeIndex for key
    using DataMapAsTypeIndexMap = typename DataMap::template map_by<TypeIndexKey_Tag>::type;
    //DataMap as if it was simple map with just QString for key
    using DataMapAsTypeNameMap  = typename DataMap::template map_by<TypeNameKey_Tag>::type;


    // Will overwrite DataEntry if it exists for the type, but will LOG_WARN about it.
    // User is responsible for supplying correct pair of keys (which uniquely identify the same value)
    // If both keys CLEARLY dont identify same value, will do nothing and LOG_ERROR.
    void addEntryForType(QtTypeIndex typeIndex, const QString &typeName, DataEntry&& entry);

    const DataEntry* getEntry   (QtTypeIndex    typeIndex);
    const DataEntry* getEntry   (const QString& typeName);
    DataEntry*       getEntryPtr(QtTypeIndex    typeIndex);
    DataEntry*       getEntryPtr(const QString& typeName);

    bool entryExists(QtTypeIndex typeIndex);
    bool entryExists(const QString& typeName);
    bool entryExistsForEither(QtTypeIndex typeIndex, const QString& typeName);

	DataMapAsTypeIndexMap& getDataMapAsTypeindexMap();
    DataMapAsTypeNameMap&  getDataMapAsTypeNameMap();
    DataMap& getDataMap();

private:
    DataMap dataMap;
};

template <typename DataEntry>
inline void DataForTypeMap<DataEntry>::addEntryForType(QtTypeIndex typeIndex, const QString &typeName, DataEntry &&entry)
{
    auto typeIndexExists = entryExists(typeIndex);
    auto typeNameExists  = entryExists(typeName);

    if (typeIndexExists != typeNameExists)
    {
        SV_ERROR(std::format("DataForTypeMap: typeIndexExists is [{}] for [{}] but typeNameExists is [{}] for [{}], "
                             "so you are not even supplying properly unique key pair. Entry will not be added",
                             typeIndexExists, typeIndex, typeNameExists, typeName));
        return;
    }

    if (typeIndexExists || typeNameExists)
    {
        SV_WARN(std::format("DataForTypeMap: overwriting entry for type [{}, {}]", typeIndex, typeName));
    }

    dataMap.insert({typeIndex, typeName, std::move(entry)});
}

template <typename DataEntry>
inline const DataEntry *DataForTypeMap<DataEntry>::getEntry(QtTypeIndex typeIndex)
{
    auto& theMap = getDataMapAsTypeindexMap();
    auto found = theMap.find(typeIndex);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline const DataEntry *DataForTypeMap<DataEntry>::getEntry(const QString &typeName)
{
    auto& theMap = getDataMapAsTypeNameMap();
    auto found = theMap.find(typeName);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline DataEntry *DataForTypeMap<DataEntry>::getEntryPtr(QtTypeIndex typeIndex)
{
    auto& theMap = getDataMapAsTypeindexMap();
    auto found = theMap.find(typeIndex);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline DataEntry *DataForTypeMap<DataEntry>::getEntryPtr(const QString &typeName)
{
    auto& theMap = getDataMapAsTypeNameMap();
    auto found = theMap.find(typeName);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline bool DataForTypeMap<DataEntry>::entryExists(QtTypeIndex typeIndex)
{
    return getDataMapAsTypeindexMap().find(typeIndex) != getDataMapAsTypeindexMap().end();
}

template <typename DataEntry>
inline bool DataForTypeMap<DataEntry>::entryExists(const QString &typeName)
{
    return getDataMapAsTypeNameMap().find(typeName) != getDataMapAsTypeNameMap().end();
}

template <typename DataEntry>
inline bool DataForTypeMap<DataEntry>::entryExistsForEither(QtTypeIndex typeIndex, const QString &typeName)
{
    return entryExists(typeIndex) || entryExists(typeName);
}

template <typename DataEntry>
inline DataForTypeMap<DataEntry>::DataMapAsTypeIndexMap &DataForTypeMap<DataEntry>::getDataMapAsTypeindexMap()
{
    return dataMap.template by<TypeIndexKey_Tag>();
}

template <typename DataEntry>
inline DataForTypeMap<DataEntry>::DataMapAsTypeNameMap &DataForTypeMap<DataEntry>::getDataMapAsTypeNameMap()
{
    return dataMap.template by<TypeNameKey_Tag>();
}

template <typename DataEntry>
inline DataForTypeMap<DataEntry>::DataMap &DataForTypeMap<DataEntry>::getDataMap()
{
    return dataMap;
}
