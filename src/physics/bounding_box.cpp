#include "bounding_box.h"

#include "triangle_tests.h"
#include "bounding_sphere.h"
#include "bounding_frustum.h"
#include "bounding_oriented_box.h"

 //-----------------------------------------------------------------------------
 // Transform an axis aligned box by an angle preserving transform.
 //-----------------------------------------------------------------------------
void BoundingBox::Transform(BoundingBox& Out, const glm::mat4x4& M) const noexcept {

    // Compute and transform the corners and find new min/max bounds.
    glm::vec4 Corner = (glm::vec4(Extents, 0.0f) * g_BoxOffset[0]) + glm::vec4(Center, 0.0f);
    Corner = M * Corner;

    glm::vec4 Min, Max;
    Min = Max = Corner;

    for (size_t i = 1u; i < CORNER_COUNT; ++i) {
        Corner = (glm::vec4(Extents, 0.0f) * g_BoxOffset[i]) + glm::vec4(Center, 0.0f);
        Corner = M * Corner;

        Min = glm::min(Min, Corner);
        Max = glm::max(Max, Corner);
    }

    // Store center and extents.
    Out.Center = (Min + Max) * 0.5f;
    Out.Extents = (Max - Min) * 0.5f;
}

void BoundingBox::Transform(BoundingBox& Out, float Scale, const glm::quat& Rotation, const glm::vec3& Translation) const noexcept {

    // Compute and transform the corners and find new min/max bounds.
    glm::vec4 Corner = (glm::vec4(Extents, 0.0f) * g_BoxOffset[0]) + glm::vec4(Center, 0.0f);
    Corner = ((Corner * Scale) * Rotation) + glm::vec4(Translation, 0.0f);

    glm::vec4 Min, Max;
    Min = Max = Corner;

    for (size_t i = 1u; i < CORNER_COUNT; ++i) {
        Corner = (glm::vec4(Extents, 0.0f) * g_BoxOffset[i]) + glm::vec4(Center, 0.0f);
        Corner = ((Corner * Scale) * Rotation) + glm::vec4(Translation, 0.0f);

        Min = glm::min(Min, Corner);
        Max = glm::max(Max, Corner);
    }

    // Store center and extents.
    Out.Center = (Min + Max) * 0.5f;
    Out.Extents = (Max - Min) * 0.5f;
}

//-----------------------------------------------------------------------------
// Get the corner points of the box
//-----------------------------------------------------------------------------
void BoundingBox::GetCorners(glm::vec3* Corners) const noexcept {
    for (size_t i = 0u; i < CORNER_COUNT; ++i) {
        glm::vec4 C = (glm::vec4(Extents, 0.0f) * g_BoxOffset[i]) + glm::vec4(Center, 0.0f);
        Corners[i] = C;
    }
}

//-----------------------------------------------------------------------------
// Point in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const glm::vec3& Point) const noexcept {
    return Vector3InBounds(Point - Center, Extents) ? CONTAINS : DISJOINT;
}

//-----------------------------------------------------------------------------
// Triangle in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    if (!Intersects(V0, V1, V2)) {
        return DISJOINT;
    }

    glm::vec3 d = glm::abs(V0 - Center);
    bool Inside = d.x <= Extents.x || d.y <= Extents.y || d.z <= Extents.z;

    d = glm::abs(V1 - Center);
    Inside &= d.x <= Extents.x || d.y <= Extents.y || d.z <= Extents.z;

    d = glm::abs(V2 - Center);
    Inside &= d.x <= Extents.x || d.y <= Extents.y || d.z <= Extents.z;

    return Inside ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Sphere in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const BoundingSphere& sh) const noexcept {
    glm::vec3 SphereCenter = sh.Center;
    glm::vec3 SphereRadius = glm::vec3(sh.Radius);

    glm::vec3 BoxCenter = Center;
    glm::vec3 BoxExtents = Extents;

    glm::vec3 BoxMin = BoxCenter - BoxExtents;
    glm::vec3 BoxMax = BoxCenter + BoxExtents;

    // Find the distance to the nearest point on the box.
    // for each i in (x, y, z)
    // if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    // else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    glm::vec3 d = glm::vec3(0.0f);

    // Compute d for each dimension.
    glm::bvec3 LessThanMin = glm::lessThan(SphereCenter, BoxMin);
    glm::bvec3 GreaterThanMax = glm::greaterThan(SphereCenter, BoxMax);

    glm::vec3 MinDelta = SphereCenter - BoxMin;
    glm::vec3 MaxDelta = SphereCenter - BoxMax;

    // Choose value for each dimension based on the comparison.
    d.x = LessThanMin.x ? MinDelta.x : d.x;
    d.y = LessThanMin.x ? MinDelta.y : d.y;
    d.z = LessThanMin.z ? MinDelta.z : d.z;
    d = VectorSelect(d, MaxDelta, GreaterThanMax);

    // Use a dot-product to square them and sum them together.
    float d2 = glm::dot(d, d);

    if (d2 > (SphereRadius.x * SphereRadius.x)) {
        return DISJOINT;
    }

    glm::bvec3 InsideAll = glm::lessThanEqual(BoxMin + SphereRadius, SphereCenter);
    InsideAll &= glm::lessThanEqual(SphereCenter, BoxMax - SphereRadius);
    InsideAll &= glm::greaterThan(BoxMax - BoxMin, SphereRadius);

    return glm::all(InsideAll) ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Axis-aligned box in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const BoundingBox& box) const noexcept {
    glm::vec3 CenterA = Center;
    glm::vec3 ExtentsA = Extents;

    glm::vec3 CenterB = box.Center;
    glm::vec3 ExtentsB = box.Extents;

    glm::vec3 MinA = CenterA - ExtentsA;
    glm::vec3 MaxA = CenterA + ExtentsA;

    glm::vec3 MinB = CenterB - ExtentsB;
    glm::vec3 MaxB = CenterB + ExtentsB;

    // for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
    glm::bvec3 Disjoint = glm::greaterThan(MinA, MaxB) || glm::greaterThan(MinB, MaxA);

    if (glm::any(Disjoint)) {
        return DISJOINT;
    }

    // for each i in (x, y, z) if a_min(i) <= b_min(i) and b_max(i) <= a_max(i) then A contains B
    glm::bvec3 Inside = glm::lessThanEqual(MinA, MinB) && glm::lessThanEqual(MaxB, MaxA);

    return glm::all(Inside) ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Oriented box in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const BoundingOrientedBox& box) const noexcept {
    if (!box.Intersects(*this)) {
        return DISJOINT;
    }

    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;

    // Subtract off the AABB center to remove a subtract below
    glm::vec3 oCenter = box.Center - vCenter;

    glm::vec3 oExtents = box.Extents;
    glm::quat oOrientation = box.Orientation;

    bool Inside = true;

    for (size_t i = 0u; i < BoundingOrientedBox::CORNER_COUNT; ++i) {
        glm::vec3 C = (oExtents * glm::vec3(g_BoxOffset[i]) * oOrientation) + oCenter;
        glm::vec3 d = glm::abs(C);
        Inside &= glm::all(glm::lessThanEqual(d, vExtents));
    }

    return Inside ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Frustum in axis-aligned box test
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::Contains(const BoundingFrustum& fr) const noexcept {
    if (!fr.Intersects(*this)) {
        return DISJOINT;
    }

    glm::vec3 Corners[BoundingFrustum::CORNER_COUNT];
    fr.GetCorners(Corners);

    bool Inside = true;

    for (size_t i = 0u; i < BoundingFrustum::CORNER_COUNT; ++i) {
        glm::vec3 Point = Corners[i];
        glm::vec3 d = glm::abs(Point - Center);
        Inside &= glm::all(glm::lessThanEqual(d, Extents));
    }

    return Inside ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Sphere vs axis-aligned box test
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const BoundingSphere& sh) const noexcept {
    glm::vec3 SphereCenter = sh.Center;
    float SphereRadius = sh.Radius;

    glm::vec3 BoxCenter = Center;
    glm::vec3 BoxExtents = Extents;

    glm::vec3 BoxMin = BoxCenter - BoxExtents;
    glm::vec3 BoxMax = BoxCenter + BoxExtents;

    // Find the distance to the nearest point on the box.
    // for each i in (x, y, z)
    // if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    // else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    glm::vec3 d = glm::vec3(0.0f);

    // Compute d for each dimension.
    glm::bvec3 LessThanMin = glm::lessThan(SphereCenter, BoxMin);
    glm::bvec3 GreaterThanMax = glm::greaterThan(SphereCenter, BoxMax);

    glm::vec3 MinDelta = SphereCenter - BoxMin;
    glm::vec3 MaxDelta = SphereCenter - BoxMax;

    // Choose value for each dimension based on the comparison.
    d = VectorSelect(d, MinDelta, LessThanMin);
    d = VectorSelect(d, MaxDelta, GreaterThanMax);

    // Use a dot-product to square them and sum them together.
    float d2 = glm::dot(d, d);

    return d2 <= (SphereRadius * SphereRadius);
}

//-----------------------------------------------------------------------------
// Axis-aligned box vs. axis-aligned box test
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const BoundingBox& box) const noexcept {
    glm::vec3 CenterA = Center;
    glm::vec3 ExtentsA = Extents;

    glm::vec3 CenterB = box.Center;
    glm::vec3 ExtentsB = box.Extents;

    glm::vec3 MinA = CenterA - ExtentsA;
    glm::vec3 MaxA = CenterA + ExtentsA;

    glm::vec3 MinB = CenterB - ExtentsB;
    glm::vec3 MaxB = CenterB + ExtentsB;

    // for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then return false
    glm::bvec3 Disjoint = glm::greaterThan(MinA, MaxB) || glm::greaterThan(MinB, MaxA);

    return !glm::any(Disjoint);
}

//-----------------------------------------------------------------------------
// Oriented box vs. axis-aligned box test
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const BoundingOrientedBox& box) const noexcept {
    return box.Intersects(*this);
}

//-----------------------------------------------------------------------------
// Frustum vs. axis-aligned box test
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const BoundingFrustum& fr) const noexcept {
    return fr.Intersects(*this);
}

//-----------------------------------------------------------------------------
// Triangle vs. axis aligned box test
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    glm::vec3 Zero = glm::vec3(0.0f);

    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;

    glm::vec3 BoxMin = vCenter - vExtents;
    glm::vec3 BoxMax = vCenter + vExtents;

    // Test the axes of the box (in effect test the AAB against the minimal AAB
    // around the triangle).
    glm::vec3 TriMin = glm::min(glm::min(V0, V1), V2);
    glm::vec3 TriMax = glm::max(glm::max(V0, V1), V2);

    // for each i in (x, y, z) if a_min(i) > b_max(i) or b_min(i) > a_max(i) then disjoint
    glm::bvec3 Disjoint = glm::greaterThan(TriMin, BoxMax) || glm::greaterThan(BoxMin, TriMax);
    if (glm::any(Disjoint)) {
        return false;
    }

    // Test the plane of the triangle.
    glm::vec3 Normal = glm::cross(V1 - V0, V2 - V0);
    float Dist = glm::dot(Normal, V0);

    // for each i in (x, y, z) if n(i) >= 0 then v_min(i)=b_min(i), v_max(i)=b_max(i)
    // else v_min(i)=b_max(i), v_max(i)=b_min(i)
    glm::bvec3 NormalSelect = glm::greaterThan(Normal, Zero);
    glm::vec3 V_Min = VectorSelect(BoxMax, BoxMin, NormalSelect);
    glm::vec3 V_Max = VectorSelect(BoxMin, BoxMax, NormalSelect);

    // if n dot v_min + d > 0 || n dot v_max + d < 0 then disjoint
    float MinDist = glm::dot(V_Min, Normal);
    float MaxDist = glm::dot(V_Max, Normal);

    bool NoIntersection = MinDist > Dist;
    NoIntersection |= MaxDist < Dist;

    // Move the box center to zero to simplify the following tests.
    glm::vec3 TV0 = V0 - vCenter;
    glm::vec3 TV1 = V1 - vCenter;
    glm::vec3 TV2 = V2 - vCenter;

    // Test the edge/edge axes (3*3).
    glm::vec3 e0 = TV1 - TV0;
    glm::vec3 e1 = TV2 - TV1;
    glm::vec3 e2 = TV0 - TV2;
    // Make w zero.

    glm::vec3 Axis;
    float p0, p1, p2;
    float Min, Max;
    float Radius;

    // Axis == (1,0,0) x e0 = (0, -e0.z, e0.y)
    Axis = glm::vec3(0.0f, -e0.z, e0.y);
    p0 = glm::dot(TV0, Axis);
    // p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
    p2 = glm::dot(TV2, Axis);
    Min = glm::min(p0, p2);
    Max = glm::max(p0, p2);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (1,0,0) x e1 = (0, -e1.z, e1.y)
    Axis = glm::vec3(0.0f, -e1.z, e1.y);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (1,0,0) x e2 = (0, -e2.z, e2.y)
    Axis = glm::vec3(0.0f, -e2.z, e2.y);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,1,0) x e0 = (e0.z, 0, -e0.x)
    Axis = glm::vec3(e0.z, 0.0f, -e0.x);
    p0 = dot(TV0, Axis);
    // p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
    p2 = dot(TV2, Axis);
    Min = glm::min(p0, p2);
    Max = glm::max(p0, p2);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,1,0) x e1 = (e1.z, 0, -e1.x)
    Axis = glm::vec3(e1.z, 0.0f, -e1.x);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,0,1) x e2 = (e2.z, 0, -e2.x)
    Axis = glm::vec3(e2.z, 0.0f, -e1.x);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,0,1) x e0 = (-e0.y, e0.x, 0)
    Axis = glm::vec3(-e0.y, e0.x, 0.0f);
    p0 = glm::dot(TV0, Axis);
    // p1 = XMVector3Dot( V1, Axis ); // p1 = p0;
    p2 = glm::dot(TV2, Axis);
    Min = glm::min(p0, p2);
    Max = glm::max(p0, p2);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,0,1) x e1 = (-e1.y, e1.x, 0)
    Axis = glm::vec3(-e1.y, e1.x, 0.0f);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p1;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    // Axis == (0,0,1) x e2 = (-e2.y, e2.x, 0)
    Axis = glm::vec3(-e2.y, e2.x, 0.0f);
    p0 = glm::dot(TV0, Axis);
    p1 = glm::dot(TV1, Axis);
    // p2 = XMVector3Dot( V2, Axis ); // p2 = p0;
    Min = glm::min(p0, p1);
    Max = glm::max(p0, p1);
    Radius = glm::dot(vExtents, glm::abs(Axis));
    NoIntersection |= Min > Radius;
    NoIntersection |= Max < -Radius;

    return NoIntersection;
}

//-----------------------------------------------------------------------------
PlaneIntersectionType BoundingBox::Intersects(const glm::vec4& Plane) const noexcept {

    bool Outside, Inside;
    IntersectAxisAlignedBoxPlane(Center, Extents, Plane, Outside, Inside);

    // If the box is outside any plane it is outside.
    if (Outside) {
        return FRONT;
    }

    // If the box is inside all planes it is inside.
    if (Inside) {
        return BACK;
    }

    // The box is not inside all planes or outside a plane it intersects.
    return INTERSECTING;
}

//-----------------------------------------------------------------------------
// Compute the intersection of a ray (Origin, Direction) with an axis aligned
// box using the slabs method.
//-----------------------------------------------------------------------------
bool BoundingBox::Intersects(const glm::vec3& Origin, const glm::vec3& Direction, float& Dist) const noexcept {

    // Adjust ray origin to be relative to center of the box.
    glm::vec3 TOrigin = Center - Origin;

    // Compute the dot product againt each axis of the box.
    // Since the axii are (1,0,0), (0,1,0), (0,0,1) no computation is necessary.
    glm::vec3 AxisDotOrigin = TOrigin;
    glm::vec3 AxisDotDirection = Direction;

    // if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
    glm::bvec3 IsParallel = glm::lessThanEqual(glm::abs(AxisDotDirection),  glm::vec3(g_RayEpsilon));

    // Test against all three axii simultaneously.
    glm::vec3 InverseAxisDotDirection = (1.0f / AxisDotDirection);
    glm::vec3 t1 = (AxisDotOrigin - Extents) * InverseAxisDotDirection;
    glm::vec3 t2 = (AxisDotOrigin + Extents) * InverseAxisDotDirection;

    // Compute the max of min(t1,t2) and the min of max(t1,t2) ensuring we don't
    // use the results from any directions parallel to the slab.
    glm::vec3 t_min = VectorSelect(glm::min(t1, t2), glm::vec3(g_FltMin), IsParallel);
    glm::vec3 t_max = VectorSelect(glm::max(t1, t2), glm::vec3(g_FltMax), IsParallel);

    // t_min.x = maximum( t_min.x, t_min.y, t_min.z );
    // t_max.x = minimum( t_max.x, t_max.y, t_max.z );
    t_min = glm::max(t_min, glm::vec3(t_min.y));  // x = max(x,y)
    t_min = glm::max(t_min, glm::vec3(t_min.z));  // x = max(max(x,y),z)
    t_max = glm::min(t_max, glm::vec3(t_max.y));  // x = min(x,y)
    t_max = glm::min(t_max, glm::vec3(t_max.z));  // x = min(min(x,y),z)

    // if ( t_min > t_max ) return false;
    glm::bvec3 NoIntersection = glm::greaterThan(glm::vec3(t_min.x), glm::vec3(t_max.x));

    // if ( t_max < 0.0f ) return false;
    NoIntersection |= glm::lessThan(glm::vec3(t_max.x), glm::vec3(0.0f));

    // if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return false;
    glm::bvec3 ParallelOverlap = VectorInBounds(AxisDotOrigin, Extents);
    NoIntersection |= IsParallel && ParallelOverlap;

    if (!glm::any(NoIntersection)) {
        // Store the x-component to *pDist
        Dist = t_min.x;
        return true;
    }

    Dist = 0.f;
    return false;
}

//-----------------------------------------------------------------------------
// Test an axis alinged box vs 6 planes (typically forming a frustum).
//-----------------------------------------------------------------------------
ContainmentType BoundingBox::ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept {
    
    bool Outside, Inside;

    // Test against each plane.
    IntersectAxisAlignedBoxPlane(Center, Extents, Plane0, Outside, Inside);

    bool AnyOutside = Outside;
    bool AllInside = Inside;

    IntersectAxisAlignedBoxPlane(Center, Extents, Plane1, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectAxisAlignedBoxPlane(Center, Extents, Plane2, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectAxisAlignedBoxPlane(Center, Extents, Plane3, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectAxisAlignedBoxPlane(Center, Extents, Plane4, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectAxisAlignedBoxPlane(Center, Extents, Plane5, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    // If the box is outside any plane it is outside.
    if (AnyOutside) {
        return DISJOINT;
    }

    // If the box is inside all planes it is inside.
    if (AllInside) {
        return CONTAINS;
    }

    // The box is not inside all planes or outside a plane, it may intersect.
    return INTERSECTS;
}

//-----------------------------------------------------------------------------
// Create axis-aligned box that contains two other bounding boxes
//-----------------------------------------------------------------------------
void BoundingBox::CreateMerged(BoundingBox& Out, const BoundingBox& b1, const BoundingBox& b2) noexcept {
    glm::vec3 b1Center = b1.Center;
    glm::vec3 b1Extents = b1.Extents;

    glm::vec3 b2Center = b2.Center;
    glm::vec3 b2Extents = b2.Extents;

    glm::vec3 Min = b1Center - b1Extents;
    Min = glm::min(Min, b2Center - b2Extents);

    glm::vec3 Max = b1Center + b1Extents;
    Max = glm::max(Max, b2Center + b2Extents);

    Out.Center = (Min + Max) * 0.5f;
    Out.Extents = (Max - Min) * 0.5f;
}

//-----------------------------------------------------------------------------
// Create axis-aligned box that contains a bounding sphere
//-----------------------------------------------------------------------------
void BoundingBox::CreateFromSphere(BoundingBox& Out, const BoundingSphere& sh) noexcept {
    glm::vec3 spCenter = sh.Center;
    glm::vec3 shRadius = glm::vec3(sh.Radius);

    glm::vec3 Min = spCenter - shRadius;
    glm::vec3 Max = spCenter + shRadius;

    Out.Center = (Min + Max) * 0.5f;
    Out.Extents = (Max - Min) * 0.5f;
}

//-----------------------------------------------------------------------------
// Create axis-aligned box from min/max points
//-----------------------------------------------------------------------------
void BoundingBox::CreateFromPoints(BoundingBox& Out, const glm::vec3& pt1, const glm::vec3& pt2) noexcept {
    glm::vec3 Min = glm::min(pt1, pt2);
    glm::vec3 Max = glm::max(pt1, pt2);

    // Store center and extents.
    Out.Center = (Min + Max) * 0.5f;
    Out.Extents = (Max - Min) * 0.5f;
}

//-----------------------------------------------------------------------------
// Find the minimum axis aligned bounding box containing a set of points.
//-----------------------------------------------------------------------------
void BoundingBox::CreateFromPoints(BoundingBox& Out, size_t Count, glm::vec3* pPoints, size_t Stride) noexcept {

    // Find the minimum and maximum x, y, and z
    glm::vec3 vMin, vMax;

    vMin = vMax = *pPoints;

    for (size_t i = 1u; i < Count; ++i) {
        glm::vec3 Point = glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

        vMin = glm::min(vMin, Point);
        vMax = glm::max(vMax, Point);
    }

    // Store center and extents.
    Out.Center = (vMin + vMax) * 0.5f;
    Out.Extents = (vMax - vMin) * 0.5f;
}