#include "test_menu_ui.h"

#include <imgui.h>

TestMenuUI::TestMenuUI() {
}

TestMenuUI::~TestMenuUI() {}

bool TestMenuUI::VOnRestore() {
	return true;
}

bool TestMenuUI::VOnRender(const GameTimerDelta& delta) {
	using namespace std;
	if (!m_is_visible) return true;

	ImGui::ShowDemoWindow();

	return true;
}

void TestMenuUI::VOnUpdate(const GameTimerDelta& delta) {}

int TestMenuUI::VGetZOrder() const {
	return 999;
}

void TestMenuUI::VSetZOrder(int const zOrder) {}