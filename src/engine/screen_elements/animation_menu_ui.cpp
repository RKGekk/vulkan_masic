#include "animation_menu_ui.h"

#include "../../application.h"
#include "../base_engine_logic.h"
#include "../views/human_view.h"

AnimationMenuUI::AnimationMenuUI() {}

AnimationMenuUI::~AnimationMenuUI() {}

bool AnimationMenuUI::VOnRestore() {
	return true;
}

bool AnimationMenuUI::VOnRender(const GameTimerDelta& delta) {
    using namespace std::literals;

    Application& app = Application::Get();
    std::shared_ptr<BaseEngineLogic> game_logic = app.GetGameLogic();
    std::shared_ptr<ActorAnimationPlayer> animation_player = game_logic->GetAnimationPlayer();
    if(!animation_player) return true;

    if (ImGui::Begin("Player Menu")) {
		float total_animation_time = animation_player->GetTotalAnimationTime();
		float current_time = animation_player->GetCurrentAnimationTime();

		if (ImGui::SliderFloat("Time", ((float*)&current_time), 0.0f, total_animation_time, "%.4f")) {
			animation_player->SetDuration(current_time);
		}

		if (ImGui::Button("Play")) {
			animation_player->Play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) {
			animation_player->Pause();
		}

		ActorAnimationPlayer::AnimMap& anim_map = animation_player->GetAnimMap();
		for (auto& [actor_ptr, anim] : anim_map) {
			unsigned int act_id = actor_ptr->GetId();
			std::string act_id_str = std::to_string(act_id);
			std::string actor_name = actor_ptr->GetName();
			std::string chanel_name = "Actor "s + act_id_str + " "s + actor_name + " channel";
			if (ImGui::CollapsingHeader(chanel_name.c_str())) {
				int ct = 0u;
				if (ImGui::TreeNode("Translation channels")) {
					for (ActorAnimationPlayer::KeyframeTranslation& tkf : anim.TranslationKeyframes) {
						std::string tkf_name = "Trc "s + std::to_string(ct);
						if (ImGui::SliderFloat("T Time Point", ((float*)&tkf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
						if (ImGui::SliderFloat3(tkf_name.c_str(), ((float*)&tkf.Translation), -2.0f, 2.0f)) {}
						++ct;
					}
					ImGui::TreePop();
				}
				ct = 0u;
				if (ImGui::TreeNode("Rotation channels")) {
					for (ActorAnimationPlayer::KeyframeRotation& rkf : anim.RotationKeyframes) {
						std::string rkf_name = "YPRc "s + std::to_string(ct);
						if (ImGui::SliderFloat("R Time Point", ((float*)&rkf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
						glm::vec3 pyr = glm::eulerAngles(rkf.RotationQuat);
						pyr.x = DirectX::XMConvertToDegrees(pyr.x);
						pyr.y = DirectX::XMConvertToDegrees(pyr.y);
						pyr.z = DirectX::XMConvertToDegrees(pyr.z);
						if (ImGui::SliderFloat3(rkf_name.c_str(), ((float*)&pyr), -180.0f, 180.0f)) {
							pyr.x = DirectX::XMConvertToRadians(pyr.x);
							pyr.y = DirectX::XMConvertToRadians(pyr.y);
							pyr.z = DirectX::XMConvertToRadians(pyr.z);
							rkf.RotationQuat = glm::eulerAngleXYZ(pyr.x, pyr.y, pyr.z);
						}
						++ct;
					}
					ImGui::TreePop();
				}
				ct = 0u;
				if (ImGui::TreeNode("Scale channels")) {
					for (ActorAnimationPlayer::KeyframeScale& skf : anim.ScaleKeyframes) {
						std::string skf_name = "Scc "s + std::to_string(ct);
						if (ImGui::SliderFloat("S Time Point", ((float*)&skf.TimePos), 0.0f, total_animation_time, "%.4f")) {}
						if (ImGui::SliderFloat3(skf_name.c_str(), ((float*)&skf.Scale), 0.0001f, 2.0f)) {}
						++ct;
					}
					ImGui::TreePop();
				}
			}
		}
	}

    ImGui::End();

    return true;
}

void AnimationMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int AnimationMenuUI::VGetZOrder() const {
	return 1;
}

void AnimationMenuUI::VSetZOrder(int const zOrder) {}