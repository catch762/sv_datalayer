#include "DefaultTypeNames.h"

template<typename T>
bool nameIs(const char* name)
{
	SV_ASSERT(name);
	if (auto* namePtr = typeName<T>())
	{
		return std::strcmp(namePtr, name) == 0;
	}
	else return false;
}

void DefaultTypeNames::registerEverything()
{
	SV_ASSERT(nameIs<bool>("bool"));
	SV_ASSERT(nameIs<int>("int"));
	SV_ASSERT(nameIs<double>("double"));
	SV_ASSERT(nameIs<QString>("QString"));
	SV_ASSERT(nameIs<LimitedInt>("LimitedInt"));
	SV_ASSERT(nameIs<LimitedIntVec>("LimitedIntVec"));
	SV_ASSERT(nameIs<LimitedDouble>("LimitedDouble"));
	SV_ASSERT(nameIs<LimitedDoubleVec>("LimitedDoubleVec"));
	SV_ASSERT(nameIs<Enum>("Enum"));
	SV_ASSERT(nameIs<EnumVec>("EnumVec"));
	SV_ASSERT(nameIs<BoolVec>("BoolVec"));

	/*
#define CHECKT(T) if(typeName<T>()){ SV_LOG(#T " ok"); }else{SV_ERROR(#T " no name!");}
	CHECKT(bool);
	CHECKT(BoolVec);
	CHECKT(int);
	CHECKT(double);
	CHECKT(QString);
	CHECKT(LimitedInt);
	CHECKT(LimitedIntVec);
	CHECKT(LimitedDouble);
	CHECKT(LimitedDoubleVec);
	CHECKT(Enum);
	CHECKT(EnumVec);
#undef CHECKT
	*/

	TypeNames::registerNameFunction<bool>();
	TypeNames::registerNameFunction<BoolVec>();
	TypeNames::registerNameFunction<int>();
	TypeNames::registerNameFunction<double>();
	TypeNames::registerNameFunction<QString>();
	TypeNames::registerNameFunction<LimitedInt>();
	TypeNames::registerNameFunction<LimitedIntVec>();
	TypeNames::registerNameFunction<LimitedDouble>();
	TypeNames::registerNameFunction<LimitedDoubleVec>();
	TypeNames::registerNameFunction<Enum>();
	TypeNames::registerNameFunction<EnumVec>();
}
