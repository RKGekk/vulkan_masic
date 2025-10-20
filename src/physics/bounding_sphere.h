#pragma once

#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL
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

struct BoundingBox;
struct BoundingOrientedBox;
struct BoundingFrustum;

struct BoundingSphere {
    glm::vec3 Center;           // Center of the sphere.
    float Radius;               // Radius of the sphere.

    // Creators
    BoundingSphere() noexcept : Center(0.0f, 0.0f, 0.0f), Radius(1.0f) {}

    BoundingSphere(const BoundingSphere&) = default;
    BoundingSphere& operator=(const BoundingSphere&) = default;

    BoundingSphere(BoundingSphere&&) = default;
    BoundingSphere& operator=(BoundingSphere&&) = default;

    BoundingSphere(const glm::vec3& center, float radius) noexcept : Center(center), Radius(radius) {}

    // Methods
    void Transform(BoundingSphere& Out, const glm::mat4x4& M) const noexcept;
    void Transform(BoundingSphere& Out, float Scale, const glm::quat& Rotation, const glm::vec3& Translation) const noexcept;
    // Transform the sphere

    ContainmentType Contains(const glm::vec3& Point) const noexcept;
    ContainmentType Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept;
    ContainmentType Contains(const BoundingSphere& sh) const noexcept;
    ContainmentType Contains(const BoundingBox& box) const noexcept;
    ContainmentType Contains(const BoundingOrientedBox& box) const noexcept;
    ContainmentType Contains(const BoundingFrustum& fr) const noexcept;

    bool Intersects(const BoundingSphere& sh) const noexcept;
    bool Intersects(const BoundingBox& box) const noexcept;
    bool Intersects(const BoundingOrientedBox& box) const noexcept;
    bool Intersects(const BoundingFrustum& fr) const noexcept;

    bool Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept;
    // Triangle-sphere test

    PlaneIntersectionType Intersects(const glm::vec4& Plane) const noexcept;
    // Plane-sphere test

    bool Intersects(const glm::vec3& Origin, const glm::vec3& Direction, float& Dist) const noexcept;
    // Ray-sphere test

    ContainmentType ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept;
    // Test sphere against six planes (see BoundingFrustum::GetPlanes)

    // Static methods
    static void CreateMerged(BoundingSphere& Out, const BoundingSphere& S1, const BoundingSphere& S2) noexcept;

    static void CreateFromBoundingBox(BoundingSphere& Out, const BoundingBox& box) noexcept;
    static void CreateFromBoundingBox(BoundingSphere& Out, const BoundingOrientedBox& box) noexcept;
    static void CreateFromPoints(BoundingSphere& Out, size_t Count, glm::vec3* pPoints, size_t Stride) noexcept;
    static void CreateFromFrustum(BoundingSphere& Out, const BoundingFrustum& fr) noexcept;
};