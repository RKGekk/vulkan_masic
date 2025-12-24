#include "bounding_frustum.h"
#include "triangle_tests.h"
#include "bounding_sphere.h"
#include "bounding_box.h"
#include "bounding_oriented_box.h"

BoundingFrustum::BoundingFrustum(const glm::mat4x4& Projection, bool rhcoords) noexcept {
    CreateFromMatrix(*this, Projection, rhcoords);
}


// Transform a frustum by an angle preserving transform.
void BoundingFrustum::Transform(BoundingFrustum& Out, const glm::mat4x4& M) const noexcept {
    // Composite the frustum rotation and the transform rotation
    glm::mat4x4 nM;
    nM[0] = glm::normalize(M[0]);
    nM[1] = glm::normalize(M[1]);
    nM[2] = glm::normalize(M[2]);
    nM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::quat Rotation(nM);

    glm::quat vOrientation = Orientation * Rotation;

    // Transform the center.
    glm::vec4 vOrigin = M * glm::vec4(Origin, 1.0f);

    // Store the frustum.
    Out.Origin = vOrigin;
    Out.Orientation = vOrientation;

    // Scale the near and far distances (the slopes remain the same).
    float dX = glm::dot(M[0], M[0]);
    float dY = glm::dot(M[1], M[1]);
    float dZ = glm::dot(M[2], M[2]);

    float d = glm::max(dX, glm::max(dY, dZ));
    float Scale = std::sqrtf(d);

    Out.Near = Near * Scale;
    Out.Far = Far * Scale;

    // Copy the slopes.
    Out.RightSlope = RightSlope;
    Out.LeftSlope = LeftSlope;
    Out.TopSlope = TopSlope;
    Out.BottomSlope = BottomSlope;
}

void BoundingFrustum::Transform(BoundingFrustum& Out, float Scale, glm::quat Rotation, glm::vec3 Translation) const noexcept {
    // Composite the frustum rotation and the transform rotation.
    glm::quat vOrientation = Orientation * Rotation;

    // Transform the origin.
    glm::vec4 vOrigin = (glm::vec4(Origin * Scale, 1.0f) * Rotation) + glm::vec4(Translation, 0.0f);

    // Store the frustum.
    Out.Origin = vOrigin;
    Out.Orientation = vOrientation;

    // Scale the near and far distances (the slopes remain the same).
    Out.Near = Near * Scale;
    Out.Far = Far * Scale;

    // Copy the slopes.
    Out.RightSlope = RightSlope;
    Out.LeftSlope = LeftSlope;
    Out.TopSlope = TopSlope;
    Out.BottomSlope = BottomSlope;
}


// Get the corner points of the frustum
void BoundingFrustum::GetCorners(glm::vec3* Corners) const noexcept {
    // Build the corners of the frustum.
    glm::vec4 vRightTop(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 vRightBottom(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vLeftTop(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 vLeftBottom(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vNear(Near);
    glm::vec4 vFar(Far);

    // Returns 8 corners position of bounding frustum.
    //     Near    Far
    //    0----1  4----5
    //    |    |  |    |
    //    |    |  |    |
    //    3----2  7----6

    glm::vec4 vCorners[CORNER_COUNT];
    vCorners[0] = vLeftTop * vNear;
    vCorners[1] = vRightTop * vNear;
    vCorners[2] = vRightBottom * vNear;
    vCorners[3] = vLeftBottom * vNear;
    vCorners[4] = vLeftTop * vFar;
    vCorners[5] = vRightTop * vFar;
    vCorners[6] = vRightBottom * vFar;
    vCorners[7] = vLeftBottom * vFar;

    for (size_t i = 0; i < CORNER_COUNT; ++i) {
        glm::vec4 C = (vCorners[i] * Orientation) + glm::vec4(Origin, 0.0f);
        Corners[i] = C;
    }
}


// Point in frustum test.
ContainmentType BoundingFrustum::Contains(const glm::vec3& Point) const noexcept {
    // Build frustum planes.
    glm::vec4 Planes[6];
    Planes[0] = glm::vec4(0.0f, 0.0f, -1.0f, Near);
    Planes[1] = glm::vec4(0.0f, 0.0f, 1.0f, -Far);
    Planes[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    Planes[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    Planes[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    Planes[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    // Transform point into local space of frustum.
    glm::vec4 TPoint = glm::vec4(Point - Origin, 1.0f) * glm::inverse(Orientation);

    // Set w to one.
    TPoint.w = 1.0f;

    bool Outside = false;

    // Test point against each plane of the frustum.
    for (size_t i = 0u; i < 6u; ++i) {
        float Dot = glm::dot(TPoint, Planes[i]);
        Outside |= Dot > 0.0f;
    }

    return Outside ? CONTAINS : DISJOINT;
}

// Triangle vs frustum test.
ContainmentType BoundingFrustum::Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    // Create 6 planes (do it inline to encourage use of registers)
    glm::vec4 NearPlane(0.0f, 0.0f, -1.0f, Near);
    NearPlane = PlaneTransform(NearPlane, Orientation, Origin);
    NearPlane = PlaneNormalize(NearPlane);

    glm::vec4 FarPlane(0.0f, 0.0f, 1.0f, -Far);
    FarPlane = PlaneTransform(FarPlane, Orientation, Origin);
    FarPlane = PlaneNormalize(FarPlane);

    glm::vec4 RightPlane(1.0f, 0.0f, -RightSlope, 0.0f);
    RightPlane = PlaneTransform(RightPlane, Orientation, Origin);
    RightPlane = PlaneNormalize(RightPlane);

    glm::vec4 LeftPlane(-1.0f, 0.0f, LeftSlope, 0.0f);
    LeftPlane = PlaneTransform(LeftPlane, Orientation, Origin);
    LeftPlane = PlaneNormalize(LeftPlane);

    glm::vec4 TopPlane(0.0f, 1.0f, -TopSlope, 0.0f);
    TopPlane = PlaneTransform(TopPlane, Orientation, Origin);
    TopPlane = PlaneNormalize(TopPlane);

    glm::vec4 BottomPlane(0.0f, -1.0f, BottomSlope, 0.0f);
    BottomPlane = PlaneTransform(BottomPlane, Orientation, Origin);
    BottomPlane = PlaneNormalize(BottomPlane);

    return TriangleTests::ContainedBy(V0, V1, V2, NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
}


//-----------------------------------------------------------------------------
ContainmentType BoundingFrustum::Contains(const BoundingSphere& sh) const noexcept {
    // Create 6 planes (do it inline to encourage use of registers)
    glm::vec4 NearPlane (0.0f, 0.0f, -1.0f, Near);
    NearPlane = PlaneTransform(NearPlane, Orientation, Origin);
    NearPlane = PlaneNormalize(NearPlane);

    glm::vec4 FarPlane(0.0f, 0.0f, 1.0f, -Far);
    FarPlane = PlaneTransform(FarPlane, Orientation, Origin);
    FarPlane = PlaneNormalize(FarPlane);

    glm::vec4 RightPlane(1.0f, 0.0f, -RightSlope, 0.0f);
    RightPlane = PlaneTransform(RightPlane, Orientation, Origin);
    RightPlane = PlaneNormalize(RightPlane);

    glm::vec4 LeftPlane(-1.0f, 0.0f, LeftSlope, 0.0f);
    LeftPlane = PlaneTransform(LeftPlane, Orientation, Origin);
    LeftPlane = PlaneNormalize(LeftPlane);

    glm::vec4 TopPlane(0.0f, 1.0f, -TopSlope, 0.0f);
    TopPlane = PlaneTransform(TopPlane, Orientation, Origin);
    TopPlane = PlaneNormalize(TopPlane);

    glm::vec4 BottomPlane(0.0f, -1.0f, BottomSlope, 0.0f);
    BottomPlane = PlaneTransform(BottomPlane, Orientation, Origin);
    BottomPlane = PlaneNormalize(BottomPlane);

    return sh.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
}

//-----------------------------------------------------------------------------
ContainmentType BoundingFrustum::Contains(const BoundingBox& box) const noexcept {
    // Create 6 planes (do it inline to encourage use of registers)
    glm::vec4 NearPlane(0.0f, 0.0f, -1.0f, Near);
    NearPlane = PlaneTransform(NearPlane, Orientation, Origin);
    NearPlane = PlaneNormalize(NearPlane);

    glm::vec4 FarPlane(0.0f, 0.0f, 1.0f, -Far);
    FarPlane = PlaneTransform(FarPlane, Orientation, Origin);
    FarPlane = PlaneNormalize(FarPlane);

    glm::vec4 RightPlane(1.0f, 0.0f, -RightSlope, 0.0f);
    RightPlane = PlaneTransform(RightPlane, Orientation, Origin);
    RightPlane = PlaneNormalize(RightPlane);

    glm::vec4 LeftPlane(-1.0f, 0.0f, LeftSlope, 0.0f);
    LeftPlane = PlaneTransform(LeftPlane, Orientation, Origin);
    LeftPlane = PlaneNormalize(LeftPlane);

    glm::vec4 TopPlane(0.0f, 1.0f, -TopSlope, 0.0f);
    TopPlane = PlaneTransform(TopPlane, Orientation, Origin);
    TopPlane = PlaneNormalize(TopPlane);

    glm::vec4 BottomPlane(0.0f, -1.0f, BottomSlope, 0.0f);
    BottomPlane = PlaneTransform(BottomPlane, Orientation, Origin);
    BottomPlane = PlaneNormalize(BottomPlane);

    return box.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
}

//-----------------------------------------------------------------------------
ContainmentType BoundingFrustum::Contains(const BoundingOrientedBox& box) const noexcept {
    // Create 6 planes (do it inline to encourage use of registers)
    glm::vec4 NearPlane(0.0f, 0.0f, -1.0f, Near);
    NearPlane = PlaneTransform(NearPlane, Orientation, Origin);
    NearPlane = PlaneNormalize(NearPlane);

    glm::vec4 FarPlane(0.0f, 0.0f, 1.0f, -Far);
    FarPlane = PlaneTransform(FarPlane, Orientation, Origin);
    FarPlane = PlaneNormalize(FarPlane);

    glm::vec4 RightPlane(1.0f, 0.0f, -RightSlope, 0.0f);
    RightPlane = PlaneTransform(RightPlane, Orientation, Origin);
    RightPlane = PlaneNormalize(RightPlane);

    glm::vec4 LeftPlane(-1.0f, 0.0f, LeftSlope, 0.0f);
    LeftPlane = PlaneTransform(LeftPlane, Orientation, Origin);
    LeftPlane = PlaneNormalize(LeftPlane);

    glm::vec4 TopPlane(0.0f, 1.0f, -TopSlope, 0.0f);
    TopPlane = PlaneTransform(TopPlane, Orientation, Origin);
    TopPlane = PlaneNormalize(TopPlane);

    glm::vec4 BottomPlane(0.0f, -1.0f, BottomSlope, 0.0f);
    BottomPlane = PlaneTransform(BottomPlane, Orientation, Origin);
    BottomPlane = PlaneNormalize(BottomPlane);

    return box.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
}


//-----------------------------------------------------------------------------
ContainmentType BoundingFrustum::Contains(const BoundingFrustum& fr) const noexcept {
    // Create 6 planes (do it inline to encourage use of registers)
    glm::vec4 NearPlane(0.0f, 0.0f, -1.0f, Near);
    NearPlane = PlaneTransform(NearPlane, Orientation, Origin);
    NearPlane = PlaneNormalize(NearPlane);

    glm::vec4 FarPlane(0.0f, 0.0f, 1.0f, -Far);
    FarPlane = PlaneTransform(FarPlane, Orientation, Origin);
    FarPlane = PlaneNormalize(FarPlane);

    glm::vec4 RightPlane(1.0f, 0.0f, -RightSlope, 0.0f);
    RightPlane = PlaneTransform(RightPlane, Orientation, Origin);
    RightPlane = PlaneNormalize(RightPlane);

    glm::vec4 LeftPlane(-1.0f, 0.0f, LeftSlope, 0.0f);
    LeftPlane = PlaneTransform(LeftPlane, Orientation, Origin);
    LeftPlane = PlaneNormalize(LeftPlane);

    glm::vec4 TopPlane(0.0f, 1.0f, -TopSlope, 0.0f);
    TopPlane = PlaneTransform(TopPlane, Orientation, Origin);
    TopPlane = PlaneNormalize(TopPlane);

    glm::vec4 BottomPlane(0.0f, -1.0f, BottomSlope, 0.0f);
    BottomPlane = PlaneTransform(BottomPlane, Orientation, Origin);
    BottomPlane = PlaneNormalize(BottomPlane);

    return fr.ContainedBy(NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
}


//-----------------------------------------------------------------------------
// Exact sphere vs frustum test.  The algorithm first checks the sphere against
// the planes of the frustum, then if the plane checks were indeterminate finds
// the nearest feature (plane, line, point) on the frustum to the center of the
// sphere and compares the distance to the nearest feature to the radius of the
// sphere
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const BoundingSphere& sh) const noexcept {
    // Build the frustum planes.
    glm::vec4 Planes[6];
    Planes[0] = glm::vec4(0.0f, 0.0f, -1.0f, Near);
    Planes[1] = glm::vec4(0.0f, 0.0f, 1.0f, -Far);
    Planes[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    Planes[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    Planes[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    Planes[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    // Normalize the planes so we can compare to the sphere radius.
    Planes[2] = glm::normalize(Planes[2]);
    Planes[3] = glm::normalize(Planes[3]);
    Planes[4] = glm::normalize(Planes[4]);
    Planes[5] = glm::normalize(Planes[5]);

    // Load the sphere.
    glm::vec4 vCenter = glm::vec4(sh.Center, 1.0f);
    float vRadius = sh.Radius;

    // Transform the center of the sphere into the local space of frustum.
    vCenter = (vCenter - glm::vec4(Origin, 1.0f)) * glm::inverse(Orientation);

    // Set w of the center to one so we can dot4 with the plane.
    vCenter.w = 1.0f;

    // Check against each plane of the frustum.
    bool Outside = false;
    bool InsideAll = true;
    bool CenterInsideAll = true;

    float Dist[6];

    for (size_t i = 0; i < 6; ++i) {
        Dist[i] = glm::dot(vCenter, Planes[i]);

        // Outside the plane?
        Outside |= Dist[i] > vRadius;

        // Fully inside the plane?
        InsideAll &= Dist[i] <= -vRadius;

        // Check if the center is inside the plane.
        CenterInsideAll &= Dist[i] <= 0.0f;
    }

    // If the sphere is outside any of the planes it is outside.
    if (Outside) {
        return false;
    }

    // If the sphere is inside all planes it is fully inside.
    if (InsideAll) {
        return true;
    }

    // If the center of the sphere is inside all planes and the sphere intersects
    // one or more planes then it must intersect.
    if (CenterInsideAll) {
        return true;
    }

    // The sphere may be outside the frustum or intersecting the frustum.
    // Find the nearest feature (face, edge, or corner) on the frustum
    // to the sphere.

    // The faces adjacent to each face are:
    static const size_t adjacent_faces[6][4] = {
        { 2, 3, 4, 5 },    // 0
        { 2, 3, 4, 5 },    // 1
        { 0, 1, 4, 5 },    // 2
        { 0, 1, 4, 5 },    // 3
        { 0, 1, 2, 3 },    // 4
        { 0, 1, 2, 3 }     // 5
    };

    bool Intersects = false;

    // Check to see if the nearest feature is one of the planes.
    for (size_t i = 0u; i < 6u; ++i) {
        // Find the nearest point on the plane to the center of the sphere.
        glm::vec4 Point = vCenter - (Planes[i] * Dist[i]);

        // Set w of the point to one.
        Point.w = 1.0f;

        // If the point is inside the face (inside the adjacent planes) then
        // this plane is the nearest feature.
        bool InsideFace = true;

        for (size_t j = 0u; j < 4u; ++j) {
            size_t plane_index = adjacent_faces[i][j];

            InsideFace &= glm::dot(Point, Planes[plane_index]) <= 0.0f;
        }

        // Since we have already checked distance from the plane we know that the
        // sphere must intersect if this plane is the nearest feature.
        Intersects |= (Dist[i] > 0.0f) && InsideFace;
    }

    if (Intersects) {
        return true;
    }

    // Build the corners of the frustum.
    glm::vec3 vRightTop(RightSlope, TopSlope, 1.0f);
    glm::vec3 vRightBottom(RightSlope, BottomSlope, 1.0f);
    glm::vec3 vLeftTop(LeftSlope, TopSlope, 1.0f);
    glm::vec3 vLeftBottom(LeftSlope, BottomSlope, 1.0f);
    glm::vec3 vNear(Near);
    glm::vec3 vFar(Far);

    glm::vec3 Corners[CORNER_COUNT];
    Corners[0] = vRightTop * vNear;
    Corners[1] = vRightBottom * vNear;
    Corners[2] = vLeftTop * vNear;
    Corners[3] = vLeftBottom * vNear;
    Corners[4] = vRightTop * vFar;
    Corners[5] = vRightBottom * vFar;
    Corners[6] = vLeftTop * vFar;
    Corners[7] = vLeftBottom * vFar;

    // The Edges are:
    static const size_t edges[12][2] = {
        { 0, 1 }, { 2, 3 }, { 0, 2 }, { 1, 3 },    // Near plane
        { 4, 5 }, { 6, 7 }, { 4, 6 }, { 5, 7 },    // Far plane
        { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 },
    }; // Near to far

    float RadiusSq = vRadius * vRadius;

    // Check to see if the nearest feature is one of the edges (or corners).
    for (size_t i = 0u; i < 12u; ++i) {
        size_t ei0 = edges[i][0];
        size_t ei1 = edges[i][1];

        // Find the nearest point on the edge to the center of the sphere.
        // The corners of the frustum are included as the endpoints of the edges.
        glm::vec3 Point = PointOnLineSegmentNearestPoint(Corners[ei0], Corners[ei1], vCenter);

        glm::vec3 Delta = glm::vec3(vCenter) - Point;

        float DistSq = glm::dot(Delta, Delta);

        // If the distance to the center of the sphere to the point is less than
        // the radius of the sphere then it must intersect.
        Intersects |= DistSq <= RadiusSq;
    }

    if (Intersects) {
        return true;
    }

    // The sphere must be outside the frustum.
    return false;
}


//-----------------------------------------------------------------------------
// Exact axis aligned box vs frustum test.  Constructs an oriented box and uses
// the oriented box vs frustum test.
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const BoundingBox& box) const noexcept {
    // Make the axis aligned box oriented and do an OBB vs frustum test.
    BoundingOrientedBox obox(box.Center, box.Extents, glm::quat(0.0f, 0.0f, 0.0f, 1.0f));
    return Intersects(obox);
}


//-----------------------------------------------------------------------------
// Exact oriented box vs frustum test.
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const BoundingOrientedBox& box) const noexcept {
    // Build the frustum planes.
    glm::vec4 Planes[6];
    Planes[0] = glm::vec4(0.0f, 0.0f, -1.0f, Near);
    Planes[1] = glm::vec4(0.0f, 0.0f, 1.0f, -Far);
    Planes[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    Planes[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    Planes[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    Planes[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    // Load the box.
    glm::vec4 Center = glm::vec4(box.Center, 0.0f);
    glm::vec4 Extents = glm::vec4(box.Extents, 0.0f);
    glm::quat BoxOrientation = box.Orientation;

    // Transform the oriented box into the space of the frustum in order to
    // minimize the number of transforms we have to do.
    Center = (Center - glm::vec4(Origin, 0.0f)) * glm::inverse(Orientation);
    BoxOrientation = BoxOrientation * glm::conjugate(Orientation);

    // Set w of the center to one so we can dot4 with the plane.
    Center.w = 1.0f;

    // Build the 3x3 rotation matrix that defines the box axes.
    glm::mat4x4 R = glm::mat4(BoxOrientation);

    // Check against each plane of the frustum.
    bool Outside = false;
    bool InsideAll = true;
    bool CenterInsideAll = true;

    for (size_t i = 0u; i < 6u; ++i) {
        // Compute the distance to the center of the box.
        float Dist = glm::dot(Center, Planes[i]);

        // Project the axes of the box onto the normal of the plane.  Half the
        // length of the projection (sometime called the "radius") is equal to
        // h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
        // where h(i) are extents of the box, n is the plane normal, and b(i) are the
        // axes of the box.
        glm::vec4 Radius;
        Radius.x = glm::dot(Planes[i], R[0]);
        Radius.y = glm::dot(Planes[i], R[1]);
        Radius.z = glm::dot(Planes[i], R[2]);
        Radius.w = glm::dot(Extents, glm::abs(Radius));

        // Outside the plane?
        Outside |= Dist > Radius.w;

        // Fully inside the plane?
        InsideAll &= Dist <= -Radius.w;

        // Check if the center is inside the plane.
        CenterInsideAll &= Dist <= 0.0f;
    }

    // If the box is outside any of the planes it is outside.
    if (Outside) {
        return false;
    }

    // If the box is inside all planes it is fully inside.
    if (InsideAll) {
        return true;
    }

    // If the center of the box is inside all planes and the box intersects
    // one or more planes then it must intersect.
    if (CenterInsideAll) {
        return true;
    }

    // Build the corners of the frustum.
    glm::vec4 vRightTop(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 vRightBottom(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vLeftTop(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 vLeftBottom(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vNear(Near, Near, Near, Near);
    glm::vec4 vFar(Far, Far, Far, Far);

    glm::vec4 Corners[CORNER_COUNT];
    Corners[0] = vRightTop * vNear;
    Corners[1] = vRightBottom * vNear;
    Corners[2] = vLeftTop * vNear;
    Corners[3] = vLeftBottom * vNear;
    Corners[4] = vRightTop * vFar;
    Corners[5] = vRightBottom * vFar;
    Corners[6] = vLeftTop * vFar;
    Corners[7] = vLeftBottom * vFar;

    // Test against box axes (3)
    {
        // Find the min/max values of the projection of the frustum onto each axis.
        glm::vec4 FrustumMin;
        glm::vec4 FrustumMax;

        FrustumMin.x = glm::dot(Corners[0], R[0]);
        FrustumMin.y = glm::dot(Corners[0], R[1]);
        FrustumMin.z = glm::dot(Corners[0], R[2]);

        FrustumMax = FrustumMin;

        for (size_t i = 1u; i < BoundingOrientedBox::CORNER_COUNT; ++i) {
            glm::vec4 Temp;
            Temp.x = glm::dot(Corners[i], R[0]);
            Temp.y = glm::dot(Corners[i], R[1]);
            Temp.z = glm::dot(Corners[i], R[2]);

            FrustumMin.x = glm::min(FrustumMin.x, Temp.x);
            FrustumMin.y = glm::min(FrustumMin.y, Temp.y);
            FrustumMin.z = glm::min(FrustumMin.z, Temp.z);

            FrustumMax.x = glm::max(FrustumMax.x, Temp.x);
            FrustumMax.y = glm::max(FrustumMax.y, Temp.y);
            FrustumMax.z = glm::max(FrustumMax.z, Temp.z);
        }

        // Project the center of the box onto the axes.
        glm::vec4 BoxDist;
        BoxDist.x = glm::dot(Center, R[0]);
        BoxDist.y = glm::dot(Center, R[1]);
        BoxDist.z = glm::dot(Center, R[2]);

        // The projection of the box onto the axis is just its Center and Extents.
        // if (min > box_max || max < box_min) reject;
        bool Result[3];
        Result[0] = (FrustumMin.x > (BoxDist.x + Extents.x)) || (FrustumMax.x < (BoxDist.x - Extents.x));
        Result[1] = (FrustumMin.y > (BoxDist.y + Extents.y)) || (FrustumMax.y < (BoxDist.y - Extents.y));
        Result[2] = (FrustumMin.z > (BoxDist.z + Extents.z)) || (FrustumMax.z < (BoxDist.z - Extents.z));

        if (Result[0] || Result[1] || Result[2]) {
            return false;
        }
    }

    // Test against edge/edge axes (3*6).
    glm::vec4 FrustumEdgeAxis[6];

    FrustumEdgeAxis[0] = vRightTop;
    FrustumEdgeAxis[1] = vRightBottom;
    FrustumEdgeAxis[2] = vLeftTop;
    FrustumEdgeAxis[3] = vLeftBottom;
    FrustumEdgeAxis[4] = vRightTop - vLeftTop;
    FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

    for (size_t i = 0u; i < 3u; ++i) {
        for (size_t j = 0u; j < 6u; ++j) {
            // Compute the axis we are going to test.
            glm::vec4 Axis = glm::vec4(glm::cross(glm::vec3(R[i]), glm::vec3(FrustumEdgeAxis[j])), 0.0f);

            // Find the min/max values of the projection of the frustum onto the axis.
            float FrustumMin = glm::dot(Axis, Corners[0]);
            float FrustumMax = FrustumMin;

            for (size_t k = 1u; k < CORNER_COUNT; k++) {
                float Temp = glm::dot(Axis, Corners[k]);
                FrustumMin = glm::min(FrustumMin, Temp);
                FrustumMax = glm::max(FrustumMax, Temp);
            }

            // Project the center of the box onto the axis.
            float Dist = glm::dot(Center, Axis);

            // Project the axes of the box onto the axis to find the "radius" of the box.
            glm::vec4 Radius(0.0f);
            Radius.x = glm::dot(Axis, R[0]);
            Radius.y = glm::dot(Axis, R[1]);
            Radius.z = glm::dot(Axis, R[2]);
            Radius.w = glm::dot(Extents, glm::abs(Radius));

            // if (center > max + radius || center < min - radius) reject;
            Outside |= Dist > (FrustumMax + Radius.w);
            Outside |= Dist < (FrustumMin - Radius.w);
        }
    }

    if (Outside) {
        return false;
    }

    // If we did not find a separating plane then the box must intersect the frustum.
    return true;
}


//-----------------------------------------------------------------------------
// Exact frustum vs frustum test.
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const BoundingFrustum& fr) const noexcept {
    // Load origin and orientation of frustum B.
    glm::vec4 OriginB = glm::vec4(Origin, 0.0f);
    glm::quat OrientationB = Orientation;

    // Build the planes of frustum B.
    glm::vec4 AxisB[6];
    AxisB[0] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    AxisB[1] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    AxisB[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    AxisB[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    AxisB[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    AxisB[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    glm::vec4 PlaneDistB[6];
    PlaneDistB[0] = glm::vec4(-Near);
    PlaneDistB[1] = glm::vec4(Far);
    PlaneDistB[2] = glm::vec4(0.0f);
    PlaneDistB[3] = glm::vec4(0.0f);
    PlaneDistB[4] = glm::vec4(0.0f);
    PlaneDistB[5] = glm::vec4(0.0f);

    // Load origin and orientation of frustum A.
    glm::vec4 OriginA = glm::vec4(fr.Origin, 0.0f);
    glm::quat OrientationA = fr.Orientation;

    // Transform frustum A into the space of the frustum B in order to
    // minimize the number of transforms we have to do.
    OriginA = glm::inverse(OrientationB) * (OriginA - OriginB);
    OrientationA = OrientationA * glm::conjugate(OrientationB);

    // Build the corners of frustum A (in the local space of B).
    glm::vec4 RightTopA = glm::vec4(fr.RightSlope, fr.TopSlope, 1.0f, 0.0f);
    glm::vec4 RightBottomA = glm::vec4(fr.RightSlope, fr.BottomSlope, 1.0f, 0.0f);
    glm::vec4 LeftTopA = glm::vec4(fr.LeftSlope, fr.TopSlope, 1.0f, 0.0f);
    glm::vec4 LeftBottomA = glm::vec4(fr.LeftSlope, fr.BottomSlope, 1.0f, 0.0f);
    glm::vec4 NearA = glm::vec4(fr.Near);
    glm::vec4 FarA = glm::vec4(fr.Far);

    RightTopA = OrientationA * RightTopA;
    RightBottomA = OrientationA * RightBottomA;
    LeftTopA = OrientationA * LeftTopA;
    LeftBottomA = OrientationA * LeftBottomA;

    glm::vec4 CornersA[CORNER_COUNT];
    CornersA[0] = (RightTopA * NearA) + OriginA;
    CornersA[1] = (RightBottomA * NearA) + OriginA;
    CornersA[2] = (LeftTopA * NearA) + OriginA;
    CornersA[3] = (LeftBottomA * NearA) + OriginA;
    CornersA[4] = (RightTopA * FarA) + OriginA;
    CornersA[5] = (RightBottomA * FarA) + OriginA;
    CornersA[6] = (LeftTopA * FarA) + OriginA;
    CornersA[7] = (LeftBottomA * FarA) + OriginA;

    // Check frustum A against each plane of frustum B.
    bool Outside = false;
    bool InsideAll = true;

    for (size_t i = 0u; i < 6u; ++i) {
        // Find the min/max projection of the frustum onto the plane normal.
        float Min;
        float Max;

        Min = Max = glm::dot(AxisB[i], CornersA[0]);

        for (size_t j = 1u; j < CORNER_COUNT; ++j) {
            float Temp = glm::dot(AxisB[i], CornersA[j]);
            Min = glm::min(Min, Temp);
            Max = glm::max(Max, Temp);
        }

        // Outside the plane?
        Outside |= (Min > PlaneDistB[i].x) || (Min > PlaneDistB[i].y);

        // Fully inside the plane?
        InsideAll &= (Max < PlaneDistB[i].x) && (Max < PlaneDistB[i].y);
    }

    // If the frustum A is outside any of the planes of frustum B it is outside.
    if (Outside) {
        return false;
    }

    // If frustum A is inside all planes of frustum B it is fully inside.
    if (InsideAll) {
        return true;
    }

    // Build the corners of frustum B.
    glm::vec4 RightTopB = glm::vec4(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 RightBottomB = glm::vec4(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 LeftTopB = glm::vec4(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 LeftBottomB = glm::vec4(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 NearB = glm::vec4(Near);
    glm::vec4 FarB = glm::vec4(Far);

    glm::vec4 CornersB[BoundingFrustum::CORNER_COUNT];
    CornersB[0] = RightTopB * NearB;
    CornersB[1] = RightBottomB * NearB;
    CornersB[2] = LeftTopB * NearB;
    CornersB[3] = LeftBottomB * NearB;
    CornersB[4] = RightTopB * FarB;
    CornersB[5] = RightBottomB * FarB;
    CornersB[6] = LeftTopB * FarB;
    CornersB[7] = LeftBottomB * FarB;

    // Build the planes of frustum A (in the local space of B).
    glm::vec4 AxisA[6];
    float PlaneDistA[6];

    AxisA[0] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    AxisA[1] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    AxisA[2] = glm::vec4(1.0f, 0.0f, -fr.RightSlope, 0.0f);
    AxisA[3] = glm::vec4(-1.0f, 0.0f, fr.LeftSlope, 0.0f);
    AxisA[4] = glm::vec4(0.0f, 1.0f, -fr.TopSlope, 0.0f);
    AxisA[5] = glm::vec4(0.0f, -1.0f, fr.BottomSlope, 0.0f);

    AxisA[0] = AxisA[0] * OrientationA;
    AxisA[1] = -AxisA[0];
    AxisA[2] = AxisA[2] * OrientationA;
    AxisA[3] = AxisA[3] * OrientationA;
    AxisA[4] = AxisA[4] * OrientationA;
    AxisA[5] = AxisA[5] * OrientationA;

    PlaneDistA[0] = glm::dot(AxisA[0], CornersA[0]);  // Re-use corner on near plane.
    PlaneDistA[1] = glm::dot(AxisA[1], CornersA[4]);  // Re-use corner on far plane.
    PlaneDistA[2] = glm::dot(AxisA[2], OriginA);
    PlaneDistA[3] = glm::dot(AxisA[3], OriginA);
    PlaneDistA[4] = glm::dot(AxisA[4], OriginA);
    PlaneDistA[5] = glm::dot(AxisA[5], OriginA);

    // Check each axis of frustum A for a seperating plane (5).
    for (size_t i = 0u; i < 6u; ++i) {
        // Find the minimum projection of the frustum onto the plane normal.
        float Min = glm::dot(AxisA[i], CornersB[0]);

        for (size_t j = 1u; j < CORNER_COUNT; ++j) {
            float Temp = glm::dot(AxisA[i], CornersB[j]);
            Min = glm::min(Min, Temp);
        }

        // Outside the plane?
        Outside |= Min > PlaneDistA[i];
    }

    // If the frustum B is outside any of the planes of frustum A it is outside.
    if (Outside) {
        return false;
    }

    // Check edge/edge axes (6 * 6).
    glm::vec4 FrustumEdgeAxisA[6];
    FrustumEdgeAxisA[0] = RightTopA;
    FrustumEdgeAxisA[1] = RightBottomA;
    FrustumEdgeAxisA[2] = LeftTopA;
    FrustumEdgeAxisA[3] = LeftBottomA;
    FrustumEdgeAxisA[4] = RightTopA - LeftTopA;
    FrustumEdgeAxisA[5] = LeftBottomA - LeftTopA;

    glm::vec4 FrustumEdgeAxisB[6];
    FrustumEdgeAxisB[0] = RightTopB;
    FrustumEdgeAxisB[1] = RightBottomB;
    FrustumEdgeAxisB[2] = LeftTopB;
    FrustumEdgeAxisB[3] = LeftBottomB;
    FrustumEdgeAxisB[4] = RightTopB - LeftTopB;
    FrustumEdgeAxisB[5] = LeftBottomB - LeftTopB;

    for (size_t i = 0u; i < 6u; ++i) {
        for (size_t j = 0u; j < 6u; ++j) {
            // Compute the axis we are going to test.
            glm::vec4 Axis = glm::vec4(glm::cross(glm::vec3(FrustumEdgeAxisA[i]), glm::vec3(FrustumEdgeAxisB[j])), 0.0f);

            // Find the min/max values of the projection of both frustums onto the axis.
            float MinA;
            float MaxA;
            float MinB;
            float MaxB;

            MinA = MaxA = glm::dot(Axis, CornersA[0]);
            MinB = MaxB = glm::dot(Axis, CornersB[0]);

            for (size_t k = 1; k < CORNER_COUNT; ++k) {
                float TempA = glm::dot(Axis, CornersA[k]);
                MinA = glm::min(MinA, TempA);
                MaxA = glm::max(MaxA, TempA);

                float TempB = glm::dot(Axis, CornersB[k]);
                MinB = glm::min(MinB, TempB);
                MaxB = glm::max(MaxB, TempB);
            }

            // if (MinA > MaxB || MinB > MaxA) reject
            Outside |= MinA > MaxB;
            Outside |= MinB > MaxA;
        }
    }

    // If there is a seperating plane, then the frustums do not intersect.
    if (Outside) {
        return false;
    }

    // If we did not find a separating plane then the frustums intersect.
    return true;
}


//-----------------------------------------------------------------------------
// Triangle vs frustum test.
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    // Build the frustum planes (NOTE: D is negated from the usual).
    glm::vec4 Planes[6];
    Planes[0] = glm::vec4(0.0f, 0.0f, -1.0f, -Near);
    Planes[1] = glm::vec4(0.0f, 0.0f, 1.0f, Far);
    Planes[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    Planes[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    Planes[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    Planes[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    glm::quat invOrientation = glm::inverse(Orientation);

    // Transform triangle into the local space of frustum.
    glm::vec4 TV0 = invOrientation * glm::vec4(V0 - Origin, 1.0f);
    glm::vec4 TV1 = invOrientation * glm::vec4(V1 - Origin, 1.0f);
    glm::vec4 TV2 = invOrientation * glm::vec4(V2 - Origin, 1.0f);

    // Test each vertex of the triangle against the frustum planes.
    bool Outside = false;
    bool InsideAll = true;

    for (size_t i = 0u; i < 6u; ++i) {
        float Dist0 = glm::dot(TV0, Planes[i]);
        float Dist1 = glm::dot(TV1, Planes[i]);
        float Dist2 = glm::dot(TV2, Planes[i]);

        float MinDist = glm::min(Dist0, Dist1);
        MinDist = glm::min(MinDist, Dist2);
        float MaxDist = glm::max(Dist0, Dist1);
        MaxDist = glm::max(MaxDist, Dist2);

        float PlaneDist = Planes[i].w;

        // Outside the plane?
        Outside |= MinDist > PlaneDist;

        // Fully inside the plane?
        InsideAll &= MaxDist <= PlaneDist;
    }

    // If the triangle is outside any of the planes it is outside.
    if (Outside) {
        return false;
    }

    // If the triangle is inside all planes it is fully inside.
    if (InsideAll) {
        return true;
    }

    // Build the corners of the frustum.
    glm::vec3 vRightTop = glm::vec4(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec3 vRightBottom = glm::vec4(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec3 vLeftTop = glm::vec4(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec3 vLeftBottom = glm::vec4(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec3 vNear = glm::vec4(Near);
    glm::vec3 vFar = glm::vec4(Far);

    glm::vec3 Corners[CORNER_COUNT];
    Corners[0] = vRightTop * vNear;
    Corners[1] = vRightBottom * vNear;
    Corners[2] = vLeftTop * vNear;
    Corners[3] = vLeftBottom * vNear;
    Corners[4] = vRightTop * vFar;
    Corners[5] = vRightBottom * vFar;
    Corners[6] = vLeftTop * vFar;
    Corners[7] = vLeftBottom * vFar;

    // Test the plane of the triangle.
    glm::vec3 Normal = glm::cross(V1 - V0, V2 - V0);
    float Dist = glm::dot(Normal, V0);

    float MinDist;
    float MaxDist;
    MinDist = MaxDist = glm::dot(Corners[0], Normal);
    for (size_t i = 1u; i < CORNER_COUNT; ++i) {
        float Temp = glm::dot(Corners[i], Normal);
        MinDist = glm::min(MinDist, Temp);
        MaxDist = glm::max(MaxDist, Temp);
    }

    Outside = (MinDist > Dist) || (MaxDist < Dist);
    if (Outside) {
        return false;
    }

    // Check the edge/edge axes (3*6).
    glm::vec3 TriangleEdgeAxis[3];
    TriangleEdgeAxis[0] = V1 - V0;
    TriangleEdgeAxis[1] = V2 - V1;
    TriangleEdgeAxis[2] = V0 - V2;

    glm::vec3 FrustumEdgeAxis[6];
    FrustumEdgeAxis[0] = vRightTop;
    FrustumEdgeAxis[1] = vRightBottom;
    FrustumEdgeAxis[2] = vLeftTop;
    FrustumEdgeAxis[3] = vLeftBottom;
    FrustumEdgeAxis[4] = vRightTop - vLeftTop;
    FrustumEdgeAxis[5] = vLeftBottom - vLeftTop;

    for (size_t i = 0u; i < 3u; ++i) {
        for (size_t j = 0u; j < 6u; ++j) {
            // Compute the axis we are going to test.
            glm::vec3 Axis = glm::cross(TriangleEdgeAxis[i], FrustumEdgeAxis[j]);

            // Find the min/max of the projection of the triangle onto the axis.
            float MinA;
            float MaxA;

            float Dist0 = glm::dot(V0, Axis);
            float Dist1 = glm::dot(V1, Axis);
            float Dist2 = glm::dot(V2, Axis);

            MinA = glm::min(Dist0, Dist1);
            MinA = glm::min(MinA, Dist2);
            MaxA = glm::max(Dist0, Dist1);
            MaxA = glm::max(MaxA, Dist2);

            // Find the min/max of the projection of the frustum onto the axis.
            float MinB;
            float MaxB;

            MinB = MaxB = glm::dot(Axis, Corners[0]);

            for (size_t k = 1u; k < CORNER_COUNT; ++k) {
                float Temp = glm::dot(Axis, Corners[k]);
                MinB = glm::min(MinB, Temp);
                MaxB = glm::max(MaxB, Temp);
            }

            // if (MinA > MaxB || MinB > MaxA) reject;
            Outside |= MinA > MaxB;
            Outside |= MinB > MaxA;
        }
    }

    if (Outside) {
        return false;
    }

    // If we did not find a separating plane then the triangle must intersect the frustum.
    return true;
}


//-----------------------------------------------------------------------------
PlaneIntersectionType BoundingFrustum::Intersects(const glm::vec4& Plane) const noexcept {

    // Load origin and orientation of the frustum.
    glm::vec4 vOrigin = glm::vec4(Origin, 1.0f);
    // Set w of the origin to one so we can dot4 with a plane.

    // Build the corners of the frustum (in world space).
    glm::vec4 RightTop = glm::vec4(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 RightBottom = glm::vec4(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 LeftTop = glm::vec4(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 LeftBottom = glm::vec4(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vNear = glm::vec4(Near);
    glm::vec4 vFar = glm::vec4(Far);

    RightTop = Orientation * RightTop;
    RightBottom = Orientation * RightBottom;
    LeftTop = Orientation * LeftTop;
    LeftBottom = Orientation * LeftBottom;

    glm::vec4 Corners0 = (RightTop * vNear) + vOrigin;
    glm::vec4 Corners1 = (RightBottom * vNear) + vOrigin;
    glm::vec4 Corners2 = (LeftTop * vNear) + vOrigin;
    glm::vec4 Corners3 = (LeftBottom * vNear) + vOrigin;
    glm::vec4 Corners4 = (RightTop * vFar) + vOrigin;
    glm::vec4 Corners5 = (RightBottom * vFar) + vOrigin;
    glm::vec4 Corners6 = (LeftTop * vFar) + vOrigin;
    glm::vec4 Corners7 = (LeftBottom * vFar) + vOrigin;

    bool Outside, Inside;
    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane, Outside, Inside);

    // If the frustum is outside any plane it is outside.
    if (Outside) {
        return FRONT;
    }

    // If the frustum is inside all planes it is inside.
    if (Inside) {
        return BACK;
    }

    // The frustum is not inside all planes or outside a plane it intersects.
    return INTERSECTING;
}


//-----------------------------------------------------------------------------
// Ray vs. frustum test
//-----------------------------------------------------------------------------
bool BoundingFrustum::Intersects(const glm::vec3& rayOrigin, const glm::vec3& Direction, float& Dist) const noexcept
{
    // If ray starts inside the frustum, return a distance of 0 for the hit
    if (Contains(rayOrigin) == CONTAINS) {
        Dist = 0.0f;
        return true;
    }

    // Build the frustum planes.
    glm::vec4 Planes[6];
    Planes[0] = glm::vec4(0.0f, 0.0f, -1.0f, Near);
    Planes[1] = glm::vec4(0.0f, 0.0f, 1.0f, -Far);
    Planes[2] = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
    Planes[3] = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
    Planes[4] = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
    Planes[5] = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);

    // This algorithm based on "Fast Ray-Convex Polyhedron Intersectin," in James Arvo, ed., Graphics Gems II pp. 247-250
    float tnear = -FLT_MAX;
    float tfar = FLT_MAX;

    for (size_t i = 0u; i < 6u; ++i) {
        glm::vec4 Plane = PlaneTransform(Planes[i], Orientation, Origin);
        Plane = PlaneNormalize(Plane);

        float AxisDotOrigin = PlaneDotCoord(Plane, rayOrigin);
        float AxisDotDirection = glm::dot(glm::vec3(Plane), Direction);

        if (glm::abs(AxisDotDirection) <= g_RayEpsilon.x) {
            // Ray is parallel to plane - check if ray origin is inside plane's
            if (AxisDotOrigin > 0.0f) {
                // Ray origin is outside half-space.
                Dist = 0.0f;
                return false;
            }
        }
        else {
            // Ray not parallel - get distance to plane.
            float vd = AxisDotDirection;
            float vn = AxisDotOrigin;
            float t = -vn / vd;
            if (vd < 0.0f) {
                // Front face - T is a near point.
                if (t > tfar) {
                    Dist = 0.0f;
                    return false;
                }
                if (t > tnear) {
                    // Hit near face.
                    tnear = t;
                }
            }
            else {
                // back face - T is far point.
                if (t < tnear) {
                    Dist = 0.0f;
                    return false;
                }
                if (t < tfar) {
                    // Hit far face.
                    tfar = t;
                }
            }
        }
    }

    // Survived all tests.
    // Note: if ray originates on polyhedron, may want to change 0.0f to some
    // epsilon to avoid intersecting the originating face.
    float distance = (tnear >= 0.0f) ? tnear : tfar;
    if (distance >= 0.0f) {
        Dist = distance;
        return true;
    }

    Dist = 0.0f;
    return false;
}


//-----------------------------------------------------------------------------
// Test a frustum vs 6 planes (typically forming another frustum).
//-----------------------------------------------------------------------------
ContainmentType BoundingFrustum::ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept {
    
    glm::vec4 vOrigin = glm::vec4(Origin, 1.0f);
    // Set w of the origin to one so we can dot4 with a plane.

    // Build the corners of the frustum (in world space).
    glm::vec4 RightTop = glm::vec4(RightSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 RightBottom = glm::vec4(RightSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 LeftTop = glm::vec4(LeftSlope, TopSlope, 1.0f, 0.0f);
    glm::vec4 LeftBottom = glm::vec4(LeftSlope, BottomSlope, 1.0f, 0.0f);
    glm::vec4 vNear = glm::vec4(Near);
    glm::vec4 vFar = glm::vec4(Far);

    RightTop = Orientation * RightTop;
    RightBottom = Orientation * RightBottom;
    LeftTop = Orientation * LeftTop;
    LeftBottom = Orientation * LeftBottom;

    glm::vec4 Corners0 = (RightTop * vNear) + vOrigin;
    glm::vec4 Corners1 = (RightBottom * vNear) + vOrigin;
    glm::vec4 Corners2 = (LeftTop * vNear) + vOrigin;
    glm::vec4 Corners3 = (LeftBottom * vNear) + vOrigin;
    glm::vec4 Corners4 = (RightTop * vFar) + vOrigin;
    glm::vec4 Corners5 = (RightBottom * vFar) + vOrigin;
    glm::vec4 Corners6 = (LeftTop * vFar) + vOrigin;
    glm::vec4 Corners7 = (LeftBottom * vFar) + vOrigin;

    bool Outside, Inside;

    // Test against each plane.
    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane0, Outside, Inside);

    bool AnyOutside = Outside;
    bool AllInside = Inside;

    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane1, Outside, Inside);

    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane2, Outside, Inside);

    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane3, Outside, Inside);

    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane4, Outside, Inside);

    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectFrustumPlane(Corners0, Corners1, Corners2, Corners3, Corners4, Corners5, Corners6, Corners7, Plane5, Outside, Inside);

    AnyOutside |= Outside;
    AllInside &= Inside;

    // If the frustum is outside any plane it is outside.
    if (AnyOutside) {
        return DISJOINT;
    }

    // If the frustum is inside all planes it is inside.
    if (AllInside) {
        return CONTAINS;
    }

    // The frustum is not inside all planes or outside a plane, it may intersect.
    return INTERSECTS;
}


//-----------------------------------------------------------------------------
// Build the 6 frustum planes from a frustum.
//
// The intended use for these routines is for fast culling to a view frustum.
// When the volume being tested against a view frustum is small relative to the
// view frustum it is usually either inside all six planes of the frustum
// (CONTAINS) or outside one of the planes of the frustum (DISJOINT). If neither
// of these cases is true then it may or may not be intersecting the frustum
// (INTERSECTS)
//-----------------------------------------------------------------------------
void BoundingFrustum::GetPlanes(glm::vec4* NearPlane, glm::vec4* FarPlane, glm::vec4* RightPlane, glm::vec4* LeftPlane, glm::vec4* TopPlane, glm::vec4* BottomPlane) const noexcept {
    if (NearPlane) {
        glm::vec4 vNearPlane = glm::vec4(0.0f, 0.0f, -1.0f, Near);
        vNearPlane = PlaneTransform(vNearPlane, Orientation, Origin);
        *NearPlane = PlaneNormalize(vNearPlane);
    }

    if (FarPlane) {
        glm::vec4 vFarPlane = glm::vec4(0.0f, 0.0f, 1.0f, -Far);
        vFarPlane = PlaneTransform(vFarPlane, Orientation, Origin);
        *FarPlane = PlaneNormalize(vFarPlane);
    }

    if (RightPlane) {
        glm::vec4 vRightPlane = glm::vec4(1.0f, 0.0f, -RightSlope, 0.0f);
        vRightPlane = PlaneTransform(vRightPlane, Orientation, Origin);
        *RightPlane = PlaneNormalize(vRightPlane);
    }

    if (LeftPlane) {
        glm::vec4 vLeftPlane = glm::vec4(-1.0f, 0.0f, LeftSlope, 0.0f);
        vLeftPlane = PlaneTransform(vLeftPlane, Orientation, Origin);
        *LeftPlane = PlaneNormalize(vLeftPlane);
    }

    if (TopPlane) {
        glm::vec4 vTopPlane = glm::vec4(0.0f, 1.0f, -TopSlope, 0.0f);
        vTopPlane = PlaneTransform(vTopPlane, Orientation, Origin);
        *TopPlane = PlaneNormalize(vTopPlane);
    }

    if (BottomPlane) {
        glm::vec4 vBottomPlane = glm::vec4(0.0f, -1.0f, BottomSlope, 0.0f);
        vBottomPlane = PlaneTransform(vBottomPlane, Orientation, Origin);
        *BottomPlane = PlaneNormalize(vBottomPlane);
    }
}


//-----------------------------------------------------------------------------
// Build a frustum from a persepective projection matrix.  The matrix may only
// contain a projection; any rotation, translation or scale will cause the
// constructed frustum to be incorrect.
//-----------------------------------------------------------------------------
void BoundingFrustum::CreateFromMatrix(BoundingFrustum& Out, const glm::mat4x4& Projection, bool rhcoords) noexcept {
    float forward = -1.0f;

    // Corners of the projection frustum in NDC space.
    static glm::vec4 NDCPoints[6] = {
        glm::vec4( 1.0f,  0.0f, forward, 1.0f ), // right (at far plane)
        glm::vec4(-1.0f,  0.0f, forward, 1.0f ), // left
        glm::vec4( 0.0f,  1.0f, forward, 1.0f ), // top
        glm::vec4( 0.0f, -1.0f, forward, 1.0f ), // bottom

        glm::vec4( 0.0f,  0.0f, 0.0f,    1.0f ), // near
        glm::vec4( 0.0f,  0.0f, forward, 1.0f )  // far
    };

    glm::mat4x4 matInverse = glm::inverse(Projection);

    // Compute the frustum corners in world space.
    glm::vec4 Points[6];

    for (size_t i = 0u; i < 6u; ++i) {
        // Transform point.
        Points[i] = matInverse * NDCPoints[i];
    }

    Out.Origin = glm::vec3(0.0f, 0.0f, 0.0f);
    Out.Orientation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);

    // Compute the slopes.
    Points[0] = Points[0] * (1.0f / Points[0].z);
    Points[1] = Points[1] * (1.0f / Points[1].z);
    Points[2] = Points[2] * (1.0f / Points[2].z);
    Points[3] = Points[3] * (1.0f / Points[3].z);

    Out.RightSlope = Points[0].x;
    Out.LeftSlope = Points[1].x;
    Out.TopSlope = Points[2].y;
    Out.BottomSlope = Points[3].y;

    // Compute near and far.
    Points[4] = Points[4] * (1.0f / Points[4].w);
    Points[5] = Points[5] * (1.0f / Points[5].w);

    if (rhcoords) {
        Out.Near = Points[5].z;
        Out.Far = Points[4].z;
    }
    else {
        Out.Near = Points[4].z;
        Out.Far = Points[5].z;
    }
}