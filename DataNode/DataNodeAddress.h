#pragma once
#include "DataNode.h"

//************************************************************
//	Assume there is a tree:
// 
//		comp("root", {
//						comp("next", {
//										leaf("hello", int(5))
//									 }
//					 });
//
//	And if you pass leaf node to getAbsAddress(), you will get:
//
//		"/next/hello"
//
//	Because we dont include the root name in address, ever.
// 
//  If you try to getAbsAddress() from root, you will get
//  empty NodeAddressString, which is a valid address with
//  meaning "take same node".
// 
//  I.e. if you try to get child at "/next/hello" from root
//  node you will get hello node.
//  I.e. if you try to get child at "" from root
//  node you will get root node.
// 
//  Root is the only node thats allowed to have empty name.
//************************************************************

using NodeAddressString = QString;
SV_DECL_ALIASES(NodeAddressString);

inline constexpr auto DataNodeAddressSeparator = "/";


inline NodeAddressStringOrError getAbsAddress(const DataNodeShared& node)
{
	if (!node)
	{
		return std::format("getAbsAddress error: null node ptr passed");
	}

	QString res;

	DataNodeShared curNode = node;

	while (true)
	{
		auto parentNode = curNode->tryGetParent();

		if (!parentNode)
		{
			//curNode is root. So we are not including it, we stop right there.
			break;
		}

		if (curNode->getName().isEmpty())
		{
			return std::format("getAbsAddress error: found non-root node {} with empty name, address up to that: [{}]", curNode, res);
		}

		res = DataNodeAddressSeparator + curNode->getName() + res;

		curNode = parentNode;
	}

	//may be empty and its fine
	return res;
}