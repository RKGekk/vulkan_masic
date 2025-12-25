#include "movement_controller.h"

#include "../../actors/transform_component.h"
#include "../../tools/memory_utility.h"

MovementController::MovementController(std::shared_ptr<Actor> object) : m_object(object) {
	m_last_mouse_pos_x = 0;
	m_last_mouse_pos_y = 0;

	memset(m_key, 0x00, sizeof(m_key));
	memset(m_char, 0x00, sizeof(m_char));

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

	if (m_key[ to_underlying(WindowKey::W)] || m_key[to_underlying(WindowKey::S)]) {
		float velocity = 1.0f;

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		glm::vec3 forward = tc->GetLookAt();
		//glm::vec3 forward = tc->GetForward3f();
		if(m_key[to_underlying(WindowKey::S)]) forward *= -1.0f;
		translation += forward * velocity * delta.fGetDeltaSeconds();
		
		tc->SetTransform(translation, orientation, scale);
	}

	if (m_key[to_underlying(WindowKey::A)] || m_key[to_underlying(WindowKey::D)]) {

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		glm::vec3 right = tc->GetLookRight();
		//glm::vec3 right = tc->GetRight3f();
		if(m_key[to_underlying(WindowKey::A)]) right *= -1.0f;
		translation += right * velocity * delta.fGetDeltaSeconds();
		
		tc->SetTransform(translation, orientation, scale);
	}

	if (m_key[to_underlying(WindowKey::Space)] || m_key[to_underlying(WindowKey::LControlKey)]) {

		glm::vec3 scale;
		glm::quat orientation;
		glm::vec3 translation;
		tc->Decompose(translation, orientation, scale);

		glm::vec3 up = tc->GetLookUp();
		//glm::vec3 up = tc->GetUp3f();
		if(m_key[to_underlying(WindowKey::LControlKey)]) up *= -1.0f;
		translation += up * velocity * delta.fGetDeltaSeconds();
		
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

		glm::quat q1 = glm::angleAxis(glm::radians(dx) * resistance, tc->GetUp3f());
		glm::quat q2 = glm::angleAxis(glm::radians(dy) * resistance, tc->GetRight3f());
		glm::quat q3 = glm::normalize(orientation * q1 * q2);

		glm::vec3 up = tc->GetUp3f();
		glm::vec3 forward = q3 * (tc->GetForward3f() * -1.0f);
		glm::vec3 right = glm::normalize(glm::cross(up, forward));
		up = glm::cross(forward, right);
		
		glm::mat4x4 mat(
			glm::vec4(right.x * scale.x, right.y, right.z, 0.0f),
			glm::vec4(up.x, up.y * scale.y, up.z, 0.0f),
			glm::vec4(forward.x, forward.y, forward.z * scale.z, 0.0f),
			glm::vec4(translation.x, translation.y, translation.z, 1.0f)
		);

		tc->SetTransform(mat);
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

bool MovementController::VOnKeyDown(WindowKey key, unsigned char c) {
	m_key[to_underlying(key)] = true;
	m_char[c] = true;

	return true;
}

bool MovementController::VOnKeyUp(WindowKey key, unsigned char c) {
	m_key[to_underlying(key)] = false;
	m_char[c] = false;

	return true;
}

std::shared_ptr<TransformComponent> MovementController::gather_transform() {
	if (!m_object) return nullptr;

	std::shared_ptr<TransformComponent> tc = m_object->GetComponent<TransformComponent>().lock();
	if(!tc) return nullptr;

	return tc;
}