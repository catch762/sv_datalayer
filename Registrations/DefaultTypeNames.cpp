#include "DefaultTypeNames.h"

void DefaultTypeNames::registerEverything()
{
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
