#pragma once
#include "sv_qtcommon.h"
#include <boost/bimap.hpp>

template<typename DataEntry>
class TypeDataCollection
{
public:
    //its like std::map but with two keys
    using DataForTypesMap = boost::bimaps::bimap<
		boost::bimaps::tagged<QtTypeIndex, 	struct TypeIndexKey_Tag>,   // key 1: type index
		boost::bimaps::tagged<QString, 		struct TypeNameKey_Tag>,    // key 2: type name
		boost::bimaps::with_info<DataEntry>                             // value: DataEntry
	>;
    //DataForTypesMap as if it was simple map with just QtTypeIndex for key
    using DataForTypesAsTypeIndexMap = typename DataForTypesMap::template map_by<TypeIndexKey_Tag>::type;
    //DataForTypesMap as if it was simple map with just QString for key
    using DataForTypesAsTypeNameMap  = typename DataForTypesMap::template map_by<TypeNameKey_Tag>::type;


    //These will overwrite DataEntry if it exists for the type, but will LOG_WARN about it.
    void addEntryForType(QtTypeIndex    typeIndex, DataEntry&& entry);
    void addEntryForType(const QString& typeName,  DataEntry&& entry);

    const DataEntry* getEntry();
    DataEntry*       getEntryPtr();

	DataForTypesAsTypeIndexMap& getDataMapAsTypeindexMap();
    DataForTypesAsTypeNameMap&  getDataMapAsTypeNameMap();
    DataForTypesMap& getDataMap();

private:
    DataForTypesMap dataMap;
};

template <typename DataEntry>
inline void TypeDataCollection<DataEntry>::addEntryForType(QtTypeIndex typeIndex, DataEntry &&entry)
{
}

template <typename DataEntry>
inline void TypeDataCollection<DataEntry>::addEntryForType(const QString &typeName, DataEntry &&entry)
{
}

template <typename DataEntry>
inline const DataEntry *TypeDataCollection<DataEntry>::getEntry()
{
    return nullptr;
}

template <typename DataEntry>
inline DataEntry *TypeDataCollection<DataEntry>::getEntryPtr()
{
    return nullptr;
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesAsTypeIndexMap &TypeDataCollection<DataEntry>::getDataMapAsTypeindexMap()
{
    // TODO: insert return statement here
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesAsTypeNameMap &TypeDataCollection<DataEntry>::getDataMapAsTypeNameMap()
{
    // TODO: insert return statement here
}

template <typename DataEntry>
inline TypeDataCollection<DataEntry>::DataForTypesMap &TypeDataCollection<DataEntry>::getDataMap()
{
    return dataMap;
}
