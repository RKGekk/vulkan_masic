#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "render_resource.h"
#include "render_node.h"

class VulkanDevice;

class RenderGraph {
public:
	bool init(std::shared_ptr<VulkanDevice> device);
    void destroy();

	void add_pass(std::shared_ptr<RenderNode> render_node);
	void topological_sort();


private:
	using RenderNodePtr = std::shared_ptr<RenderNode>;
	std::shared_ptr<VulkanDevice> m_device;
	std::vector<RenderNodePtr> m_render_nodes;
	std::unordered_map<RenderNodePtr, std::unordered_set<RenderNodePtr>> m_adjency_list;
	std::vector<RenderNodePtr> m_topologically_sorted_nodes;
};