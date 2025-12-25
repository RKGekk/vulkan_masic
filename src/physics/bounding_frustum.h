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

struct BoundingSphere;
struct BoundingBox;
struct BoundingOrientedBox;

                                                                                                    
//               ---------------------------------------------------------------------=
//                =.................................|................................-
//                 =................................|...............................=
//                  =...............................|..............................=
//                   +..............................|.............................=
//                    =.............................+............................-
//                     -...........................FAR..........................:
//                      =......................................................=
//                       -..........:%#:........................:%#:..........=
//                        =.....:=+.................................:*-:.....+
//                          =:==:.......................................-=-.=
//                   LEFT    :.............................................:  RIGHT
//                            :...........................................- 
//                             *........................................:+                             
//                              =.................NEAR..................-                              
//                               =..................^..................=                               
//                                =.................|:................=                                
//                                 -................|................=                                 
//                                  :...............|...............-                                  
//                                  .:..............|...............:                                   
//                                   ::.............|..............-                                    
//                                    =---------------------------:                                    
//                                     -                         :                                     
//                                      :                       :                                      
//                                       -                     :
//                                        =                   =
//                                         -                 =
//                                          :               -
//                                           .     +Z      :
//                                            .     ^     .                                            
//                                             .    |    .                                              
//                                              .   |   .                                               
//                                               .  |  .                                                
//                                                - | .
//                                                 -|:
//                                              ----|-----------------------> +X
//                                                  |

//-----------------------------------------------------------------------------
// frustum takes on a canonical form centered at the origin looking down
// the positive z-axis. Here, the near and far planes are trivially specified
// by their distances along the z-axis, the left and right planes are symmetric
// and pass through the origin, and the top and bottom planes are also
// symmetric and pass through the origin.
//-----------------------------------------------------------------------------
struct BoundingFrustum {
    static constexpr size_t CORNER_COUNT = 8;

    glm::vec3 Origin;            // Origin of the frustum (and projection).
    glm::quat Orientation;       // Quaternion representing rotation.

    float RightSlope;           // Positive X (X/Z)
    float LeftSlope;            // Negative X
    float TopSlope;             // Positive Y (Y/Z)
    float BottomSlope;          // Negative Y
    float Near;                 // Z of the near plane.
    float Far;                  // Z of the far plane.

    // Creators
    BoundingFrustum() noexcept : Origin(0.0f, 0.0f, 0.0f),
                                 Orientation(1.0f, 0.0f, 0.0f, 0.0f),
                                 RightSlope(1.0f),
                                 LeftSlope(-1.0f),
                                 TopSlope(1.0f),
                                 BottomSlope(-1.0f),
                                 Near(0.0f),
                                 Far(1.0f) {}

    BoundingFrustum(const BoundingFrustum&) = default;
    BoundingFrustum& operator=(const BoundingFrustum&) = default;

    BoundingFrustum(BoundingFrustum&&) = default;
    BoundingFrustum& operator=(BoundingFrustum&&) = default;

    BoundingFrustum(const glm::vec3& origin,
                    const glm::vec4& orientation,
                    float rightSlope,
                    float leftSlope,
                    float topSlope,
                    float bottomSlope,
                    float nearPlane,
                    float farPlane) noexcept : Origin(origin),
                                               Orientation(orientation),
                                               RightSlope(rightSlope),
                                               LeftSlope(leftSlope),
                                               TopSlope(topSlope),
                                               BottomSlope(bottomSlope),
                                               Near(nearPlane),
                                               Far(farPlane) {}

    BoundingFrustum(const glm::mat4x4& Projection, bool rhcoords = true, bool zforward = false) noexcept;

    // Methods
    void Transform(BoundingFrustum& Out, const glm::mat4x4& M) const noexcept;
    void Transform(BoundingFrustum& Out, float Scale, glm::quat Rotation, glm::vec3 Translation) const noexcept;

    // Gets the 8 corners of the frustum
    void GetCorners(glm::vec3* Corners) const noexcept;
    
    ContainmentType Contains(const glm::vec3& Point) const noexcept;
    ContainmentType Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept;
    ContainmentType Contains(_In_ const BoundingSphere& sp) const noexcept;
    ContainmentType Contains(_In_ const BoundingBox& box) const noexcept;
    ContainmentType Contains(_In_ const BoundingOrientedBox& box) const noexcept;
    ContainmentType Contains(_In_ const BoundingFrustum& fr) const noexcept;
    // Frustum-Frustum test

    bool Intersects(const BoundingSphere& sh) const noexcept;
    bool Intersects(const BoundingBox& box) const noexcept;
    bool Intersects(const BoundingOrientedBox& box) const noexcept;
    bool Intersects(const BoundingFrustum& fr) const noexcept;

    bool Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept;
    // Triangle-Frustum test

    PlaneIntersectionType Intersects(const glm::vec4& Plane) const noexcept;
    // Plane-Frustum test

    bool Intersects(const glm::vec3& rayOrigin, const glm::vec3& Direction, float& Dist) const noexcept;
    // Ray-Frustum test

    ContainmentType ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept;
    // Test frustum against six planes (see BoundingFrustum::GetPlanes)

    void GetPlanes(glm::vec4* NearPlane, glm::vec4* FarPlane, glm::vec4* RightPlane, glm::vec4* LeftPlane, glm::vec4* TopPlane, glm::vec4* BottomPlane) const noexcept;
    // Create 6 Planes representation of Frustum

    // Static methods
    static void CreateFromMatrix(BoundingFrustum& Out, const glm::mat4x4& Projection, bool rhcoords = true, bool zforward = false) noexcept;
};