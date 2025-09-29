#define _ENABLE_EXTENDED_ALIGNED_STORAGE
#include "scene.h"

#include <algorithm>
#include <numeric>

Scene::Scene() {
    m_local_transform.push_back(glm::mat4(1.0f));
    m_global_transform.push_back(glm::mat4(1.0f));
    m_hierarchy.push_back({});
}

int Scene::addNode(NodeIndex parent_index) {
    assert(parent_index > NO_INDEX);

    const NodeIndex new_node_index = (NodeIndex)m_hierarchy.size();
    Hierarchy hierarchy{}; 
	hierarchy.parent = parent_index;

    m_local_transform.push_back(glm::mat4(1.0f));
    m_global_transform.push_back(glm::mat4(1.0f));
    m_hierarchy.push_back(hierarchy);

    const NodeIndex old_child_index = m_hierarchy[parent_index].first_child;
    m_hierarchy[parent_index].first_child = new_node_index;
    if (old_child_index != NO_INDEX) {
        m_hierarchy[parent_index].first_child = new_node_index;
        m_hierarchy[new_node_index].next_sibling = old_child_index;
    }
    m_hierarchy[new_node_index].level = m_hierarchy[parent_index].level + 1;

    return new_node_index;
}

void Scene::markAsChanged(NodeIndex node_index) {
    const NodeLevel level = m_hierarchy[node_index].level;
    m_dirty_at_level[level].push_back(node_index);

    for (int n = m_hierarchy[node_index].first_child; n != -1; n = m_hierarchy[n].next_sibling) {
        markAsChanged(n);
    }
}

int Scene::findNodeByName(const std::string& name) {
    // Extremely simple linear search without any hierarchy reference
    // To support DFS/BFS searches separate traversal routines are needed

    size_t sz = m_hierarchy.size();
    for (size_t n = 0; n < sz; ++n) {
        if (m_node_name_map.contains(n)) {
            int name_index = m_node_name_map.at(n);
            if (name_index != NO_INDEX) {
                if (m_node_names[name_index] == name) {
                    return (int)n;
                }
            }
        }
    }

    return NO_INDEX;
}

// CPU version of global transform update []
bool Scene::recalculateGlobalTransforms() {
    bool was_updated = false;

    for (int lvl = 1; lvl < MAX_NODE_LEVEL; ++lvl) {
        for (int dirty_node_index : m_dirty_at_level[lvl]) {
            const int parent_node_index = m_hierarchy[dirty_node_index].parent;
            m_global_transform[dirty_node_index] = m_global_transform[parent_node_index] * m_local_transform[dirty_node_index];
        }
        was_updated |= !m_dirty_at_level[lvl].empty();
        m_dirty_at_level[lvl].clear();
    }

    return was_updated;
}

static void addUniqueIdx(std::vector<Scene::NodeIndex>& v, Scene::NodeIndex index) {
    if (!std::binary_search(v.begin(), v.end(), index)) {
        v.push_back(index);
    }
}

// Recurse down from a node and collect all nodes which are already marked for deletion
static void collectNodesToDelete(const std::vector<Scene::Hierarchy>& hierarchy, Scene::NodeIndex from_node, std::vector<Scene::NodeIndex>& collected_nodes) {
    for (int n = hierarchy[from_node].first_child; n != Scene::NO_INDEX; n = hierarchy[n].next_sibling) {
        addUniqueIdx(collected_nodes, n);
        collectNodesToDelete(hierarchy, n, collected_nodes);
    }
}

// From https://stackoverflow.com/a/64152990/1182653
// Delete a list of items from std::vector with indices in 'selection'
// e.g., eraseSelected({1, 2, 3, 4, 5}, {1, 3})  ->   {1, 3, 5}
//                         ^     ^    2 and 4 get deleted
template <typename T, typename Index = Scene::NodeIndex>
inline void eraseSelected(std::vector<T>& v, const std::vector<Index>& selection) {
    // cut off the elements moved to the end of the vector by std::stable_partition
    v.resize(
        std::distance(
            v.begin(),
            // std::stable_partition moves any element whose index is in 'selection' to the end
            std::stable_partition(
                v.begin(),
                v.end(),
                [&selection, &v](const T& item) {
                    return !std::binary_search(
                        selection.begin(),
                        selection.end(),
                        static_cast<Index>(static_cast<const T*>(&item) - &v[0])
                    );
                }
            )
        )
    );
}

Scene::NodeIndex Scene::findLastNonDeletedItem(const std::vector<Scene::NodeIndex>& new_indices, Scene::NodeIndex node) {
    // we have to be more subtle:
    //   if the (newIndices[firstChild_] == -1), we should follow the link and extract the last non-removed item
    //   ..
    if (node == NO_INDEX) return NO_INDEX;

    return (new_indices[node] == NO_INDEX) ? findLastNonDeletedItem(new_indices, m_hierarchy[node].next_sibling) : new_indices[node];
}

void shiftMapIndices(std::unordered_map<Scene::NodeIndex, uint32_t>& items, const std::vector<Scene::NodeIndex>& new_indices) {
    std::unordered_map<uint32_t, uint32_t> new_items;
    for (const auto& m : items) {
        int new_index = new_indices[m.first];
        if (new_index != Scene::NO_INDEX) {
            new_items[new_index] = m.second;
        }
    }
    items = new_items;
}

void Scene::deleteSceneNodes(const std::vector<NodeIndex>& nodes_indices_to_delete) {
    // 0) Add all the nodes down below in the hierarchy
    std::vector<NodeIndex> copy_of_indices_to_delete = nodes_indices_to_delete;
    for (uint32_t n : nodes_indices_to_delete) {
        collectNodesToDelete(m_hierarchy, n, copy_of_indices_to_delete);
    }

    // aux array with node indices to keep track of the moved ones [moved = [](node) { return (node != nodes[node]); ]
    std::vector<Scene::NodeIndex> nodes(m_hierarchy.size());
    std::iota(nodes.begin(), nodes.end(), 0u);

    // 1.a) Move all the indicesToDelete to the end of 'nodes' array (and cut them off, a variation of swap'n'pop for multiple elements)
    const size_t old_size = nodes.size();
    eraseSelected(nodes, copy_of_indices_to_delete);

    // 1.b) Make a newIndices[oldIndex] mapping table
    std::vector<Scene::NodeIndex> new_indices(old_size, NO_INDEX);
    for (Scene::NodeIndex i = 0; i < nodes.size(); i++) {
        new_indices[nodes[i]] = i;
    }

    // 2) Replace all non-null parent/firstChild/nextSibling pointers in all the nodes by new positions
    auto node_mover_fn = [this, &new_indices](Hierarchy& h) {
        return Hierarchy{
            .parent = (h.parent != NO_INDEX) ? new_indices[h.parent] : NO_INDEX,
            .first_child = findLastNonDeletedItem(new_indices, h.first_child),
            .next_sibling = findLastNonDeletedItem(new_indices, h.next_sibling)
        };
    };
    std::transform(m_hierarchy.begin(), m_hierarchy.end(), m_hierarchy.begin(), node_mover_fn);

    // 3) Finally throw away the hierarchy items
    eraseSelected(m_hierarchy, copy_of_indices_to_delete);

    // 4) As in mergeScenes() routine we also have to adjust all the "components" (i.e., meshes, materials, names and transformations)

    // 4a) Transformations are stored in arrays, so we just erase the items as we did with the scene.hierarchy_
    eraseSelected(m_local_transform, copy_of_indices_to_delete);
    eraseSelected(m_global_transform, copy_of_indices_to_delete);

    // 4b) All the maps should change the key values with the newIndices[] array
    shiftMapIndices(m_node_type_flags_map, new_indices);
    shiftMapIndices(m_node_mesh_map, new_indices);
    shiftMapIndices(m_node_group_map, new_indices);
    shiftMapIndices(m_node_material_map, new_indices);
    shiftMapIndices(m_node_name_map, new_indices);

    // 5) scene node names list is not modified, but in principle it can be (remove all non-used items and adjust the nameForNode_ map)
    // 6) Material names list is not modified also, but if some materials fell out of use
}

template <typename T>
inline void mergeVectors(std::vector<T>& v1, const std::vector<T>& v2) {
  v1.insert(v1.end(), v2.begin(), v2.end());
}

void Scene::shiftNodes(int start_offset, int node_count, int shift_amount) {
    auto shiftNode = [shift_amount](Scene::Hierarchy& node) {
        if (node.parent > Scene::NO_INDEX) node.parent += shift_amount;
        if (node.first_child > Scene::NO_INDEX) node.first_child += shift_amount;
        if (node.next_sibling > Scene::NO_INDEX) node.next_sibling += shift_amount;
        // node->level does not require to be shifted
    };

    // If there are too many nodes, we can use std::execution::par with std::transform:
    //	 std::transform(scene.hierarchy_.begin() + startOffset,
    //                  scene.hierarchy_.begin() + nodeCount,
    //                  scene.hierarchy_.begin() + startOffset,
    //                  shiftNode);
    //	 for (auto i = scene.hierarchy_.begin() + startOffset ; i != scene.hierarchy_.begin() + nodeCount ; i++)
    //		 shiftNode(*i);

    for (int i = 0; i < node_count; ++i) {
        shiftNode(m_hierarchy[i + start_offset]);
    }
}

// Add the items from otherMap shifting indices and values along the way
using ItemMap = std::unordered_map<uint32_t, uint32_t>;
void mergeMaps(ItemMap& m, const ItemMap& other_map, int index_offset, int item_offset) {
    for (const auto& i : other_map) {
        m[i.first + index_offset] = i.second + item_offset;
    }
}

void Scene::mergeScenes(const std::vector<Scene*>& scenes, const std::vector<glm::mat4>& root_transforms, const std::vector<uint32_t>& mesh_counts, bool merge_meshes, bool merge_materials) {
    // Create new root node
    m_hierarchy = {
        {
            .parent = NO_INDEX,
            .first_child = 1,
            .next_sibling = NO_INDEX,
            .level = 0
        }
    };

    m_node_name_map[0] = 0;
    m_node_names = { "NewRoot" };

    m_local_transform.push_back(glm::mat4(1.f));
    m_global_transform.push_back(glm::mat4(1.f));

    if (scenes.empty()) return;

    int offs = 1;
    int mesh_offs = 0;
    int name_offs = (int)m_node_names.size();
    int material_offs = 0;
    auto mesh_count = mesh_counts.begin();

    if (!merge_materials) {
        m_materials = scenes[0]->m_materials;
    }

    // FIXME: too much logic (for all the components in a scene, though mesh data and materials go separately - they're dedicated data lists)
    for (const Scene* s : scenes) {
        mergeVectors(m_local_transform, s->m_local_transform);
        mergeVectors(m_global_transform, s->m_global_transform);

        mergeVectors(m_hierarchy, s->m_hierarchy);

        mergeVectors(m_node_names, s->m_node_names);
        if (merge_materials) {
            mergeVectors(m_materials, s->m_materials);
        }

        const int node_count = (int)s->m_hierarchy.size();

        shiftNodes(offs, node_count, offs);

        mergeMaps(m_node_type_flags_map, s->m_node_type_flags_map, offs, merge_meshes ? mesh_offs : 0);
        mergeMaps(m_node_mesh_map, s->m_node_mesh_map, offs, merge_meshes ? mesh_offs : 0);
        mergeMaps(m_node_group_map, s->m_node_group_map, offs, merge_meshes ? mesh_offs : 0);
        mergeMaps(m_node_material_map, s->m_node_material_map, offs, merge_materials ? material_offs : 0);
        mergeMaps(m_node_name_map, s->m_node_name_map, offs, name_offs);

        offs += node_count;

        material_offs += (int)s->m_materials.size();
        name_offs += (int)s->m_node_names.size();

        if (merge_meshes) {
            mesh_offs += *mesh_count;
            mesh_count++;
        }
    }

    // fixing 'nextSibling' fields in the old roots (zero-index in all the scenes)
    offs = 1;
    int idx = 0;
    for (const Scene* s : scenes) {
        const int node_count = (int)s->m_hierarchy.size();
        const bool is_last = (idx == scenes.size() - 1);
        // calculate new next sibling for the old scene roots
        const int next = is_last ? NO_INDEX : offs + node_count;

        m_hierarchy[offs].next_sibling = next;
        // attach to new root
        m_hierarchy[offs].parent = 0;

        // transform old root nodes, if the transforms are given
        if (!root_transforms.empty()) {
            m_local_transform[offs] = root_transforms[idx] * m_local_transform[offs];
        }

        offs += node_count;
        idx++;
    }

    // now, shift levels of all nodes below the root
    for (auto i = m_hierarchy.begin() + 1; i != m_hierarchy.end(); ++i) {
        i->level++;
    }
}