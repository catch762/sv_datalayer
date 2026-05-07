#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>

template<typename DataEntry>
class TypeDataCollection
{
public:
    //Its like std::map but with two keys
    using DataForTypesMap = boost::bimaps::bimap<
		boost::bimaps::tagged<QtTypeIndex, 	struct TypeIndexKey_Tag>,   // key 1: type index
		boost::bimaps::tagged<QString, 		struct TypeNameKey_Tag>,    // key 2: type name
		boost::bimaps::with_info<DataEntry>                             // value: DataEntry
	>;
    //DataForTypesMap as if it was simple map with just QtTypeIndex for key
    using DataForTypesAsTypeIndexMap = typename DataForTypesMap::template map_by<TypeIndexKey_Tag>::type;
    //DataForTypesMap as if it was simple map with just QString for key
    using DataForTypesAsTypeNameMap  = typename DataForTypesMap::template map_by<TypeNameKey_Tag>::type;


    //Will overwrite DataEntry if it exists for the type, but will LOG_WARN about it.
    void addEntryForType(QtTypeIndex    typeIndex, const QString &typeName, DataEntry&& entry);

    const DataEntry* getEntry   (QtTypeIndex    typeIndex);
    const DataEntry* getEntry   (const QString& typeName);
    DataEntry*       getEntryPtr(QtTypeIndex    typeIndex);
    DataEntry*       getEntryPtr(const QString& typeName);

	DataForTypesAsTypeIndexMap& getDataMapAsTypeindexMap();
    DataForTypesAsTypeNameMap&  getDataMapAsTypeNameMap();
    DataForTypesMap& getDataMap();

private:
    DataForTypesMap dataMap;
};

template <typename DataEntry>
inline void TypeDataCollection<DataEntry>::addEntryForType(QtTypeIndex typeIndex, const QString &typeName, DataEntry &&entry)
{
    auto typeIndexExists = getDataMapAsTypeindexMap().find(typeIndex) != getDataMapAsTypeindexMap().end();
    auto typeNameExists  = getDataMapAsTypeNameMap().find(typeName) != getDataMapAsTypeNameMap().end();

    if (typeIndexExists != typeNameExists)
    {
        SV_ERROR(std::format("TypeDataCollection: typeIndexExists is [{}] for [{}] but typeNameExists is [{}] for [{}], "
                             "so you are not even supplying properly unique key pair. Entry will not be added",
                             typeIndexExists, typeIndex, typeNameExists, typeName));
        return;
    }

    if (typeIndexExists || typeNameExists)
    {
        SV_WARN(std::format("TypeDataCollection: overwriting entry for type [{}, {}]", typeIndex, typeName));
    }

    theMap.insert({typeIndex, typeName, std::move(entry)});
}

template <typename DataEntry>
inline const DataEntry *TypeDataCollection<DataEntry>::getEntry(QtTypeIndex typeIndex)
{
    auto& theMap = getDataMapAsTypeindexMap();
    auto found = theMap.find(typeIndex);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline const DataEntry *TypeDataCollection<DataEntry>::getEntry(const QString &typeName)
{
    auto& theMap = getDataMapAsTypeNameMap();
    auto found = theMap.find(typeName);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline DataEntry *TypeDataCollection<DataEntry>::getEntryPtr(QtTypeIndex typeIndex)
{
    auto& theMap = getDataMapAsTypeindexMap();
    auto found = theMap.find(typeIndex);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline DataEntry *TypeDataCollection<DataEntry>::getEntryPtr(const QString &typeName)
{
    auto& theMap = getDataMapAsTypeNameMap();
    auto found = theMap.find(typeName);
    if (found != theMap.end()) return &found->info;
    else return nullptr;
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesAsTypeIndexMap &TypeDataCollection<DataEntry>::getDataMapAsTypeindexMap()
{
    return dataMap.by<TypeIndexKey_Tag>();
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesAsTypeNameMap &TypeDataCollection<DataEntry>::getDataMapAsTypeNameMap()
{
    return dataMap.by<TypeNameKey_Tag>();
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesMap &TypeDataCollection<DataEntry>::getDataMap()
{
    return dataMap;
}
