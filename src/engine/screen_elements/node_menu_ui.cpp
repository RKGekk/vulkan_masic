#include "node_menu_ui.h"

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../scene/scene.h"
#include "../views/human_view.h"
#include "../../scene/nodes/scene_node.h"
#include "../../scene/nodes/mesh_node.h"
#include "../../scene/nodes/camera_node.h"
#include "../../scene/nodes/basic_camera_node.h"
#include "../../scene/nodes/aabb_node.h"
#include "imgui_tools.h"

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

                const std::unordered_map<Scene::NodeIndex, Scene::NameIndex>& node_name_map = scene->getNodeNameMap();
                const std::vector<std::string>& node_names = scene->getNodeNames();
                std::string node_name = node_name_map.contains(node_index) ? "--> n-"s + node_names.at(node_name_map.at(node_index)) : "-- > n-N"s;

                std::string header = getSummaryForHierarchyStr(node_index, hierarchy_node, node_type_flags, node_name);
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
                            std::string node_type_v = node_type_flags_raw + " --> "s + getNodeFlagsStr(node_type_flags);
                            ImGui::InputText("Type Flags", const_cast<char*>(node_type_v.c_str()), 128, ImGuiInputTextFlags_ReadOnly);
                        }

                        ImGui::SeparatorText("Transforms");

                        if (ImGui::TreeNode("Local Transform")) {
                            ImGui::PushID("Local Transform");
                            printMatrixImGUI(scene->getNodeLocalTransform(node_index));
        					ImGui::TreePop();
                            ImGui::PopID();
        				}

                        if (ImGui::TreeNode("Global Transform")) {
                            ImGui::PushID("Global Transform");
                            printMatrixImGUI(scene->getNodeGlobalTransform(node_index));
        					ImGui::TreePop();
                            ImGui::PopID();
        				}

                        ImGui::SeparatorText("Node Types");

                        std::shared_ptr<SceneNode> pMeshNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_MESH);
                        if(pMeshNode && ImGui::TreeNode("Mesh")) {
                            std::shared_ptr<MeshNode> pMesh = std::dynamic_pointer_cast<MeshNode>(pMeshNode);
                            printMeshNodeImGUI(pMesh);
                            ImGui::TreePop();
                        }

                        std::shared_ptr<SceneNode> pCameraNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_CAMERA);
                        if(pCameraNode && ImGui::TreeNode("Camera")) {
                            std::shared_ptr<CameraNode> pCamera = std::dynamic_pointer_cast<CameraNode>(pCameraNode);
                            printCameraNodeImGUI(pCamera);
                            ImGui::TreePop();
                        }

                        std::shared_ptr<SceneNode> pAABBNode = scene->getProperty(node_index, Scene::NODE_TYPE_FLAG_AABB);
                        if(pAABBNode && ImGui::TreeNode("AABB")) {
                            std::shared_ptr<AABBNode> pAABB = std::dynamic_pointer_cast<AABBNode>(pAABBNode);
                            printAABBNodeImGUI(pAABB);
                            ImGui::TreePop();
                        }
                        
						ImGui::TreePop();
					}

					ImGui::TreePop();
				}
            }
        }
        if (ImGui::CollapsingHeader("Hierarchy Tree View")) {
            DrawNodes(scene->getRootNode());
        }
    }
    
	ImGui::End();

    return true;
}

void NodeMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int NodeMenuUI::VGetZOrder() const {
    return 1;
}

void NodeMenuUI::VSetZOrder(int const zOrder) {}