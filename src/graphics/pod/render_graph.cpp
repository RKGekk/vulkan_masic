#include "render_graph.h"

#include "vulkan_device.h"

bool RenderGraph::init(std::shared_ptr<VulkanDevice> device) {
    m_device = std::move(device);

    return true;
}

void RenderGraph::destroy() {

}

void RenderGraph::add_pass(std::shared_ptr<RenderNode> render_node) {

    for(const RenderNodePtr& graph_node : m_render_nodes) {
        for(const auto&[written_resource_name, written_resource_ptr] : render_node->getWrittenResourcesMap()) {
            if(graph_node->isRead(written_resource_name)) {
                m_adjency_list[render_node].insert(graph_node);
            }
        }
    }

    m_render_nodes.push_back(std::move(render_node));
}

void RenderGraph::topological_sort() {
    m_topologically_sorted_nodes.clear();
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

        for (const RenderNodePtr& adj_node_ptr : m_adjency_list.at(node_ptr)) {
            max_neighbor_dist = std::max(max_neighbor_dist, node_dist_to_start[adj_node_ptr]);
        }

        if (max_neighbor_dist != -1) {
            node_dist_to_start[node_ptr] = 1 + max_neighbor_dist;
            dependency_level_count = std::max(1 + max_neighbor_dist, dependency_level_count);
        }
    }

    m_dependency_levels.resize(dependency_level_count);
    for (const auto&[node_ptr, level] : node_dist_to_start) {
        std::shared_ptr<DependencyLevel> dependency_level = std::make_shared<DependencyLevel>(level);
        m_dependency_levels[level] = std::move(dependency_level);
    }
}