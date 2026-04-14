#include "render_graph.h"

#include "../api/vulkan_device.h"
#include "../api/vulkan_pipeline.h"
#include "../api/vulkan_render_pass.h"
#include "render_node.h"
#include "graphics_render_node_config.h"
#include "graphics_render_node.h"
#include "../../application.h"

#include <pugixml.hpp>

#include <filesystem>

bool RenderGraph::init(std::shared_ptr<VulkanDevice> device) {
    using namespace std::literals;

    m_device = std::move(device);
    m_sorted = false;

    pugi::xml_document xml_doc;
	pugi::xml_parse_result parse_res = xml_doc.load_file("graphics_pipelines.xml");
	if (!parse_res) { return false;	}

	pugi::xml_node root_node = xml_doc.root();
	if (!root_node) { return false; }
	root_node = root_node.child("RenderGraph");

    pugi::xml_node render_nodes_node = root_node.child("RenderNodes");
	if (!render_nodes_node) return false;

    for (pugi::xml_node render_node = render_nodes_node.first_child(); render_node; render_node = render_node.next_sibling()) {
        std::string render_node_type = render_node.name();
        std::string node_name = render_node.attribute("name").as_string();
        
        if(render_node_type == "GraphicsRenderNode"s) {
            std::shared_ptr<GraphicsRenderNodeConfig> render_node_config = std::make_shared<GraphicsRenderNodeConfig>();
            render_node_config->init(device, Application::GetRenderer().getResourcesManager(), render_node);
            m_graphics_cfg_name_map[node_name] = std::move(render_node_config);
        }
    }

    return true;
}

void RenderGraph::destroy() {
    size_t sz = m_render_nodes.size();
    for(size_t i = 0u; i < sz; ++i) {
        RenderNodePtr& render_node = m_render_nodes[i];
        render_node->destroy();
    }
    m_render_nodes.clear();
    m_read_map.clear();
    m_written_map.clear();
    m_adjency_list.clear();
    m_rev_adjency_list.clear();
    m_topologically_sorted_nodes.clear();
    m_render_node_sort_idx.clear();
    m_dependency_levels.clear();
}

const std::shared_ptr<GraphicsRenderNodeConfig>& RenderGraph::getGraphicsRenderNodeConfig(const std::string& config_mame) const {
    return m_graphics_cfg_name_map.at(config_mame);
}

void RenderGraph::add_pass(std::shared_ptr<RenderNode> render_node) {
    for(const auto&[written_resource_name, written_resource_ptr] : render_node->getWrittenResourcesMap()) {
        m_written_map[written_resource_name].insert(render_node);
    }
    
    for(const auto&[read_resource_name, read_resource_ptr] : render_node->getReadResourcesMap()) {
        m_read_map[read_resource_name].insert(render_node);
    }

    m_render_nodes.push_back(std::move(render_node));
    m_sorted = false;
}

void RenderGraph::topological_sort() {
    if(m_render_nodes.size() == 1u) {
        m_topologically_sorted_nodes.clear();
        m_topologically_sorted_nodes.push_back(m_render_nodes.front());
        m_render_node_sort_idx.clear();
        m_render_node_sort_idx[m_topologically_sorted_nodes.front()] = 0u;
        m_sorted = true;
        return;
    }

    for(const RenderNodePtr& render_node : m_render_nodes) {
        for(const auto&[written_resource_name, written_resource_ptr] : render_node->getWrittenResourcesMap()) {
            if(!m_read_map.contains(written_resource_name)) continue;
            for(const RenderNodePtr& reader_node : m_read_map.at(written_resource_name)) {
                m_adjency_list[render_node].insert(reader_node);
                m_rev_adjency_list[reader_node].insert(render_node);
            }
        }
    }

    if(m_adjency_list.size() == 0u) {
        m_topologically_sorted_nodes.clear();
        m_topologically_sorted_nodes.insert(m_topologically_sorted_nodes.end(), m_render_nodes.begin(), m_render_nodes.end());
        m_render_node_sort_idx.clear();
        for(size_t i = 0; i < m_topologically_sorted_nodes.size(); ++i) {
            const RenderNodePtr& render_node = m_topologically_sorted_nodes.at(i);
            m_render_node_sort_idx[render_node] = i;
        }
        m_sorted = true;
        return;
    }

    m_topologically_sorted_nodes.clear();
    m_render_node_sort_idx.clear();
    m_topologically_sorted_nodes.reserve(m_render_nodes.size());

    std::unordered_map<RenderNodePtr, int> count_edges_to_node;
    count_edges_to_node.reserve(m_render_nodes.size());
    for (const auto&[node_ptr, adj_nodes_set] : m_adjency_list) {
        for (const RenderNodePtr& adj_node_ptr : adj_nodes_set) {
            count_edges_to_node[adj_node_ptr]++;
        }
    }

    std::stack<std::pair<RenderNodePtr, bool>> stack;
    for (const auto&[node_ptr, ct] : count_edges_to_node) {
        if (!ct) {
            stack.push({ node_ptr, false });
        }
    }

    std::unordered_set<RenderNodePtr> visited_set;
    visited_set.reserve(m_render_nodes.size());

    while (stack.size()) {
        auto [node_ptr, is_processed] = stack.top();
        stack.pop();

        if (is_processed) {
            m_topologically_sorted_nodes.push_back(node_ptr);
            continue;
        }

        if (!visited_set.count(node_ptr)) {
            visited_set.insert(node_ptr);
            stack.push({node_ptr, true});

            std::for_each(
                m_adjency_list.at(node_ptr).begin(),
                m_adjency_list.at(node_ptr).end(),
                [&stack, &visited_set](const RenderNodePtr& new_node_ptr) {
                    if (!visited_set.count(new_node_ptr)) {
                        stack.push({ new_node_ptr, false });
                    }
                }
            );
        }
    }

    std::reverse(m_topologically_sorted_nodes.begin(), m_topologically_sorted_nodes.end());
    for(size_t i = 0; i < m_topologically_sorted_nodes.size(); ++i) {
        const RenderNodePtr& render_node = m_topologically_sorted_nodes.at(i);
        m_render_node_sort_idx[render_node] = i;
    }

    m_sorted = true;
}

void RenderGraph::build_dependency_levels() {
    m_dependency_levels.clear();

    std::unordered_map<RenderNodePtr, int> node_dist_to_start;
    for (const RenderNodePtr& node_ptr : m_topologically_sorted_nodes) {
        node_dist_to_start[node_ptr] = 0;
    }

    int dependency_level_count = 1;

    for (RenderNodeList::reverse_iterator it = m_topologically_sorted_nodes.rbegin(); it != m_topologically_sorted_nodes.rend(); ++it) {
        const RenderNodePtr& node_ptr = *it;
        int max_neighbor_dist = -1;

        if(!m_adjency_list.contains(node_ptr)) continue;
        for (const RenderNodePtr& adj_node_ptr : m_adjency_list.at(node_ptr)) {
            max_neighbor_dist = std::max(max_neighbor_dist, node_dist_to_start[adj_node_ptr]);
        }

        if (max_neighbor_dist != -1) {
            node_dist_to_start[node_ptr] = 1 + max_neighbor_dist;
            dependency_level_count = std::max(1 + max_neighbor_dist, dependency_level_count);
        }
    }

    m_dependency_levels.resize(dependency_level_count);
    for (int level = 0; level < dependency_level_count; ++level) {
        std::shared_ptr<DependencyLevel> dependency_level = std::make_shared<DependencyLevel>(level);
        m_dependency_levels[level] = std::move(dependency_level);
    }

    for (const auto&[node_ptr, level] : node_dist_to_start) {
        m_dependency_levels[level]->addNode(node_ptr);
    }

    for (int level = 0; level < dependency_level_count; ++level) {
        m_dependency_levels[level]->sortPipelines();
    }
}

const RenderGraph::RenderNodeList& RenderGraph::getTopologicallySortedNodes() {
    if(!m_sorted) {
        topological_sort();
        build_dependency_levels();
    }
    return m_topologically_sorted_nodes;
}

const RenderGraph::RenderNodePtr& RenderGraph::getRenderNodeByID(size_t id) const {
    return m_topologically_sorted_nodes.at(id);
}

RenderGraph::RenderNodePtr RenderGraph::getLastWritten(const RenderNodePtr& render_node, const std::string& gloabal_resuorce_name) const {
    size_t current_idx = getLastWrittenIdentity(render_node, gloabal_resuorce_name);
    if(current_idx == NO_ID) {
        return nullptr;
    }
    else {
        return m_topologically_sorted_nodes.at(current_idx);
    }
}

size_t RenderGraph::getLastWrittenIdentity(const RenderNodePtr& render_node, const std::string& gloabal_resuorce_name) const {
    size_t current_idx = NO_ID;
    if(!m_rev_adjency_list.contains(render_node)) return NO_ID;
    for (const RenderNodePtr& write_node : m_rev_adjency_list.at(render_node)) {
        if(!write_node->isWrittenGlobal(gloabal_resuorce_name)) continue;
        size_t write_node_idx = m_render_node_sort_idx.at(write_node);
        if(current_idx > write_node_idx) {
            current_idx = write_node_idx;
        }
    }
    return current_idx;
}

RenderGraph::RenderNodePtr RenderGraph::getLastRead(const RenderNodePtr& render_node, const std::string& gloabal_resuorce_name) const {
    size_t current_idx = getLastReadIdentity(render_node, gloabal_resuorce_name);
    if(current_idx == NO_ID) {
        return nullptr;
    }
    else {
        return m_topologically_sorted_nodes.at(current_idx);
    }
}

size_t RenderGraph::getLastReadIdentity(const RenderNodePtr& render_node, const std::string& gloabal_resuorce_name) const {
    size_t current_idx = NO_ID;
    if(!m_rev_adjency_list.contains(render_node)) return NO_ID;
    for (const RenderNodePtr& write_node : m_rev_adjency_list.at(render_node)) {
        if(!write_node->isWrittenGlobal(gloabal_resuorce_name)) continue;
        size_t write_node_idx = m_render_node_sort_idx.at(write_node);
        if(current_idx > write_node_idx) {
            current_idx = write_node_idx;
        }
    }
    return current_idx;
}

size_t RenderGraph::getTopologicalIdentity(const RenderNodePtr& render_node) const {
    return m_render_node_sort_idx.at(render_node);
}