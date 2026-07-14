#pragma once
#include "DataNode.h"

//there will be huge rewrite later, when requirements arrive

using NodeAddressString = QString;
SV_DECL_ALIASES(NodeAddressString);

inline constexpr auto DataNodeAddressSeparator = "/";

inline NodeAddressStringOrError getAddress(const DataNodeShared& node)
{
	if (!node)
	{
		return std::format("getAddress error: null node ptr passed");
	}

	QString res;

	DataNodeShared curNode = node;

	while (true)
	{
		if (curNode->getName().isEmpty())
		{
			return std::format("getAddress error: found node {} with empty name, address up to that: [{}]", curNode, res);
		}

		res = curNode->getName() + res;

		if (auto parentNode = curNode->tryGetParent())
		{
			res = DataNodeAddressSeparator + res;
			curNode = parentNode;
		}
		else
		{
			break;
		}
	}

	return res;
}