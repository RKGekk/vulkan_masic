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
                        for(const std::shared_ptr<AnimationNode>& anim_node : clip_data) {
                            printHierarchyTreeView(anim_node);
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