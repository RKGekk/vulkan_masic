#include "node_menu_ui.h"

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../scene/scene.h"
#include "../views/human_view.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"
#include "../../scene/nodes/camera_node.h"
#include "../../scene/nodes/aabb_node.h"

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
                    if (ImGui::TreeNode("Hierarchy")) {
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
                    
                    if (ImGui::TreeNode("Properties")) {
                        
                        ImGui::SeparatorText("Properties");

                        if(node_name_map.contains(node_index)) {
                            std::string node_name = node_names.at(node_name_map.at(node_index)).c_str();
                            ImGui::InputText("Name", const_cast<char*>(node_name.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                        }

                        if(scene->getNodeTypeFlagsMap().contains(node_index)) {
                            std::string node_type_flags_raw = std::to_string(node_type_flags);
                            std::string node_type_v = node_type_flags_raw + " --> " + node_type_flags_str;
                            ImGui::InputText("Type Flags", const_cast<char*>(node_type_v.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                        }

                        ImGui::SeparatorText("Transforms");

                        if (ImGui::TreeNode("Local Transform")) {
        					if (ImGui::InputFloat4("R1", ((float*)&scene->getNodeLocalTransform(node_index)) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R2", ((float*)&scene->getNodeLocalTransform(node_index)) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R3", ((float*)&scene->getNodeLocalTransform(node_index)) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R4", ((float*)&scene->getNodeLocalTransform(node_index)) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					ImGui::TreePop();
        				}

                        if (ImGui::TreeNode("Global Transform")) {
        					if (ImGui::InputFloat4("R1", ((float*)&scene->getNodeGlobalTransform(node_index)) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R2", ((float*)&scene->getNodeGlobalTransform(node_index)) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R3", ((float*)&scene->getNodeGlobalTransform(node_index)) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					if (ImGui::InputFloat4("R4", ((float*)&scene->getNodeGlobalTransform(node_index)) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
        					ImGui::TreePop();
        				}

                        if (ImGui::TreeNode("Decompose Local")) {
					    	glm::vec3 scale_xm;
					    	glm::quat rotation_xm;
					    	glm::vec3 translation_xm;

                            glm::mat4x4 mat = scene->getNodeLocalTransform(node_index);
                            translation_xm = mat[3];

                            for (int i = 0; i < 3; ++i) {
                                scale_xm[i] = glm::length(mat[i]);
                                mat[i] /= scale_xm[i];
                            }
                        
                            rotation_xm = glm::toQuat(mat);

					    	glm::vec3 pyr_xm = glm::eulerAngles(rotation_xm);
					    	glm::vec3 ypr_xm(
					    		glm::degrees(pyr_xm.y),
					    		glm::degrees(pyr_xm.x),
					    		glm::degrees(pyr_xm.z)
					    	);

					    	if (ImGui::InputFloat4("Rq", ((float*)&rotation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Tr", ((float*)&translation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Ypr", ((float*)&ypr_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	
					    	ImGui::TreePop();
					    }

                        if (ImGui::TreeNode("Decompose Global")) {
					    	glm::vec3 scale_xm;
					    	glm::quat rotation_xm;
					    	glm::vec3 translation_xm;

                            glm::mat4x4 mat = scene->getNodeGlobalTransform(node_index);
                            translation_xm = mat[3];

                            for (int i = 0; i < 3; ++i) {
                                scale_xm[i] = glm::length(mat[i]);
                                mat[i] /= scale_xm[i];
                            }
                        
                            rotation_xm = glm::toQuat(mat);

					    	glm::vec3 pyr_xm = glm::eulerAngles(rotation_xm);
					    	glm::vec3 ypr_xm(
					    		glm::degrees(pyr_xm.y),
					    		glm::degrees(pyr_xm.x),
					    		glm::degrees(pyr_xm.z)
					    	);

					    	if (ImGui::InputFloat4("Rq", ((float*)&rotation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Sc", ((float*)&scale_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Tr", ((float*)&translation_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	if (ImGui::InputFloat3("Ypr", ((float*)&ypr_xm), "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
					    	
					    	ImGui::TreePop();
					    }

                        ImGui::SeparatorText("Node Types");


                        
						ImGui::TreePop();
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