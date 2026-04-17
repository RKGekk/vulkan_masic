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
#include "dependency_level.h"

class VulkanDevice;
class RenderNode;
class GraphicsRenderNodeConfig;
class WindowSurface;

class RenderGraph {
public:
	using RenderNodePtr = std::shared_ptr<RenderNode>;
	using RenderNodeList = std::vector<RenderNodePtr>;
	using RenderNodeSet = std::unordered_set<RenderNodePtr>;
	static const size_t NO_ID = -1;

	bool init(std::shared_ptr<VulkanDevice> device, const std::shared_ptr<WindowSurface>& window);
    void destroy();

	const std::shared_ptr<GraphicsRenderNodeConfig>& getGraphicsRenderNodeConfig(const std::string& config_name) const;
	std::shared_ptr<GraphicsRenderNodeConfig> makeGraphicsRenderNodeCfgInstance(const std::string& config_name, const std::string& config_extension);

	void add_pass(std::shared_ptr<RenderNode> render_node);
	void topological_sort();
	void build_dependency_levels();

	const RenderNodeList& getTopologicallySortedNodes();
	const RenderNodePtr& getRenderNodeByID(size_t id) const;
	const std::vector<std::shared_ptr<DependencyLevel>>& getDependencyLevels();

	RenderNodePtr getLastWritten(const RenderNodePtr& render_node, const std::string& global_resource_name) const;
	size_t getLastWrittenIdentity(const RenderNodePtr& render_node, const std::string& global_resource_name) const;
	bool isWrittenInGraph(const std::string& gloabal_resuorce_name) const;
	const RenderNodeSet& getWrittenBy(const std::string& global_resource_name) const;

	RenderNodePtr getLastRead(const RenderNodePtr& render_node, const std::string& global_resource_name) const;
	size_t getLastReadIdentity(const RenderNodePtr& render_node, const std::string& global_resource_name) const;
	size_t getTopologicalIdentity(const RenderNodePtr& render_node) const;
	bool isReadInGraph(const std::string& global_resource_name) const;
	const RenderNodeSet& getReadBy(const std::string& global_resource_name) const;

private:

	std::shared_ptr<VulkanDevice> m_device;

	std::unordered_map<std::string, std::shared_ptr<GraphicsRenderNodeConfig>> m_graphics_cfg_name_map;

	RenderNodeList m_render_nodes;
	std::unordered_map<std::string, RenderNodeSet> m_read_map;
	std::unordered_map<std::string, RenderNodeSet> m_written_map;

	std::unordered_map<RenderNodePtr, RenderNodeSet> m_adjency_list; // Write To Read
	std::unordered_map<RenderNodePtr, RenderNodeSet> m_rev_adjency_list; // Read To Write
	RenderNodeList m_topologically_sorted_nodes;
	std::unordered_map<RenderNodePtr, size_t> m_render_node_sort_idx;
	std::vector<std::shared_ptr<DependencyLevel>> m_dependency_levels;
	bool m_sorted;
};