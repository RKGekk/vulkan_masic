#pragma once

#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include "glm/gtc/quaternion.hpp"
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "containment_type.h"
#include "plane_intersection_type.h"

glm::vec4 PlaneTransform(const glm::vec4& Plane, const glm::quat& Rotation, const glm::vec3& Translation) noexcept;
glm::vec4 PlaneNormalize(const glm::vec4& P) noexcept;
float PlaneDotCoord(const glm::vec4& P, glm::vec3 V) noexcept;
glm::vec3 PointOnLineSegmentNearestPoint(const glm::vec3& S1, const glm::vec3& S2, const glm::vec3& P) noexcept;
void IntersectSpherePlane(const glm::vec3& Center, float Radius, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept;
void IntersectFrustumPlane(const glm::vec4& Point0, const glm::vec4& Point1, const glm::vec4& Point2, const glm::vec4& Point3, const glm::vec4& Point4, const glm::vec4& Point5, const glm::vec4& Point6, const glm::vec4& Point7, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept;
void IntersectTrianglePlane(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept;
bool PointOnPlaneInsideTriangle(const glm::vec3& P, const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) noexcept;

static const glm::vec4 g_RayEpsilon;

namespace TriangleTests {
    bool Intersects(const glm::vec3& Origin, const glm::vec3& Direction, const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, float& Dist) noexcept;
    // Ray-Triangle

    bool Intersects(const glm::vec3& A0, const glm::vec3& A1, const glm::vec3& A2, const glm::vec3& B0, const glm::vec3& B1, const glm::vec3& B2) noexcept;
    // Triangle-Triangle

    PlaneIntersectionType Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane) noexcept;
    // Plane-Triangle

    ContainmentType ContainedBy(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) noexcept;
    // Test a triangle against six planes at once (see BoundingFrustum::GetPlanes)
}