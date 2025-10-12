#pragma once

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

struct BoundingSphere;
struct BoundingOrientedBox;
struct BoundingFrustum;

static const glm::vec4 g_BoxOffset[8];
static const glm::vec4 g_FltMin;
static const glm::vec4 g_FltMax;

// Axis-aligned bounding box
struct BoundingBox {
    static constexpr size_t CORNER_COUNT = 8;

    glm::vec3 Center;            // Center of the box.
    glm::vec3 Extents;           // Distance from the center to each side.

    // Creators
    BoundingBox() noexcept : Center(0.0f, 0.0f, 0.0f), Extents(1.0f, 1.0f, 1.0f) {}

    BoundingBox(const BoundingBox&) = default;
    BoundingBox& operator=(const BoundingBox&) = default;

    BoundingBox(BoundingBox&&) = default;
    BoundingBox& operator=(BoundingBox&&) = default;

    BoundingBox(const glm::vec3& center, const glm::vec3& extents) noexcept : Center(center), Extents(extents) {}

    // Methods
    void Transform(BoundingBox& Out, const glm::mat4x4& M) const noexcept;
    void Transform(BoundingBox& Out, float Scale, const glm::quat& Rotation, const glm::vec3& Translation) const noexcept;

    void GetCorners(glm::vec3* Corners) const noexcept;
    // Gets the 8 corners of the box

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
    // Triangle-Box test

    PlaneIntersectionType Intersects(const glm::vec4& Plane) const noexcept;
    // Plane-box test

    bool Intersects(const glm::vec3& Origin, const glm::vec3& Direction, float& Dist) const noexcept;
    // Ray-Box test

    ContainmentType ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept;
    // Test box against six planes (see BoundingFrustum::GetPlanes)

    // Static methods
    static void CreateMerged(BoundingBox& Out, const BoundingBox& b1, const BoundingBox& b2) noexcept;
    static void CreateFromSphere(BoundingBox& Out, const BoundingSphere& sh) noexcept;
    static void CreateFromPoints(BoundingBox& Out, const glm::vec3& pt1, const glm::vec3& pt2) noexcept;
    static void CreateFromPoints(BoundingBox& Out, size_t Count, glm::vec3* pPoints, size_t Stride) noexcept;
};