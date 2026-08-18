#include "managers_menu_ui.h"

#include "../../application.h"
#include "../../scene/animation_manager.h"
#include "../../scene/skeleton_manager.h"
#include "imgui_tools.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>

ManagersMenuUI::ManagersMenuUI() {
    //m_animation_manager = Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->getAnimationManager();
}

ManagersMenuUI::~ManagersMenuUI() {

}

bool ManagersMenuUI::VOnRestore() {
	return true;
}

bool ManagersMenuUI::VOnRender(const GameTimerDelta& delta, uint32_t image_index) {
    using namespace std::literals;
    if (!m_is_visible) return true;

	if (ImGui::Begin("Managers Menu")) {
        if(const std::shared_ptr<AnimationManager>& animation_manager = Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->getAnimationManager()) {
            if (ImGui::CollapsingHeader("Animation Manager")) {
                for(const auto&[clip_name, clip_data] : animation_manager->GetClipMap()) {
                    if (ImGui::TreeNode(clip_name.c_str())) {

						if (ImGui::TreeNode("ControlledNodes")) {
							if (ImGui::TreeNode("FlatView")) {
								for(const std::shared_ptr<AnimationNode>& anim_node : clip_data) {
                            		printSceneNode(anim_node);
                        		}
								ImGui::TreePop();
							}
							if (ImGui::TreeNode("TreeView")) {
								for(const std::shared_ptr<AnimationNode>& anim_node : animation_manager->GetClipRoots(clip_name)) {
                            		printSceneNode(
										anim_node,
										[&animation_manager, &clip_name]
										(const std::shared_ptr<SceneNode>& child_node) {
											const std::shared_ptr<Scene>& scene = child_node->GetScene();
											if(std::shared_ptr<AnimationNode> child_anim = std::dynamic_pointer_cast<AnimationNode>(scene->getProperty(child_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_ANIMATION))) {
												return animation_manager->GetClipMap().at(clip_name).contains(child_anim);
											}
											return false;
										}
									);
                        		}
								ImGui::TreePop();
							}
							
							ImGui::TreePop();
						}

						if (ImGui::TreeNode("AnimationControl")) {
							float total_animation_time = animation_manager->GetClipTotalTime(clip_name);
							float current_time = animation_manager->GetClipCurrentTime(clip_name);

							if (ImGui::SliderFloat("Time", ((float*)&current_time), 0.0f, total_animation_time, "%.4f")) {
								animation_manager->SetClipCurrentTime(clip_name, current_time);
							}

							ImGui::PushID("Play");
							if (ImGui::Button("Play")) {
								animation_manager->Play(clip_name);
							}
							ImGui::PopID();

							ImGui::SameLine();
							ImGui::PushID("Pause");
							if (ImGui::Button("Pause")) {
								animation_manager->Pause(clip_name);
							}
							ImGui::PopID();

							ImGui::SameLine();
							ImGui::PushID("Stop");
							if (ImGui::Button("Stop")) {
								animation_manager->Stop(clip_name);
							}
							ImGui::PopID();

							ImGui::TreePop();
						}
                        ImGui::TreePop();
                    }
                }
		    }
        }

		if(const std::shared_ptr<SkeletonManager>& skeleton_manager = Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->getSkeletonManager()) {
			if (ImGui::CollapsingHeader("Skeletons Manager")) {
				for(const auto&[skin_name, skin_data] : skeleton_manager->getSkinMap()) {
					if (ImGui::TreeNode(skin_name.c_str())) {
						
						if (ImGui::TreeNode("SkinData")) {

							if (ImGui::TreeNode("FlatView")) {
								size_t ct = skin_data->inverse_bind_matrices.size();
								for(size_t joint_idx = 0u; joint_idx < ct; ++joint_idx) {
									const std::shared_ptr<BoneNode>& bone_node = skin_data->joint_to_bone_map.at(joint_idx);

									const Scene::Hierarchy& hierarchy_node = bone_node->VGetHierarchy();
    								Scene::NodeTypeFlags node_type_flags = bone_node->GetScene()->getNodeTypeFlags(bone_node->VGetNodeIndex());
    								std::string node_name = bone_node->Get().Name();
    								std::string header = getSummaryForHierarchyStr(bone_node->VGetNodeIndex(), hierarchy_node, node_type_flags, node_name);

									std::string joint_str = std::to_string(joint_idx) + " joint - "s + header;
									if(ImGui::TreeNode(joint_str.c_str())) {
										if (ImGui::TreeNode("ToRootMatrice")) {
											printMatrixImGUI(bone_node->Get().ToRoot());
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("MeshRootMatrice")) {
											printMatrixImGUI(bone_node->getBoneDataMap().at(skin_name).m_mesh_root_node->Get().FromRoot());
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("InverseBindMatrice")) {
											printMatrixImGUI(skin_data->inverse_bind_matrices.at(joint_idx));
											ImGui::TreePop();
										}
									
										if (ImGui::TreeNode("FinalMatrice")) {
											printMatrixImGUI(skin_data->final_matrices.at(joint_idx));
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("FinalDoubleQuaternion")) {
											glm::dualquat dq(skin_data->dual_quats.at(joint_idx));
											printQuatImGUI(dq.real);
											printQuatImGUI(dq.dual);
											ImGui::TreePop();
										}
										printSceneNode(skin_data->joint_to_bone_map.at(joint_idx));

										ImGui::TreePop();
									}
								}

								ImGui::TreePop();
							}

							if (ImGui::TreeNode("TreeView")) {
								for(size_t joint_idx : skin_data->root_joints) {
									const std::shared_ptr<BoneNode>& bone_node = skin_data->joint_to_bone_map.at(joint_idx);

									const Scene::Hierarchy& hierarchy_node = bone_node->VGetHierarchy();
    								Scene::NodeTypeFlags node_type_flags = bone_node->GetScene()->getNodeTypeFlags(bone_node->VGetNodeIndex());
    								std::string node_name = bone_node->Get().Name();
    								std::string header = getSummaryForHierarchyStr(bone_node->VGetNodeIndex(), hierarchy_node, node_type_flags, node_name);

									std::string joint_str = std::to_string(joint_idx) + " joint - "s + header;
									if(ImGui::TreeNode(joint_str.c_str())) {
										if (ImGui::TreeNode("ToRootMatrice")) {
											printMatrixImGUI(bone_node->Get().ToRoot());
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("MeshRootMatrice")) {
											printMatrixImGUI(bone_node->getBoneDataMap().at(skin_name).m_mesh_root_node->Get().FromRoot());
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("InverseBindMatrice")) {
											printMatrixImGUI(skin_data->inverse_bind_matrices.at(joint_idx));
											ImGui::TreePop();
										}
									
										if (ImGui::TreeNode("FinalMatrice")) {
											printMatrixImGUI(skin_data->final_matrices.at(joint_idx));
											ImGui::TreePop();
										}

										if (ImGui::TreeNode("FinalDoubleQuaternion")) {
											glm::dualquat dq(skin_data->dual_quats.at(joint_idx));
											printQuatImGUI(dq.real);
											printQuatImGUI(dq.dual);
											ImGui::TreePop();
										}

										ImGui::SeparatorText("SceneData");

										printSceneNode(
											skin_data->joint_to_bone_map.at(joint_idx),
											[&skeleton_manager, &skin_name, &skin_data]
											(const std::shared_ptr<SceneNode>& child_node) {
												const std::shared_ptr<Scene>& scene = child_node->GetScene();
												if(std::shared_ptr<BoneNode> child_bone = std::dynamic_pointer_cast<BoneNode>(scene->getProperty(child_node->VGetNodeIndex(), Scene::NODE_TYPE_FLAG_BONE))) {
													return skin_data->bone_to_joint_map.contains(child_bone);
												}
												return false;
											}
										);

										ImGui::TreePop();
									}
								}

								ImGui::TreePop();
							}
							
							ImGui::TreePop();
						}


						if (ImGui::TreeNode("SkinControl")) {

							ImGui::PushID("ResetSkin");
							if (ImGui::Button("ResetSkin")) {
								skeleton_manager->resetSkin(skin_name);
							}
							ImGui::PopID();
							
							ImGui::TreePop();
						}

						ImGui::TreePop();
					}
				}
			}
		}
	}
	ImGui::End();

	return true;
}

void ManagersMenuUI::VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) {}

int ManagersMenuUI::VGetZOrder() const {
	return 1;
}

void ManagersMenuUI::VSetZOrder(int const zOrder) {}