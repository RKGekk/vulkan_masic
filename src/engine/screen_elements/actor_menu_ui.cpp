#include "actor_menu_ui.h"

#include <imgui.h>

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../actors/actor.h"
#include "../../actors/transform_component.h"
#include "../../actors/camera_component.h"

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
						if (ImGui::InputFloat4("R1", ((float*)&tc->GetTransform()) + 0, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
						if (ImGui::InputFloat4("R2", ((float*)&tc->GetTransform()) + 4, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
						if (ImGui::InputFloat4("R3", ((float*)&tc->GetTransform()) + 8, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
						if (ImGui::InputFloat4("R4", ((float*)&tc->GetTransform()) + 12, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}
						ImGui::TreePop();
					}

					if (ImGui::TreeNode("Decompose")) {
						glm::vec3 scale_xm;
						glm::quat rotation_xm;
						glm::vec3 translation_xm;
        				tc->Decompose(translation_xm, rotation_xm, scale_xm);
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
					if (ImGui::InputFloat("Fov", ((float*)&cc_fov), 0.0f, 0.0f, "%.4f", ImGuiInputTextFlags_ReadOnly)) {}

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