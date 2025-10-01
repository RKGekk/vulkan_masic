#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext.hpp>

#include <string>

class SceneNodeProperties {
public:
	enum class DirtyFlags {
		DF_None      = 0,
		DF_Light     = (1 << 0),
		DF_Mesh      = (1 << 1),
		DF_Transform = (1 << 2),
		DF_Camera    = (1 << 3),
		DF_All = DF_Light | DF_Mesh | DF_Transform | DF_Camera
	};

	SceneNodeProperties() {};

	glm::mat4x4 ToParent() const {};
	glm::mat4x4 ToParentT() const {};

	glm::mat4x4 ToRoot() const {};
	glm::mat4x4 ToRootT() const {};

	glm::vec4 ToParentTranslation4() const {};
	glm::vec3 ToParentTranslation3() const{} ;

	glm::vec4 ToRootTranslation4() const {};
	glm::vec3 ToRootTranslation3() const{} ;

	glm::vec3 ToParentDirection() const {};
	glm::vec3 ToParentUp() const {};

	glm::vec3 ToRootDirection() const {};
	glm::vec3 ToRootUp() const {};

	glm::mat4x4 FromParent() const {};
	glm::mat4x4 FromParentT() const {};

	glm::mat4x4 FromRoot() const {};
	glm::mat4x4 FromRootT() const {};

	const char* NameCstr() const {};
	const std::string& Name() const {};

	uint32_t GetDirtyFlags() const {};
	uint32_t GetGroupID() const {};
};