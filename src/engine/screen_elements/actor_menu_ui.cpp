#include "actor_menu_ui.h"

#include <imgui.h>

#include "../../application.h"
#include "../../actors/actor.h"
#include "../base_engine_logic.h"
#include "../../actors/transform_component.h"
#include "../../actors/transform_animation_component.h"
#include "../../actors/camera_component.h"
#include "../../actors/model_component.h"
#include "imgui_tools.h"

ActorMenuUI::ActorMenuUI() : m_actor_id(INVALID_ACTOR_ID) {
	
}

ActorMenuUI::~ActorMenuUI() {}

bool ActorMenuUI::VOnRestore() {
	return true;
}

bool ActorMenuUI::VOnRender(const GameTimerDelta& delta, uint32_t image_index) {
	using namespace std;
	if (!m_is_visible) return true;

	if (ImGui::Begin("Actor Menu")) {
        //ImGui::PushButtonRepeat(false);
		if (ImGui::CollapsingHeader("Actors")) {
            ImGui::InputInt("Actor ID", &m_actor_id);

			Application& app = Application::Get();
    		std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
            std::shared_ptr<Actor> act = game_logic->VGetActor(m_actor_id).lock();
			if (act) {
				ImGui::Text("%s", act->GetName().c_str());
				std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>().lock();
				if (tc) {
					ImGui::SeparatorText("TransformComponent");

					if (ImGui::TreeNode("Matrix")) {
						printMatrixImGUI(tc->GetTransform());
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Edit")) {
						editMatrixImGUI(
							tc->GetTransform(),
							[tc](glm::bvec3 tsr, glm::vec3 tr, glm::vec3 sc, glm::quat rot){
								if(tsr.x && !tsr.y && !tsr.z) {
									tc->SetTranslation3f(tr);
								}
								else {
									tc->SetTransform(tr, rot, sc);
								}
							}
						);

						ImGui::TreePop();
					}
				}
				std::shared_ptr<CameraComponent> cc = act->GetComponent<CameraComponent>().lock();
				if(cc) {
					ImGui::SeparatorText("CameraComponent");
					
					float cc_near = cc->GetNear();
					if (ImGui::InputFloat("Near", ((float*)&cc_near), 0.0f, 0.0f, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

					float cc_far = cc->GetFar();
					if (ImGui::InputFloat("Far", ((float*)&cc_far), 0.0f, 0.0f, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

					float cc_fov = cc->GetFov();
					cc_fov = glm::degrees(cc_fov);
					if(ImGui::SliderFloat("Fov", ((float*)&cc_fov), 30.0f, 120.0f)) {
						cc->SetFov(glm::radians(cc_fov));
					}

				}
				std::shared_ptr<ModelComponent> mc = act->GetComponent<ModelComponent>().lock();
				if(mc) {
					ImGui::SeparatorText("ModelComponent");

					const std::string& resource_name = mc->GetResourceName();
					ImGui::Text("%s", resource_name.c_str());
				}
				std::shared_ptr<TransformAnimationComponent> ac = act->GetComponent<TransformAnimationComponent>().lock();
				if(ac) {
					ImGui::SeparatorText("TransformAnimationComponent");

					for(const auto&[anim_name, anim_data] : ac->GetAnimationMap()) {

						if (ImGui::TreeNode(anim_name.c_str())) {
							
							float total_animation_time = ac->GetTotalAnimationTime(anim_name);
							float current_time = ac->GetCurrentAnimationTime(anim_name);

							if (ImGui::SliderFloat("Time", ((float*)&current_time), 0.0f, total_animation_time, "%.4f")) {
								ac->SetCurrentAnimationTime(anim_name, current_time);
							}

							ImGui::PushID("Play");
							if (ImGui::Button("Play")) {
								ac->Play(anim_name);
							}
							ImGui::PopID();

							ImGui::SameLine();
							ImGui::PushID("Pause");
							if (ImGui::Button("Pause")) {
								ac->Pause(anim_name);
							}
							ImGui::PopID();

							ImGui::SameLine();
							ImGui::PushID("Stop");
							if (ImGui::Button("Stop")) {
								ac->Stop(anim_name);
							}
							ImGui::PopID();

							if (ImGui::CollapsingHeader("Animation Channels")) {
								int ct = 0u;
								if (ImGui::TreeNode("Translation channels")) {
									for (KeyframeMatrixTranslation& tkf : anim_data->TranslationKeyframes) {
										std::string trkf_name = "Trc "s + std::to_string(ct);
										std::string tgkf_name = "Tgc "s + std::to_string(ct);
										if (ImGui::SliderFloat("T Time Point", ((float*)&tkf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
										if (ImGui::SliderFloat3(trkf_name.c_str(), ((float*)&tkf.Translation), -2.0f, 2.0f)) {}
										if (ImGui::SliderFloat3(tgkf_name.c_str(), ((float*)&tkf.Tangent), -3.0f, 3.0f)) {}
									
										++ct;
									}
									ImGui::TreePop();
								}
								ct = 0u;
								if (ImGui::TreeNode("Rotation channels")) {
									for (KeyframeMatrixRotation& rkf : anim_data->RotationKeyframes) {
										std::string rkf_name = "YPRc "s + std::to_string(ct);
										if (ImGui::SliderFloat("R Time Point", ((float*)&rkf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
										glm::vec3 pyr = glm::eulerAngles(rkf.RotationQuat);
										pyr.x = glm::degrees(pyr.x);
										pyr.y = glm::degrees(pyr.y);
										pyr.z = glm::degrees(pyr.z);
										if (ImGui::SliderFloat3(rkf_name.c_str(), ((float*)&pyr), -180.0f, 180.0f)) {
											pyr.x = glm::radians(pyr.x);
											pyr.y = glm::radians(pyr.y);
											pyr.z = glm::radians(pyr.z);
											rkf.RotationQuat = glm::eulerAngleXYZ(pyr.x, pyr.y, pyr.z);
										}
										++ct;
									}
									ImGui::TreePop();
								}
								ct = 0u;
								if (ImGui::TreeNode("Scale channels")) {
									for (KeyframeMatrixScale& skf : anim_data->ScaleKeyframes) {
										std::string skf_name = "Scc "s + std::to_string(ct);
										if (ImGui::SliderFloat("S Time Point", ((float*)&skf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
										if (ImGui::SliderFloat3(skf_name.c_str(), ((float*)&skf.Scale), 0.0001f, 2.0f)) {}
										++ct;
									}
									ImGui::TreePop();
								}
							}

							ImGui::TreePop();
						}
					}
				}
			}
			
		}
        //ImGui::PopButtonRepeat();
	}
	ImGui::End();

	return true;
}

void ActorMenuUI::VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) {}

int ActorMenuUI::VGetZOrder() const {
	return 1;
}

void ActorMenuUI::VSetZOrder(int const zOrder) {}