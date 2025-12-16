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

		glm::vec3 up = tc->GetLookUp();
		glm::quat q1 = glm::rotate(orientation, glm::radians(dx) * resistance, up);

		glm::vec3 right = tc->GetLookRight();
		glm::quat q2 = glm::rotate(q1, glm::radians(dy) * resistance, right);

		tc->SetTransform(translation, q2, scale);
		//tc->SetTransform(translation, q1, scale);
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

	if(std::shared_ptr<TransformComponent> tc = gather_transform()) {
		float velocity = 0.1f;

		// glm::vec3 scale;
		// glm::quat orientation;
		// glm::vec3 translation;
		// tc->Decompose(translation, orientation, scale);

		// glm::vec3 up = tc->GetLookUp();
		// glm::quat q1 = glm::rotate(orientation, glm::radians(dx) * resistance, up);

		// glm::vec3 right = tc->GetLookRight();
		// glm::quat q2 = glm::rotate(q1, glm::radians(dy) * resistance, right);

		// tc->SetTransform(translation, q2, scale);
		//tc->SetTransform(translation, q1, scale);
	}
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