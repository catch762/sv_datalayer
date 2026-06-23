#include "sv_common.h"
#include "doctest/doctest.h"

TEST_CASE("Checking that type names are registered")
{
	//no need to check actual names - if type is registered, name is correct,
	//if its not, we get compile error (thats real purpose of this test)
	CHECK(typeName<bool>() == std::string("bool"));
}
