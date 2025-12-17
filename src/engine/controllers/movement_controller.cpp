#include "movement_controller.h"

#include "../../actors/transform_component.h"

MovementController::MovementController(std::shared_ptr<Actor> object) : m_object(object) {
	std::shared_ptr<TransformComponent> tc;
	if (object) {
		tc = object->GetComponent<TransformComponent>().lock();
	}

	m_last_mouse_pos_x = 0;
	m_last_mouse_pos_y = 0;

	memset(m_bKey, 0x00, sizeof(m_bKey));

	m_mouse_LButton_down = false;
	m_mouse_RButton_down = false;
}

void MovementController::SetObject(std::shared_ptr<Actor> new_object) {
	m_object = new_object;
}

void MovementController::OnUpdate(const GameTimerDelta& delta) {
	std::shared_ptr<TransformComponent> tc = gather_transform();
	if(!tc) return;

	float velocity = 1.0f;

	if (m_bKey['W'] || m_bKey['S']) {
		float velocity = 1.0f;

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		//glm::vec3 forward = tc->GetLookAt();
		glm::vec3 forward = tc->GetForward3f();
		if(m_bKey['S']) forward *= -1.0f;
		translation += forward * velocity * delta.fGetDeltaSeconds();
		
		tc->SetTransform(translation, orientation, scale);
	}

	if (m_bKey['A'] || m_bKey['D']) {

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		//glm::vec3 right = tc->GetLookRight();
		glm::vec3 right = tc->GetRight3f();
		if(m_bKey['A']) right *= -1.0f;
		translation += right * velocity * delta.fGetDeltaSeconds();
		
		tc->SetTransform(translation, orientation, scale);
	}
}

bool MovementController::VOnPointerMove(int x, int y, const int radius) {
	bool has_changes = m_last_mouse_pos_x != x || m_last_mouse_pos_y != y;
	if(has_changes && m_mouse_RButton_down) if(std::shared_ptr<TransformComponent> tc = gather_transform()) {
		float dx = (float)(x - m_last_mouse_pos_x);
		float dy = (float)(y - m_last_mouse_pos_y);
		float resistance = 0.1f;

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		glm::quat q(1.0f, 0.0f, 0.0f, 0.0f);
		glm::quat q1 = glm::rotate(q, glm::radians(dx) * resistance, tc->GetUp3f());
		glm::quat q2 = glm::rotate(q, glm::radians(dy) * resistance, tc->GetRight3f());
		glm::quat q3 = orientation * q1 * q2;

		glm::vec3 ypr = glm::eulerAngles(q3);
		ypr = glm::vec3(ypr.y, ypr.x, 0.0f);

		tc->SetTransform(translation, ypr, scale);
	}

	m_last_mouse_pos_x = x;
	m_last_mouse_pos_y = y;

	return true;
}

bool MovementController::VOnPointerButtonDown(int x, int y, const int radius, MouseButtonSide btn) {
	if (btn == MouseButtonSide::Left) {
		m_mouse_LButton_down = true;
		m_last_mouse_pos_x = x;
		m_last_mouse_pos_y = y;
		return true;
	}

	if (btn == MouseButtonSide::Right) {
		m_mouse_RButton_down = true;
		m_last_mouse_pos_x = x;
		m_last_mouse_pos_y = y;
		return true;
	}

	return false;
}

bool MovementController::VOnPointerButtonUp(int x, int y, const int radius, MouseButtonSide btn) {
	if (btn == MouseButtonSide::Left) {
		m_mouse_LButton_down = false;
		return true;
	}

	if (btn == MouseButtonSide::Right) {
		m_mouse_RButton_down = false;
		return true;
	}

	return false;
}

bool MovementController::VOnKeyDown(unsigned char c) {
	m_bKey[c] = true;

	return true;
}

bool MovementController::VOnKeyUp(unsigned char c) {
	m_bKey[c] = false;
	return true;
}

std::shared_ptr<TransformComponent> MovementController::gather_transform() {
	if (!m_object) return nullptr;

	std::shared_ptr<TransformComponent> tc = m_object->GetComponent<TransformComponent>().lock();
	if(!tc) return nullptr;

	return tc;
}