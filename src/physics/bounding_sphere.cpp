#include "bounding_sphere.h"
#include "triangle_tests.h"
#include "bounding_frustum.h"
#include "bounding_box.h"
#include "bounding_oriented_box.h"


//-----------------------------------------------------------------------------
// Transform a sphere by an angle preserving transform.
//-----------------------------------------------------------------------------
void BoundingSphere::Transform(BoundingSphere& Out, const glm::mat4x4& M) const noexcept {

    // Transform the center of the sphere.
    glm::vec4 C = glm::vec4(Center, 1.0f) * M;

    float dX = glm::dot(M[0], M[0]);
    float dY = glm::dot(M[1], M[1]);
    float dZ = glm::dot(M[2], M[2]);

    float d = glm::max(dX, glm::max(dY, dZ));

    // Store the center sphere.
    Out.Center = glm::vec3(C);

    // Scale the radius of the pshere.
    float Scale = std::sqrtf(d);
    Out.Radius = Radius * Scale;
}

void BoundingSphere::Transform(BoundingSphere& Out, float Scale, const glm::quat& Rotation, const glm::vec3& Translation) const noexcept {

    // Transform the center of the sphere.
    Out.Center = ((Center * Scale) * Rotation) + Translation;
    // Store the center sphere.

    // Scale the radius of the pshere.
    Out.Radius = Radius * Scale;
}

//-----------------------------------------------------------------------------
// Point in sphere test.
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::Contains(const glm::vec3& Point) const noexcept {
    glm::vec3 offset = Point - Center;
    float DistanceSquared = glm::dot(offset, offset);
    float RadiusSquared = Radius * Radius;
    return DistanceSquared < RadiusSquared ? CONTAINS : DISJOINT;
}

//-----------------------------------------------------------------------------
// Triangle in sphere test
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    if (!Intersects(V0, V1, V2)) {
        return DISJOINT;
    }
    float RadiusSquared = Radius * Radius;
    glm::vec3 offset = V0 - Center;
    float DistanceSquared = glm::dot(offset, offset);
    bool Inside = DistanceSquared <= RadiusSquared;

    offset = V1 - Center;
    DistanceSquared = glm::dot(offset, offset);
    Inside &= DistanceSquared <= RadiusSquared;

    glm::vec3 offset = V2 - Center;
    DistanceSquared = glm::dot(offset, offset);
    Inside &= DistanceSquared <= RadiusSquared;

    return Inside ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Sphere in sphere test.
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::Contains(const BoundingSphere& sh) const noexcept {
    glm::vec3 Center1 = Center;
    float r1 = Radius;

    glm::vec3 Center2 = sh.Center;
    float r2 = sh.Radius;

    glm::vec3 V = Center2 - Center1;
    float Dist = glm::length(V);

    return (r1 + r2 >= Dist) ? ((r1 - r2 >= Dist) ? CONTAINS : INTERSECTS) : DISJOINT;
}

//-----------------------------------------------------------------------------
// Axis-aligned box in sphere test
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::Contains(const BoundingBox& box) const noexcept {
    if (!box.Intersects(*this)) {
        return DISJOINT;
    }

    float RadiusSq = Radius * Radius;
    bool InsideAll = true;

    glm::vec3 offset = box.Center - Center;

    for (size_t i = 0u; i < BoundingBox::CORNER_COUNT; ++i) {
        glm::vec3 C = box.Extents * glm::vec3(g_BoxOffset[i]) + offset;
        float d = glm::dot(C, C);
        InsideAll &= d <= RadiusSq;
    }

    return InsideAll ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Oriented box in sphere test
//-----------------------------------------------------------------------------
inline ContainmentType BoundingSphere::Contains(const BoundingOrientedBox& box) const noexcept {
    if (!box.Intersects(*this)) {
        return DISJOINT;
    }

    float RadiusSq = Radius * Radius;

    bool InsideAll = true;

    for (size_t i = 0u; i < BoundingOrientedBox::CORNER_COUNT; ++i) {
        //XMVECTOR C = XMVectorAdd(XMVector3Rotate(XMVectorMultiply(boxExtents, g_BoxOffset[i]), boxOrientation), boxCenter);
        glm::vec3 C = (box.Extents * glm::vec3(g_BoxOffset[i]) * box.Orientation) + box.Center;
        glm::vec3 offset = Center - C;
        float d = glm::dot(offset, offset);
        InsideAll &= d <= RadiusSq;
    }

    return InsideAll ? CONTAINS : INTERSECTS;

}

//-----------------------------------------------------------------------------
// Frustum in sphere test
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::Contains(const BoundingFrustum& fr) const noexcept {
    if (!fr.Intersects(*this)) {
        return DISJOINT;
    }

    float RadiusSq = Radius * Radius;

    // Build the corners of the frustum.
    glm::vec3 vRightTop = glm::vec3(fr.RightSlope, fr.TopSlope, 1.0f);
    glm::vec3 vRightBottom = glm::vec3(fr.RightSlope, fr.BottomSlope, 1.0f);
    glm::vec3 vLeftTop = glm::vec3(fr.LeftSlope, fr.TopSlope, 1.0f);
    glm::vec3 vLeftBottom = glm::vec3(fr.LeftSlope, fr.BottomSlope, 1.0f);
    glm::vec3 vNear = glm::vec3(fr.Near);
    glm::vec3 vFar = glm::vec3(fr.Far);

    glm::vec3 Corners[BoundingFrustum::CORNER_COUNT];
    Corners[0] = vRightTop * vNear;
    Corners[1] = vRightBottom * vNear;
    Corners[2] = vLeftTop * vNear;
    Corners[3] = vLeftBottom * vNear;
    Corners[4] = vRightTop * vFar;
    Corners[5] = vRightBottom * vFar;
    Corners[6] = vLeftTop * vFar;
    Corners[7] = vLeftBottom * vFar;

    bool InsideAll = true;
    for (size_t i = 0u; i < BoundingFrustum::CORNER_COUNT; ++i) {
        glm::vec3 C = (Corners[i] * fr.Orientation) + fr.Origin;
        glm::vec3 offset = Center - C;
        float d = glm::dot(offset, offset);
        InsideAll &= d <= RadiusSq;
    }

    return InsideAll ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Sphere vs. sphere test.
//-----------------------------------------------------------------------------
bool BoundingSphere::Intersects(const BoundingSphere& sh) const noexcept {
    // Load A.
    glm::vec3 vCenterA = Center;
    float vRadiusA = Radius;

    // Load B.
    glm::vec3 vCenterB = sh.Center;
    float vRadiusB = sh.Radius;

    // Distance squared between centers.
    glm::vec3 Delta = vCenterB - vCenterA;
    float DistanceSquared = glm::dot(Delta, Delta);

    // Sum of the radii squared.
    float RadiusSquared = vRadiusA + vRadiusB;
    RadiusSquared = RadiusSquared * RadiusSquared;

    return DistanceSquared <= RadiusSquared;
}

//-----------------------------------------------------------------------------
// Box vs. sphere test.
//-----------------------------------------------------------------------------
bool BoundingSphere::Intersects(const BoundingBox& box) const noexcept {
    return box.Intersects(*this);
}

bool BoundingSphere::Intersects(const BoundingOrientedBox& box) const noexcept {
    return box.Intersects(*this);
}

//-----------------------------------------------------------------------------
// Frustum vs. sphere test.
//-----------------------------------------------------------------------------
bool BoundingSphere::Intersects(const BoundingFrustum& fr) const noexcept {
    return fr.Intersects(*this);
}

//-----------------------------------------------------------------------------
// Triangle vs sphere test
//-----------------------------------------------------------------------------
bool BoundingSphere::Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {

    // Compute the plane of the triangle (has to be normalized).
    glm::vec3 N = glm::normalize(glm::cross(V1 - V0, V2 - V0));

    // Find the nearest feature on the triangle to the sphere.
    float Dist = glm::dot(Center - V0, N);

    // If the center of the sphere is farther from the plane of the triangle than
    // the radius of the sphere, then there cannot be an intersection.
    bool NoIntersection = Dist < -Radius;
    NoIntersection |= Dist > Radius;

    // Project the center of the sphere onto the plane of the triangle.
    glm::vec3 Point = Center - (N * Dist);

    // Is it inside all the edges? If so we intersect because the distance
    // to the plane is less than the radius.
    bool Intersection = PointOnPlaneInsideTriangle(Point, V0, V1, V2);

    // Find the nearest point on each edge.
    float RadiusSq = Radius * Radius;

    // Edge 0,1
    Point = PointOnLineSegmentNearestPoint(V0, V1, Center);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    glm::vec3 offset = Center - Point;
    Intersection |= glm::dot(offset, offset) <= RadiusSq;

    // Edge 1,2
    Point = PointOnLineSegmentNearestPoint(V1, V2, Center);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    offset = Center - Point;
    Intersection |= glm::dot(offset, offset) <= RadiusSq;

    // Edge 2,0
    Point = PointOnLineSegmentNearestPoint(V2, V0, Center);

    // If the distance to the center of the sphere to the point is less than
    // the radius of the sphere then it must intersect.
    offset = Center - Point;
    Intersection |= glm::dot(offset, offset) <= RadiusSq;

    return Intersection && NoIntersection;
}

//-----------------------------------------------------------------------------
// Sphere-plane intersection
//-----------------------------------------------------------------------------
PlaneIntersectionType BoundingSphere::Intersects(const glm::vec4& Plane) const noexcept {

    // Load the sphere.
    glm::vec4 vCenter = glm::vec4(Center, 1.0f);
    // Set w of the center to one so we can dot4 with a plane.

    bool Outside;
    bool Inside;
    IntersectSpherePlane(vCenter, Radius, Plane, Outside, Inside);

    // If the sphere is outside any plane it is outside.
    if (Outside) {
        return FRONT;
    }

    // If the sphere is inside all planes it is inside.
    if (Inside) {
        return BACK;
    }

    // The sphere is not inside all planes or outside a plane it intersects.
    return INTERSECTING;
}

//-----------------------------------------------------------------------------
// Compute the intersection of a ray (Origin, Direction) with a sphere.
//-----------------------------------------------------------------------------
bool BoundingSphere::Intersects(const glm::vec3& Origin, const glm::vec3& Direction, float& Dist) const noexcept {

    // l is the vector from the ray origin to the center of the sphere.
    glm::vec3 l = Center - Origin;

    // s is the projection of the l onto the ray direction.
    float s = glm::dot(l, Direction);

    float l2 = glm::dot(l, l);

    float r2 = Radius * Radius;

    // m2 is squared distance from the center of the sphere to the projection.
    float m2 = l2 - (s * s);

    bool NoIntersection;

    // If the ray origin is outside the sphere and the center of the sphere is
    // behind the ray origin there is no intersection.
    NoIntersection = (s < 0.0f) && (l2 > r2);

    // If the squared distance from the center of the sphere to the projection
    // is greater than the radius squared the ray will miss the sphere.
    NoIntersection |= m2 > r2;

    // The ray hits the sphere, compute the nearest intersection point.
    float q = std::sqrt(r2 - m2);
    float t1 = s - q;
    float t2 = s + q;

    bool OriginInside = l2 <= r2;
    bool t = OriginInside ? t2 : t1;

    if (!NoIntersection) {
        // Store the x-component to *pDist.
        Dist = t;
        return true;
    }

    Dist = 0.0f;
    return false;
}

//-----------------------------------------------------------------------------
// Test a sphere vs 6 planes (typically forming a frustum).
//-----------------------------------------------------------------------------
ContainmentType BoundingSphere::ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept {
    // Load the sphere.
    glm::vec4 vCenter = glm::vec4(Center, 1.0f);
    // Set w of the center to one so we can dot4 with a plane.

    bool Outside;
    bool Inside;

    // Test against each plane.
    IntersectSpherePlane(vCenter, Radius, Plane0, Outside, Inside);

    bool AnyOutside = Outside;
    bool AllInside = Inside;

    IntersectSpherePlane(vCenter, Radius, Plane1, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectSpherePlane(vCenter, Radius, Plane2, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectSpherePlane(vCenter, Radius, Plane3, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectSpherePlane(vCenter, Radius, Plane4, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectSpherePlane(vCenter, Radius, Plane5, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    // If the sphere is outside any plane it is outside.
    if (AnyOutside) {
        return DISJOINT;
    }

    // If the sphere is inside all planes it is inside.
    if (AllInside) {
        return CONTAINS;
    }

    // The sphere is not inside all planes or outside a plane, it may intersect.
    return INTERSECTS;
}

//-----------------------------------------------------------------------------
// Creates a bounding sphere that contains two other bounding spheres
//-----------------------------------------------------------------------------
void BoundingSphere::CreateMerged(BoundingSphere& Out, const BoundingSphere& S1, const BoundingSphere& S2) noexcept {
    glm::vec3 Center1 = S1.Center;
    float r1 = S1.Radius;

    glm::vec3 Center2 = S2.Center;
    float r2 = S2.Radius;

    glm::vec3 V = Center2 - Center1;

    float Dist = glm::length(V);

    if (r1 + r2 >= Dist) {
        if (r1 - r2 >= Dist) {
            Out = S1;
            return;
        }
        else if (r2 - r1 >= Dist) {
            Out = S2;
            return;
        }
    }

    glm::vec3 N = V / Dist;

    float t1 = glm::min(-r1, Dist - r2);
    float t2 = glm::max(r1, Dist + r2);
    float t_5 = (t2 - t1) * 0.5f;

    glm::vec3 NCenter = Center1 + (N * (t_5 + t1));

    Out.Center = NCenter;
    Out.Radius = t_5;
}

//-----------------------------------------------------------------------------
// Create sphere enscribing bounding box
//-----------------------------------------------------------------------------
void BoundingSphere::CreateFromBoundingBox(BoundingSphere& Out, const BoundingBox& box) noexcept {
    Out.Center = box.Center;
    glm::vec3 vExtents = box.Extents;
    Out.Radius = glm::length(vExtents);
}

void BoundingSphere::CreateFromBoundingBox(BoundingSphere& Out, const BoundingOrientedBox& box) noexcept {
    // Bounding box orientation is irrelevant because a sphere is rotationally invariant
    Out.Center = box.Center;
    glm::vec3 vExtents = box.Extents;
    Out.Radius = glm::length(vExtents);
}

//-----------------------------------------------------------------------------
// Find the approximate smallest enclosing bounding sphere for a set of
// points. Exact computation of the smallest enclosing bounding sphere is
// possible but is slower and requires a more complex algorithm.
// The algorithm is based on  Jack Ritter, "An Efficient Bounding Sphere",
// Graphics Gems.
//-----------------------------------------------------------------------------
void BoundingSphere::CreateFromPoints(BoundingSphere& Out, size_t Count, glm::vec3* pPoints, size_t Stride) noexcept {

    // Find the points with minimum and maximum x, y, and z
    glm::vec3 MinX, MaxX, MinY, MaxY, MinZ, MaxZ;

    MinX = MaxX = MinY = MaxY = MinZ = MaxZ = *pPoints;

    for (size_t i = 1u; i < Count; ++i) {
        glm::vec3 Point = glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

        float px = Point.x;
        float py = Point.y;
        float pz = Point.z;

        if (px < MinX.x) MinX = Point;
        if (px > MaxX.x) MaxX = Point;
        if (py < MinY.y) MinY = Point;
        if (py > MaxY.y) MaxY = Point;
        if (pz < MinZ.z) MinZ = Point;
        if (pz > MaxZ.z) MaxZ = Point;
    }

    // Use the min/max pair that are farthest apart to form the initial sphere.
    glm::vec3 DeltaX = MaxX - MinX;
    float DistX = glm::length(DeltaX);

    glm::vec3 DeltaY = MaxY - MinY;
    float DistY = glm::length(DeltaY);

    glm::vec3 DeltaZ = MaxZ - MinZ;
    float DistZ = glm::length(DeltaZ);

    glm::vec3 vCenter;
    float vRadius;

    if (DistX > DistY) {
        if (DistX > DistZ) {
            // Use min/max x.
            vCenter = MaxX + ((MinX - MaxX) * 0.5f);
            vRadius = DistX * 0.5f;
        }
        else {
            // Use min/max z.
            vCenter = MaxZ + ((MinZ - MaxZ) * 0.5f);
            vRadius = DistZ * 0.5f;
        }
    }
    else {
        // Y >= X
        if (DistY > DistZ) {
            // Use min/max y.
            vCenter = MaxY + ((MinY - MaxY) * 0.5f);
            vRadius = DistY * 0.5f;
        }
        else {
            // Use min/max z.
            vCenter = MaxZ + ((MinZ - MaxZ) * 0.5f);
            vRadius = DistZ * 0.5f;
        }
    }

    // Add any points not inside the sphere.
    for (size_t i = 0u; i < Count; ++i) {
        glm::vec3 Point = glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

        glm::vec3 Delta = Point - vCenter;

        float Dist = glm::length(Delta);

        if (Dist > vRadius) {
            // Adjust sphere to include the new point.
            vRadius = (vRadius + Dist) * 0.5f;
            vCenter = vCenter + (1.0f - (vRadius / Dist) * Delta);
        }
    }

    Out.Center = vCenter;
    Out.Radius = vRadius;
}


//-----------------------------------------------------------------------------
// Create sphere containing frustum
//-----------------------------------------------------------------------------
void BoundingSphere::CreateFromFrustum(BoundingSphere& Out, const BoundingFrustum& fr) noexcept {
    glm::vec3 Corners[BoundingFrustum::CORNER_COUNT];
    fr.GetCorners(Corners);
    CreateFromPoints(Out, BoundingFrustum::CORNER_COUNT, Corners, sizeof(glm::vec3));
}