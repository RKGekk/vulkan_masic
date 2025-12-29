#include "node_menu_ui.h"

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../scene/scene.h"
#include "../views/human_view.h"

NodeMenuUI::NodeMenuUI() {
}

NodeMenuUI::~NodeMenuUI() {}

bool NodeMenuUI::VOnRestore() {
    return true;
}

void DrawNodes(const std::shared_ptr<SceneNode>& current_node) {

}

bool NodeMenuUI::VOnRender(const GameTimerDelta& delta) {
    using namespace std::literals;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<HumanView> human_view = game_logic->GetHumanView();
    if(!human_view) return true;
    std::shared_ptr<Scene> scene = human_view->VGetScene();
    if(!scene) return true;

    if (ImGui::Begin("Nodes Menu")) {
        if (ImGui::CollapsingHeader("Flat Hierarchy")) {
            const std::vector<Scene::Hierarchy>& hierarchy = scene->getHierarchy();
            size_t node_ct = hierarchy.size();
            for(Scene::NodeIndex node_index = 0u; node_index < node_ct; ++node_index) {
                const Scene::Hierarchy& hierarchy_node = scene->getNodeHierarchy(node_index);

                Scene::NodeTypeFlags node_type_flags = scene->getNodeTypeFlags(node_index);
                std::string node_type_flags_str = "f"s;
                if(!node_type_flags) node_type_flags_str += "/N"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_MESH) node_type_flags_str += "/M"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_LIGHT) node_type_flags_str += "/L"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_CAMERA) node_type_flags_str += "/C"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_SHADOW_CAMERA) node_type_flags_str += "/Sh"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_AABB) node_type_flags_str += "/AABB"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_SPHERE) node_type_flags_str += "/Sp"s;
                if(node_type_flags & Scene::NODE_TYPE_FLAG_BONE) node_type_flags_str += "/B"s;

                const std::unordered_map<Scene::NodeIndex, Scene::NameIndex>& node_name_map = scene->getNodeNameMap();
                const std::vector<std::string>& node_names = scene->getNodeNames();
                std::string node_name = node_name_map.contains(node_index) ? "--> n-"s + node_names.at(node_name_map.at(node_index)) : "-- > n-N"s;

                std::string header = "i"s + std::to_string(node_index)
                                   + " p"s + (hierarchy_node.parent != Scene::NO_INDEX ? std::to_string(hierarchy_node.parent) : "N"s)
                                   + " c"s + (hierarchy_node.first_child != Scene::NO_INDEX ? std::to_string(hierarchy_node.first_child) : "N"s)
                                   + " s"s + (hierarchy_node.next_sibling != Scene::NO_INDEX ? std::to_string(hierarchy_node.next_sibling) : "N"s)
                                   + " l"s + std::to_string(hierarchy_node.level)
                                   + " "s + node_type_flags_str
                                   + " "s + node_name;
                if (ImGui::TreeNode(header.c_str())) {
                    ImGui::SeparatorText("Hierarchy");
                    if (ImGui::BeginTable("Hierarchy Table", 3)) {
                        ImGui::TableNextRow();
                        
                        ImGui::TableSetColumnIndex(0);
                        int parent = hierarchy_node.parent;
		                ImGui::InputInt("Parent", &parent, 0, 0, ImGuiInputTextFlags_ReadOnly);

                        int first_child = hierarchy_node.first_child;
                        ImGui::InputInt("Child", &first_child, 0, 0, ImGuiInputTextFlags_ReadOnly);

                        int next_sibling = hierarchy_node.next_sibling;
                        ImGui::InputInt("Sibling", &next_sibling, 0, 0, ImGuiInputTextFlags_ReadOnly);

                        int level = hierarchy_node.level;
                        ImGui::InputInt("Level", &level, 0, 0, ImGuiInputTextFlags_ReadOnly);
                        
                        ImGui::EndTable();
                    }

					ImGui::TreePop();
				}
            }
        }
		//DrawNodes(m_scene->GetRootCast());
	}
	ImGui::End();

    return true;
}

void NodeMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int NodeMenuUI::VGetZOrder() const {
    return 1;
}

void NodeMenuUI::VSetZOrder(int const zOrder) {}