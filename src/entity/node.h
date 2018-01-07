#pragma once

#include <vector>
#include <memory>

namespace tke
{
	enum NodeType
	{
		NodeTypeScene,
		NodeTypeLight,
		NodeTypeObject,
		NodeTypeTerrain,
		NodeTypeWater
	};

	struct Node
	{
		NodeType type;
		Node *parent = nullptr;
		std::vector<std::shared_ptr<Node>> children;

		Node(NodeType _type);
	};
}
