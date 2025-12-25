#include "bounding_oriented_box.h"

#include "triangle_tests.h"
#include "bounding_sphere.h"
#include "bounding_box.h"
#include "bounding_frustum.h"

 //-----------------------------------------------------------------------------
 // Transform an oriented box by an angle preserving transform.
 //-----------------------------------------------------------------------------
void BoundingOrientedBox::Transform(BoundingOrientedBox& Out, const glm::mat4x4& M) const noexcept {
    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    // Composite the box rotation and the transform rotation.
    glm::mat4x4 nM;
    nM[0] = glm::normalize(M[0]);
    nM[1] = glm::normalize(M[1]);
    nM[2] = glm::normalize(M[2]);
    nM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::quat Rotation = glm::quat(nM);
    vOrientation = Rotation * vOrientation;

    // Transform the center.
    vCenter = M * glm::vec4(Center, 0.0f);

    // Scale the box extents.
    float dX = glm::length(M[0]);
    float dY = glm::length(M[1]);
    float dZ = glm::length(M[2]);

    glm::vec3 VectorScale = glm::vec3(dY, dX, dZ);
    vExtents = vExtents * VectorScale;

    // Store the box.
    Out.Center = vCenter;
    Out.Extents = vExtents;
    Out.Orientation = vOrientation;
}

void BoundingOrientedBox::Transform(BoundingOrientedBox& Out, float Scale, const glm::quat& Rotation, const glm::vec3& Translation) const noexcept {

    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    // Composite the box rotation and the transform rotation.
    vOrientation = vOrientation * Rotation;

    // Transform the center.
    glm::vec3 VectorScale = glm::vec3(Scale);
    vCenter = (Rotation * (vCenter * VectorScale)) + Translation;

    // Scale the box extents.
    vExtents = vExtents * VectorScale;

    // Store the box.
    Out.Center = vCenter;
    Out.Extents = vExtents;
    Out.Orientation = vOrientation;
}

//-----------------------------------------------------------------------------
// Get the corner points of the box
//-----------------------------------------------------------------------------
void BoundingOrientedBox::GetCorners(glm::vec3* Corners) const noexcept {

    // Load the box
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    for (size_t i = 0u; i < CORNER_COUNT; ++i) {
        glm::vec3 C = (vOrientation * (vExtents * glm::vec3(g_BoxOffset[i]))) + vCenter;
        Corners[i] = C;
    }
}

//-----------------------------------------------------------------------------
// Point in oriented box test.
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const glm::vec3& Point) const noexcept
{
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    // Transform the point to be local to the box.
    glm::vec3 TPoint = glm::inverse(vOrientation) * (Point - vCenter);

    return Vector3InBounds(TPoint, vExtents) ? CONTAINS : DISJOINT;
}

//-----------------------------------------------------------------------------
// Triangle in oriented bounding box
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    // Load the box center & orientation.
    glm::vec3 vCenter = Center;
    glm::quat vOrientation = Orientation;
    glm::quat invOrientation = glm::inverse(vOrientation);

    // Transform the triangle vertices into the space of the box.
    glm::vec3 TV0 = invOrientation * (V0 - vCenter);
    glm::vec3 TV1 = invOrientation * (V1 - vCenter);
    glm::vec3 TV2 = invOrientation * (V2 - vCenter);

    BoundingBox box;
    box.Center = glm::vec3(0.0f, 0.0f, 0.0f);
    box.Extents = Extents;

    // Use the triangle vs axis aligned box intersection routine.
    return box.Contains(TV0, TV1, TV2);
}

//-----------------------------------------------------------------------------
// Sphere in oriented bounding box
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const BoundingSphere& sh) const noexcept {
    glm::vec3 SphereCenter = sh.Center;
    float SphereRadius = sh.Radius;

    glm::vec3 BoxCenter = Center;
    glm::vec3 BoxExtents = Extents;
    glm::quat BoxOrientation = Orientation;

    // Transform the center of the sphere to be local to the box.
    // BoxMin = -BoxExtents
    // BoxMax = +BoxExtents
    SphereCenter = glm::inverse(BoxOrientation) * (SphereCenter - BoxCenter);

    // Find the distance to the nearest point on the box.
    // for each i in (x, y, z)
    // if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    // else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    glm::vec3 d = glm::vec3(0.0f);

    // Compute d for each dimension.
    glm::bvec3 LessThanMin = glm::lessThan(SphereCenter, -BoxExtents);
    glm::bvec3 GreaterThanMax = glm::greaterThan(SphereCenter, BoxExtents);

    glm::vec3 MinDelta = SphereCenter + BoxExtents;
    glm::vec3 MaxDelta = SphereCenter - BoxExtents;

    // Choose value for each dimension based on the comparison.
    d = VectorSelect(d, MinDelta, LessThanMin);
    d = VectorSelect(d, MaxDelta, GreaterThanMax);

    // Use a dot-product to square them and sum them together.
    float d2 = glm::dot(d, d);
    float SphereRadiusSq = SphereRadius * SphereRadius;

    if (d2 > SphereRadiusSq) {
        return DISJOINT;
    }

    // See if we are completely inside the box
    glm::vec3 SMin = SphereCenter - SphereRadius;
    glm::vec3 SMax = SphereCenter + SphereRadius;

    return (Vector3InBounds(SMin, BoxExtents) && Vector3InBounds(SMax, BoxExtents)) ? CONTAINS : INTERSECTS;
}

//-----------------------------------------------------------------------------
// Axis aligned box vs. oriented box. Constructs an oriented box and uses
// the oriented box vs. oriented box test.
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const BoundingBox& box) const noexcept {
    // Make the axis aligned box oriented and do an OBB vs OBB test.
    BoundingOrientedBox obox(box.Center, box.Extents, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    return Contains(obox);
}

//-----------------------------------------------------------------------------
// Oriented bounding box in oriented bounding box
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const BoundingOrientedBox& box) const noexcept {
    if (!Intersects(box)) {
        return DISJOINT;
    }

    // Load the boxes
    glm::vec3 aCenter = Center;
    glm::vec3 aExtents = Extents;
    glm::quat aOrientation = Orientation;

    glm::vec3 bCenter = box.Center;
    glm::vec3 bExtents = box.Extents;
    glm::quat bOrientation = box.Orientation;

    glm::vec3 offset = bCenter - aCenter;

    for (size_t i = 0u; i < CORNER_COUNT; ++i) {
        // Cb = rotate( bExtents * corneroffset[i], bOrientation ) + bcenter
        // Ca = invrotate( Cb - aCenter, aOrientation )

        glm::vec3 C = (bOrientation * (bExtents * glm::vec3(g_BoxOffset[i]))) + offset;
        C = glm::inverse(aOrientation) * C;

        if (!Vector3InBounds(C, aExtents)) {
            return INTERSECTS;
        }
    }

    return CONTAINS;
}

//-----------------------------------------------------------------------------
// Frustum in oriented bounding box
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::Contains(const BoundingFrustum& fr) const noexcept {
    if (!fr.Intersects(*this)) {
        return DISJOINT;
    }

    glm::vec3 Corners[BoundingFrustum::CORNER_COUNT];
    fr.GetCorners(Corners);

    // Load the box
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    for (size_t i = 0u; i < BoundingFrustum::CORNER_COUNT; ++i) {
        glm::vec3 C = glm::inverse(vOrientation) * (Corners[i] - vCenter);

        if (!Vector3InBounds(C, vExtents)) {
            return INTERSECTS;
        }
    }

    return CONTAINS;
}

//-----------------------------------------------------------------------------
// Sphere vs. oriented box test
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const BoundingSphere& sh) const noexcept {
    glm::vec3 SphereCenter = sh.Center;
    float SphereRadius = sh.Radius;

    glm::vec3 BoxCenter = Center;
    glm::vec3 BoxExtents = Extents;
    glm::quat BoxOrientation = Orientation;

    // Transform the center of the sphere to be local to the box.
    // BoxMin = -BoxExtents
    // BoxMax = +BoxExtents
    SphereCenter = glm::inverse(BoxOrientation) * (SphereCenter - BoxCenter);

    // Find the distance to the nearest point on the box.
    // for each i in (x, y, z)
    // if (SphereCenter(i) < BoxMin(i)) d2 += (SphereCenter(i) - BoxMin(i)) ^ 2
    // else if (SphereCenter(i) > BoxMax(i)) d2 += (SphereCenter(i) - BoxMax(i)) ^ 2

    glm::vec3 d = glm::vec3(0.0f);

    // Compute d for each dimension.
    glm::bvec3 LessThanMin = glm::lessThan(SphereCenter, -BoxExtents);
    glm::bvec3 GreaterThanMax = glm::greaterThan(SphereCenter, BoxExtents);

    glm::vec3 MinDelta = SphereCenter + BoxExtents;
    glm::vec3 MaxDelta = SphereCenter - BoxExtents;

    // Choose value for each dimension based on the comparison.
    d = VectorSelect(d, MinDelta, LessThanMin);
    d = VectorSelect(d, MaxDelta, GreaterThanMax);

    // Use a dot-product to square them and sum them together.
    float d2 = glm::dot(d, d);

    return (d2 <= (SphereRadius * SphereRadius)) ? true : false;
}

//-----------------------------------------------------------------------------
// Axis aligned box vs. oriented box. Constructs an oriented box and uses
// the oriented box vs. oriented box test.
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const BoundingBox& box) const noexcept {
    // Make the axis aligned box oriented and do an OBB vs OBB test.
    BoundingOrientedBox obox(box.Center, box.Extents, glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    return Intersects(obox);
}

//-----------------------------------------------------------------------------
// Fast oriented box / oriented box intersection test using the separating axis
// theorem.
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const BoundingOrientedBox& box) const noexcept {
    // Build the 3x3 rotation matrix that defines the orientation of B relative to A.
    glm::quat A_quat = Orientation;
    glm::quat B_quat = box.Orientation;

    glm::quat Q = A_quat * glm::conjugate(B_quat);
    glm::mat3x3 R = glm::mat3x3(Q);

    // Compute the translation of B relative to A.
    glm::vec3 A_cent = Center;
    glm::vec3 B_cent = box.Center;
    glm::vec3 t = glm::inverse(A_quat) * (B_cent - A_cent);

    //
    // h(A) = extents of A.
    // h(B) = extents of B.
    //
    // a(u) = axes of A = (1,0,0), (0,1,0), (0,0,1)
    // b(u) = axes of B relative to A = (r00,r10,r20), (r01,r11,r21), (r02,r12,r22)
    //
    // For each possible separating axis l:
    //   d(A) = sum (for i = u,v,w) h(A)(i) * abs( a(i) dot l )
    //   d(B) = sum (for i = u,v,w) h(B)(i) * abs( b(i) dot l )
    //   if abs( t dot l ) > d(A) + d(B) then disjoint
    //

    // Load extents of A and B.
    glm::vec3 h_A = Extents;
    glm::vec3 h_B = box.Extents;

    // Rows. Note R[0,1,2]X.w = 0.
    glm::vec3 R0X = R[0];
    glm::vec3 R1X = R[1];
    glm::vec3 R2X = R[2];

    R = glm::transpose(R);

    // Columns. Note RX[0,1,2].w = 0.
    glm::vec3 RX0 = R[0];
    glm::vec3 RX1 = R[1];
    glm::vec3 RX2 = R[2];

    // Absolute value of rows.
    glm::vec3 AR0X = glm::abs(R0X);
    glm::vec3 AR1X = glm::abs(R1X);
    glm::vec3 AR2X = glm::abs(R2X);

    // Absolute value of columns.
    glm::vec3 ARX0 = glm::abs(RX0);
    glm::vec3 ARX1 = glm::abs(RX1);
    glm::vec3 ARX2 = glm::abs(RX2);

    // Test each of the 15 possible seperating axii.
    float d, d_A, d_B;

    // l = a(u) = (1, 0, 0)
    // t dot l = t.x
    // d(A) = h(A).x
    // d(B) = h(B) dot abs(r00, r01, r02)
    d = t.x;
    d_A = h_A.x;
    d_B = glm::dot(h_B, AR0X);
    bool NoIntersection = glm::abs(d) > (d_A + d_B);

    // l = a(v) = (0, 1, 0)
    // t dot l = t.y
    // d(A) = h(A).y
    // d(B) = h(B) dot abs(r10, r11, r12)
    d = t.y;
    d_A = h_A.y;
    d_B = glm::dot(h_B, AR1X);
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(w) = (0, 0, 1)
    // t dot l = t.z
    // d(A) = h(A).z
    // d(B) = h(B) dot abs(r20, r21, r22)
    d = t.z;
    d_A = h_A.z;
    d_B = glm::dot(h_B, AR2X);
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = b(u) = (r00, r10, r20)
    // d(A) = h(A) dot abs(r00, r10, r20)
    // d(B) = h(B).x
    d = glm::dot(t, RX0);
    d_A = glm::dot(h_A, ARX0);
    d_B = h_B.x;
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = b(v) = (r01, r11, r21)
    // d(A) = h(A) dot abs(r01, r11, r21)
    // d(B) = h(B).y
    d = glm::dot(t, RX1);
    d_A = glm::dot(h_A, ARX1);
    d_B = h_B.y;
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = b(w) = (r02, r12, r22)
    // d(A) = h(A) dot abs(r02, r12, r22)
    // d(B) = h(B).z
    d = glm::dot(t, RX2);
    d_A = glm::dot(h_A, ARX2);
    d_B = h_B.z;
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(u) x b(u) = (0, -r20, r10)
    // d(A) = h(A) dot abs(0, r20, r10)
    // d(B) = h(B) dot abs(0, r02, r01)
    d = glm::dot(t, glm::vec3(0.0f, -RX0.z, RX0.y));
    d_A = glm::dot(h_A, glm::vec3(0.0f, ARX0.z, ARX0.y));
    d_B = glm::dot(h_B, glm::vec3(0.0f, AR0X.z, AR0X.y));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(u) x b(v) = (0, -r21, r11)
    // d(A) = h(A) dot abs(0, r21, r11)
    // d(B) = h(B) dot abs(r02, 0, r00)
    d = glm::dot(t, glm::vec3(0.0f, -RX1.z, RX1.y));
    d_A = glm::dot(h_A, glm::vec3(0.0f, ARX1.z, ARX1.y));
    d_B = glm::dot(h_B, glm::vec3(AR0X.z, 0.0f, AR0X.x));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(u) x b(w) = (0, -r22, r12)
    // d(A) = h(A) dot abs(0, r22, r12)
    // d(B) = h(B) dot abs(r01, r00, 0)
    d = glm::dot(t, glm::vec3(0.0f, -RX2.z, RX2.y));
    d_A = glm::dot(h_A, glm::vec3(0.0f, ARX2.z, ARX2.y));
    d_B = glm::dot(h_B, glm::vec3(AR0X.y, AR0X.x, 0.0f));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(v) x b(u) = (r20, 0, -r00)
    // d(A) = h(A) dot abs(r20, 0, r00)
    // d(B) = h(B) dot abs(0, r12, r11)
    d = glm::dot(t, glm::vec3(RX0.z, 0.0f, -RX0.x));
    d_A = glm::dot(h_A, glm::vec3(ARX0.z, 0.0f, ARX0.x));
    d_B = glm::dot(h_B, glm::vec3(0.0f, AR1X.z, AR1X.y));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(v) x b(v) = (r21, 0, -r01)
    // d(A) = h(A) dot abs(r21, 0, r01)
    // d(B) = h(B) dot abs(r12, 0, r10)
    d = glm::dot(t, glm::vec3(RX1.z, 0.0f, -RX1.x));
    d_A = glm::dot(h_A, glm::vec3(ARX1.z, 0.0f, ARX1.x));
    d_B = glm::dot(h_B, glm::vec3(AR1X.z, 0.0f, AR1X.x));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(v) x b(w) = (r22, 0, -r02)
    // d(A) = h(A) dot abs(r22, 0, r02)
    // d(B) = h(B) dot abs(r11, r10, 0)
    d = glm::dot(t, glm::vec3(RX2.z, 0.0f, -RX2.x));
    d_A = glm::dot(h_A, glm::vec3(ARX2.z, 0.0f, ARX2.x));
    d_B = glm::dot(h_B, glm::vec3(AR1X.y, AR1X.x, 0.0f));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(w) x b(u) = (-r10, r00, 0)
    // d(A) = h(A) dot abs(r10, r00, 0)
    // d(B) = h(B) dot abs(0, r22, r21)
    d = glm::dot(t, glm::vec3(-RX0.y, RX0.x, 0.0f));
    d_A = glm::dot(h_A, glm::vec3(ARX0.y, ARX0.x, 0.0f));
    d_B = glm::dot(h_B, glm::vec3(0.0f, AR2X.z, AR2X.y));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(w) x b(v) = (-r11, r01, 0)
    // d(A) = h(A) dot abs(r11, r01, 0)
    // d(B) = h(B) dot abs(r22, 0, r20)
    d = glm::dot(t, glm::vec3(-RX1.y, RX1.x, 0.0f));
    d_A = glm::dot(h_A, glm::vec3(ARX1.y, ARX1.x, 0.0f));
    d_B = glm::dot(h_B, glm::vec3(AR2X.z, 0.0f, AR2X.x));
    NoIntersection |= glm::abs(d) > (d_A + d_B);

    // l = a(w) x b(w) = (-r12, r02, 0)
    // d(A) = h(A) dot abs(r12, r02, 0)
    // d(B) = h(B) dot abs(r21, r20, 0)
    d = glm::dot(t, glm::vec3(-RX2.y, RX2.x, 0.0f));
    d_A = glm::dot(h_A, glm::vec3(ARX2.y, ARX2.x, 0.0f));
    d_B = glm::dot(h_B, glm::vec3(AR2X.y, AR2X.x, 0.0f));
    NoIntersection = glm::abs(d) > (d_A + d_B);

    // No seperating axis found, boxes must intersect.
    return NoIntersection;
}

//-----------------------------------------------------------------------------
// Frustum vs. oriented box test
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const BoundingFrustum& fr) const noexcept {
    return fr.Intersects(*this);
}

//-----------------------------------------------------------------------------
// Triangle vs. oriented box test.
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) const noexcept {
    // Load the box center & orientation.
    glm::vec3 vCenter = Center;
    glm::quat vOrientation = Orientation;
    glm::quat invOrientation = glm::inverse(vOrientation);

    // Transform the triangle vertices into the space of the box.
    glm::vec3 TV0 = invOrientation * (V0 - vCenter);
    glm::vec3 TV1 = invOrientation * (V1 - vCenter);
    glm::vec3 TV2 = invOrientation * (V2 - vCenter);

    BoundingBox box;
    box.Center = glm::vec3(0.0f, 0.0f, 0.0f);
    box.Extents = Extents;

    // Use the triangle vs axis aligned box intersection routine.
    return box.Intersects(TV0, TV1, TV2);
}

//-----------------------------------------------------------------------------
PlaneIntersectionType BoundingOrientedBox::Intersects(const glm::vec4& Plane) const noexcept {
    
    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat BoxOrientation = Orientation;
    // Set w of the center to one so we can dot4 with a plane.
    
    // Build the 3x3 rotation matrix that defines the box axes.
    glm::mat3x3 R = glm::mat3x3(BoxOrientation);

    bool Outside, Inside;
    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane, Outside, Inside);

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
// Compute the intersection of a ray (Origin, Direction) with an oriented box
// using the slabs method.
//-----------------------------------------------------------------------------
bool BoundingOrientedBox::Intersects(const glm::vec3& Origin, const glm::vec3& Direction, float& Dist) const noexcept {
    static const glm::dvec3 SelectY = glm::dvec3(false, true, false);
    static const glm::dvec3 SelectZ = glm::dvec3(false, false, true);;

    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat vOrientation = Orientation;

    // Get the boxes normalized side directions.
    glm::mat3x3 R = glm::mat3x3(vOrientation);

    // Adjust ray origin to be relative to center of the box.
    glm::vec3 TOrigin = vCenter - Origin;

    // Compute the dot product againt each axis of the box.
    glm::vec3 AxisDotOrigin;
    AxisDotOrigin.x = glm::dot(R[0], TOrigin);
    AxisDotOrigin.y = glm::dot(R[1], TOrigin);
    AxisDotOrigin.z = glm::dot(R[2], TOrigin);

    glm::vec3 AxisDotDirection;
    AxisDotDirection.x = glm::dot(R[0], Direction);
    AxisDotDirection.y = glm::dot(R[1], Direction);
    AxisDotDirection.z = glm::dot(R[2], Direction);

    // if (fabs(AxisDotDirection) <= Epsilon) the ray is nearly parallel to the slab.
    glm::bvec3 IsParallel = glm::lessThanEqual(glm::abs(AxisDotDirection), glm::vec3(g_RayEpsilon));

    // Test against all three axes simultaneously.
    glm::vec3 InverseAxisDotDirection = 1.0f / AxisDotDirection;
    glm::vec3 t1 = (AxisDotOrigin - vExtents) * InverseAxisDotDirection;
    glm::vec3 t2 = (AxisDotOrigin + vExtents) * InverseAxisDotDirection;

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
    bool NoIntersection = t_min.x > t_max.x;

    // if ( t_max < 0.0f ) return false;
    NoIntersection |= t_max.x < 0.0f;

    // if (IsParallel && (-Extents > AxisDotOrigin || Extents < AxisDotOrigin)) return false;
    glm::bvec3 ParallelOverlap = VectorInBounds(AxisDotOrigin, vExtents);
    NoIntersection |= !glm::any(IsParallel && glm::not_(ParallelOverlap));

    if (!NoIntersection) {
        // Store the x-component to *pDist
        Dist = t_min.x;
        return true;
    }

    Dist = 0.f;
    return false;
}

//-----------------------------------------------------------------------------
// Test an oriented box vs 6 planes (typically forming a frustum).
//-----------------------------------------------------------------------------
ContainmentType BoundingOrientedBox::ContainedBy(const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) const noexcept {
    // Load the box.
    glm::vec3 vCenter = Center;
    glm::vec3 vExtents = Extents;
    glm::quat BoxOrientation = Orientation;
    // Set w of the center to one so we can dot4 with a plane.

    // Build the 3x3 rotation matrix that defines the box axes.
    glm::mat3x3 R = glm::mat3x3(BoxOrientation);

    bool Outside, Inside;

    // Test against each plane.
    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane0, Outside, Inside);

    bool AnyOutside = Outside;
    bool AllInside = Inside;

    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane1, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane2, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane3, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane4, Outside, Inside);
    AnyOutside |= Outside;
    AllInside &= Inside;

    IntersectOrientedBoxPlane(vCenter, vExtents, R[0], R[1], R[2], Plane5, Outside, Inside);
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
// Create oriented bounding box from axis-aligned bounding box
//-----------------------------------------------------------------------------
void BoundingOrientedBox::CreateFromBoundingBox(BoundingOrientedBox& Out, const BoundingBox& box) noexcept {
    Out.Center = box.Center;
    Out.Extents = box.Extents;
    Out.Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}


//-----------------------------------------------------------------------------
// Find the approximate minimum oriented bounding box containing a set of
// points.  Exact computation of minimum oriented bounding box is possible but
// is slower and requires a more complex algorithm.
// The algorithm works by computing the inertia tensor of the points and then
// using the eigenvectors of the intertia tensor as the axes of the box.
// Computing the intertia tensor of the convex hull of the points will usually
// result in better bounding box but the computation is more complex.
// Exact computation of the minimum oriented bounding box is possible but the
// best know algorithm is O(N^3) and is significanly more complex to implement.
//-----------------------------------------------------------------------------
void BoundingOrientedBox::CreateFromPoints(BoundingOrientedBox& Out, size_t Count, const glm::vec3* pPoints, size_t Stride) noexcept {

    glm::vec3 CenterOfMass = glm::vec3(0.0f);

    // Compute the center of mass and inertia tensor of the points.
    for (size_t i = 0u; i < Count; ++i) {
        glm::vec3 Point = glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

        CenterOfMass = CenterOfMass + Point;
    }

    CenterOfMass = CenterOfMass * (1.0f / (glm::vec3(float(Count))));

    // Compute the inertia tensor of the points around the center of mass.
    // Using the center of mass is not strictly necessary, but will hopefully
    // improve the stability of finding the eigenvectors.
    glm::vec3 XX_YY_ZZ = glm::vec3(0.0f);
    glm::vec3 XY_XZ_YZ = glm::vec3(0.0f);

    for (size_t i = 0u; i < Count; ++i) {
        glm::vec3 Point = glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride)) - CenterOfMass;

        XX_YY_ZZ = XX_YY_ZZ + (Point * Point);

        glm::vec3 XXY = glm::vec3(Point.x, Point.x, Point.y);
        glm::vec3 YZZ = glm::vec3(Point.y, Point.z, Point.z);

        XY_XZ_YZ = XY_XZ_YZ + (XXY * YZZ);
    }

    glm::vec3 v1, v2, v3;

    // Compute the eigenvectors of the inertia tensor.
    CalculateEigenVectorsFromCovarianceMatrix(
        XX_YY_ZZ.x,
        XX_YY_ZZ.y,
        XX_YY_ZZ.z,
        XY_XZ_YZ.x,
        XY_XZ_YZ.y,
        XY_XZ_YZ.z,
        &v1, &v2, &v3
    );

    // Put them in a matrix.
    glm::mat3x3 R;

    R[0] = v1;
    R[1] = v2;
    R[2] = v3;

    // Multiply by -1 to convert the matrix into a right handed coordinate
    // system (Det ~= 1) in case the eigenvectors form a left handed
    // coordinate system (Det ~= -1) because XMQuaternionRotationMatrix only
    // works on right handed matrices.
    float Det = glm::determinant(R);

    if (Det < 0.0f) {
        R[0] *= -1.0f;
        R[1] *= -1.0f;
        R[2] *= -1.0f;
    }

    // Get the rotation quaternion from the matrix.
    glm::quat vOrientation = glm::quat(R);

    // Make sure it is normal (in case the vectors are slightly non-orthogonal).
    vOrientation = glm::normalize(vOrientation);

    // Rebuild the rotation matrix from the quaternion.
    R = glm::mat3x3(vOrientation);

    // Build the rotation into the rotated space.
    glm::mat3x3 InverseR = glm::transpose(R);

    // Find the minimum OBB using the eigenvectors as the axes.
    glm::vec3 vMin;
    glm::vec3 vMax;

    vMin = vMax = InverseR * (*pPoints);

    for (size_t i = 1u; i < Count; ++i) {
        glm::vec3 Point = InverseR * glm::vec3(*reinterpret_cast<const glm::vec3*>(reinterpret_cast<const uint8_t*>(pPoints) + i * Stride));

        vMin = glm::min(vMin, Point);
        vMax = glm::max(vMax, Point);
    }

    // Rotate the center into world space.
    glm::vec3 vCenter = (vMin + vMax) * 0.5f;
    vCenter = R * vCenter;

    // Store center, extents, and orientation.
    Out.Center = vCenter;
    Out.Extents = (vMax - vMin) * 0.5f;
    Out.Orientation = vOrientation;
}