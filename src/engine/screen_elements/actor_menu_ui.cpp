#include "actor_menu_ui.h"

#include <imgui.h>

#include "../../application.h"
#include "../../actors/actor.h"
#include "../base_engine_logic.h"
#include "../../actors/transform_component.h"
#include "../../actors/camera_component.h"
#include "../../actors/model_component.h"
#include "imgui_tools.h"

ActorMenuUI::ActorMenuUI() : m_actor_id(INVALID_ACTOR_ID) {
	
}

ActorMenuUI::~ActorMenuUI() {}

bool ActorMenuUI::VOnRestore() {
	return true;
}

bool ActorMenuUI::VOnRender(const GameTimerDelta& delta) {
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
				ImGui::Text(act->GetName().c_str());
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
					ImGui::Text(resource_name.c_str());
				}
			}
			
		}
        //ImGui::PopButtonRepeat();
	}
	ImGui::End();

	return true;
}

void ActorMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int ActorMenuUI::VGetZOrder() const {
	return 1;
}

void ActorMenuUI::VSetZOrder(int const zOrder) {}