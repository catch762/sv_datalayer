#include "sv_common.h"
#include "doctest/doctest.h"
#include <cstring>
#include "TypeNames.h"

template<typename T>
bool nameIs(const char* name)
{
	return std::strcmp(typeName<T>(), name) == 0;
}

TEST_CASE("Checking that type names are registered")
{
	//no need to check actual names - if type is registered, name is correct,
	//if its not, we get compile error (thats real purpose of this test)

	CHECK(nameIs<bool>("bool"));
	CHECK(nameIs<int>("int"));
	CHECK(nameIs<double>("double"));
	CHECK(nameIs<QString>("QString"));
	CHECK(nameIs<LimitedInt>("LimitedInt"));
	CHECK(nameIs<LimitedIntVec>("LimitedIntVec"));
	CHECK(nameIs<LimitedDouble>("LimitedDouble"));
	CHECK(nameIs<LimitedDoubleVec>("LimitedDoubleVec"));
	CHECK(nameIs<Enum>("Enum"));
	CHECK(nameIs<EnumVec>("EnumVec"));
	CHECK(nameIs<BoolVec>("BoolVec"));
}
