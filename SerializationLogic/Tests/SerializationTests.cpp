#include "TestUtils.h"
#include "doctest/doctest.h"
#include "Registrations/Registrations.h"

TEST_CASE("Tree with all types roundtrip to json and back")
{
    REQUIRE(DatalayerDefaultTypesMetadata::everythingWasRegistered());

    auto tree = makeTreeWithAllDefaultTypes();
    REQUIRE(tree);

    auto json = tree->toJSON();
    REQUIRE(json);

    INFO(("Tree originally converted to this JSON: " + jsonValueToString(*json).toStdString()));
    
    auto reconstructedTree = DataNode::fromJSON(*json);
    REQUIRE(reconstructedTree);

    CHECK( treesAreCompletelyEqual(*tree, *reconstructedTree) );
}
