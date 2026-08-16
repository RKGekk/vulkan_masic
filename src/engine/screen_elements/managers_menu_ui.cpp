#include "managers_menu_ui.h"

#include "../../application.h"
#include "imgui_tools.h"

ManagersMenuUI::ManagersMenuUI() {
    //m_animation_manager = Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->getAnimationManager();
}

ManagersMenuUI::~ManagersMenuUI() {

}

bool ManagersMenuUI::VOnRestore() {
	return true;
}

bool ManagersMenuUI::VOnRender(const GameTimerDelta& delta, uint32_t image_index) {
    using namespace std;
    if (!m_is_visible) return true;

	if (ImGui::Begin("Managers Menu")) {
        if(const std::shared_ptr<AnimationManager>& animation_manager = Application::Get().GetGameLogic()->GetHumanView()->VGetScene()->getAnimationManager()) {
            if (ImGui::CollapsingHeader("Animation Manager")) {
                for(const auto&[clip_name, clip_data] : animation_manager->GetClipMap()) {
                    if (ImGui::TreeNode(clip_name.c_str())) {

						if (ImGui::TreeNode("ControlledNodes")) {
							for(const std::shared_ptr<AnimationNode>& anim_node : clip_data) {
                            	printSceneNode(anim_node);
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
	}
	ImGui::End();

	return true;
}

void ManagersMenuUI::VOnUpdate(const GameTimerDelta& delta, uint32_t image_index) {}

int ManagersMenuUI::VGetZOrder() const {
	return 1;
}

void ManagersMenuUI::VSetZOrder(int const zOrder) {}