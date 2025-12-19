#include "actor_menu_ui.h"

#include <imgui.h>

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../../actors/actor.h"
#include "../../actors/transform_component.h"

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
            ImGui::Text(m_actor_name.c_str());

            ImGui::InputInt("ActID", &m_actor_id);
            //ImGui::DragInt("Actor ID", &m_actor_id, 1, 0, 3);
            //ImGui::SliderInt("Actor ID", &m_actor_id, 0, 3);

            // ImGui::SameLine();
            // ImGui::PushID(" - ");
            // if (ImGui::Button(" - ")) {
            //     --m_actor_id;
            // }
            // ImGui::PopID();

            // ImGui::SameLine();
            // ImGui::PushID(" + ");
            // if (ImGui::Button(" + ")) {
            //     ++m_actor_id;
            // }
            // ImGui::PopID();

            Application& app = Application::Get();
            std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
            std::shared_ptr<Actor> act = game_logic->VGetActor(m_actor_id).lock();
			if (act) {
                m_actor_name = act->GetName();
				std::shared_ptr<TransformComponent> tc = act->GetComponent<TransformComponent>().lock();
				if (tc) {
					// m_transform = tc->GetTransform4x4f();
                	// DirectX::XMMATRIX transform_xm = DirectX::XMLoadFloat4x4(&m_transform);
					// DirectX::XMVECTOR scale_xm;
					// DirectX::XMVECTOR rotation_xm;
					// DirectX::XMVECTOR translation_xm;
					// DirectX::XMMatrixDecompose(&scale_xm, &rotation_xm, &translation_xm, transform_xm);
					
					// m_yaw_pith_roll = tc->GetYawPitchRoll3f();
					// m_yaw_pith_roll.x = DirectX::XMConvertToDegrees(m_yaw_pith_roll.x);
					// m_yaw_pith_roll.y = DirectX::XMConvertToDegrees(m_yaw_pith_roll.y);
					// m_yaw_pith_roll.z = DirectX::XMConvertToDegrees(m_yaw_pith_roll.z);
                	// DirectX::XMStoreFloat3(&m_scale, scale_xm);
					// DirectX::XMStoreFloat4(&m_rot_quat, rotation_xm);
					// DirectX::XMStoreFloat3(&m_translate, translation_xm);
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