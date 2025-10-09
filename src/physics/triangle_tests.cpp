#include "triangle_tests.h"

const glm::vec4 g_RayEpsilon = glm::vec4(1e-20f, 1e-20f, 1e-20f, 1e-20f);

glm::vec4 PlaneTransform(const glm::vec4& Plane, const glm::quat& Rotation, const glm::vec3& Translation) noexcept {
    glm::vec4 vNormal = glm::vec4(Plane.x, Plane.y, Plane.z, 0.0f) * Rotation;
    float vD = Plane.w - glm::dot(vNormal, glm::vec4(Translation, 1.0f));
    vNormal.w = vD;
    return vNormal;
}

glm::vec4 PlaneNormalize(const glm::vec4& P) noexcept {
    float fLengthSq = std::sqrtf((P.x * P.x) + (P.y * P.y) + (P.z * P.z));
    // Prevent divide by zero
    if (fLengthSq > 0.0f) {
        fLengthSq = 1.0f / fLengthSq;
    }
    glm::vec4 vResult (P.x * fLengthSq, P.y * fLengthSq, P.z * fLengthSq, P.w * fLengthSq);
    return vResult;
}

float PlaneDotCoord(const glm::vec4& P, glm::vec3 V) noexcept {
    // Result = P[0] * V[0] + P[1] * V[1] + P[2] * V[2] + P[3]

    glm::vec4 V3(V, 1.0f);
    return glm::dot(P, V3);
}

glm::vec3 PointOnLineSegmentNearestPoint(const glm::vec3& S1, const glm::vec3& S2, const glm::vec3& P) noexcept {
    glm::vec3 Dir = S2 - S1;
    float Projection = glm::dot(P, Dir) - glm::dot(S1, Dir);
    float LengthSq = glm::dot(Dir, Dir);

    float t = Projection * (1.0f / LengthSq);
    glm::vec3 Point = (t * Dir) + S1;

    // t < 0
    bool SelectS1 = Projection < 0.0f;
    Point = SelectS1 ? S1 : Point;

    // t > 1
    bool SelectS2 = Projection > LengthSq;
    Point = SelectS2 ? S2 : Point;

    return Point;
}

void IntersectSpherePlane(const glm::vec3& Center, float Radius, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept {
    float Dist = glm::dot(glm::vec4(Center, 1.0f), Plane);

    // Outside the plane?
    Outside = Dist > Radius;

    // Fully inside the plane?
    Inside = Dist < -Radius;
}

void IntersectFrustumPlane(const glm::vec4& Point0, const glm::vec4& Point1, const glm::vec4& Point2, const glm::vec4& Point3, const glm::vec4& Point4, const glm::vec4& Point5, const glm::vec4& Point6, const glm::vec4& Point7, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept {
    // Find the min/max projection of the frustum onto the plane normal.
    float Min;
    float Max;
    float Dist;

    Min = Max = glm::dot(Plane, Point0);

    Dist = glm::dot(Plane, Point1);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point2);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point3);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point4);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point5);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point6);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    Dist = glm::dot(Plane, Point7);
    Min = glm::min(Min, Dist);
    Max = glm::max(Max, Dist);

    float PlaneDist = Plane.w;

    // Outside the plane?
    Outside = Min > PlaneDist;

    // Fully inside the plane?
    Inside = Max < PlaneDist;
}

bool PointOnPlaneInsideTriangle(const glm::vec3& P, const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2) noexcept {
    // Compute the triangle normal.
    glm::vec3 N = glm::cross(V2 - V0, V1 - V0);

    // Compute the cross products of the vector from the base of each edge to
    // the point with each edge vector.
    glm::vec3 C0 = glm::cross(P - V0, V1 - V0);
    glm::vec3 C1 = glm::cross(P - V1, V2 - V1);
    glm::vec3 C2 = glm::cross(P - V2, V0 - V2);

    // If the cross product points in the same direction as the normal the the
    // point is inside the edge (it is zero if is on the edge).
    bool Inside0 = glm::dot(C0, N) >= 0.0f;
    bool Inside1 = glm::dot(C1, N) >= 0.0f;
    bool Inside2 = glm::dot(C2, N) >= 0.0f;

    // If the point inside all of the edges it is inside.
    return Inside0 && Inside1 && Inside2;
}

/****************************************************************************
 *
 * TriangleTests
 *
 ****************************************************************************/
namespace TriangleTests {

    //-----------------------------------------------------------------------------
    // Compute the intersection of a ray (Origin, Direction) with a triangle
    // (V0, V1, V2).  Return true if there is an intersection and also set *pDist
    // to the distance along the ray to the intersection.
    //
    // The algorithm is based on Moller, Tomas and Trumbore, "Fast, Minimum Storage
    // Ray-Triangle Intersection", Journal of Graphics Tools, vol. 2, no. 1,
    // pp 21-28, 1997.
    //-----------------------------------------------------------------------------
    _Use_decl_annotations_
        inline bool XM_CALLCONV Intersects(
            FXMVECTOR Origin, FXMVECTOR Direction, FXMVECTOR V0,
            GXMVECTOR V1,
            HXMVECTOR V2, float& Dist) noexcept
    {
        assert(DirectX::Internal::XMVector3IsUnit(Direction));

        XMVECTOR Zero = XMVectorZero();

        XMVECTOR e1 = XMVectorSubtract(V1, V0);
        XMVECTOR e2 = XMVectorSubtract(V2, V0);

        // p = Direction ^ e2;
        XMVECTOR p = XMVector3Cross(Direction, e2);

        // det = e1 * p;
        XMVECTOR det = XMVector3Dot(e1, p);

        XMVECTOR u, v, t;

        if (XMVector3GreaterOrEqual(det, g_RayEpsilon))
        {
            // Determinate is positive (front side of the triangle).
            XMVECTOR s = XMVectorSubtract(Origin, V0);

            // u = s * p;
            u = XMVector3Dot(s, p);

            XMVECTOR NoIntersection = XMVectorLess(u, Zero);
            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(u, det));

            // q = s ^ e1;
            XMVECTOR q = XMVector3Cross(s, e1);

            // v = Direction * q;
            v = XMVector3Dot(Direction, q);

            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(v, Zero));
            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(XMVectorAdd(u, v), det));

            // t = e2 * q;
            t = XMVector3Dot(e2, q);

            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(t, Zero));

            if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
            {
                Dist = 0.f;
                return false;
            }
        }
        else if (XMVector3LessOrEqual(det, g_RayNegEpsilon))
        {
            // Determinate is negative (back side of the triangle).
            XMVECTOR s = XMVectorSubtract(Origin, V0);

            // u = s * p;
            u = XMVector3Dot(s, p);

            XMVECTOR NoIntersection = XMVectorGreater(u, Zero);
            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(u, det));

            // q = s ^ e1;
            XMVECTOR q = XMVector3Cross(s, e1);

            // v = Direction * q;
            v = XMVector3Dot(Direction, q);

            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(v, Zero));
            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorLess(XMVectorAdd(u, v), det));

            // t = e2 * q;
            t = XMVector3Dot(e2, q);

            NoIntersection = XMVectorOrInt(NoIntersection, XMVectorGreater(t, Zero));

            if (XMVector4EqualInt(NoIntersection, XMVectorTrueInt()))
            {
                Dist = 0.f;
                return false;
            }
        }
        else
        {
            // Parallel ray.
            Dist = 0.f;
            return false;
        }

        t = XMVectorDivide(t, det);

        // (u / det) and (v / dev) are the barycentric cooridinates of the intersection.

        // Store the x-component to *pDist
        XMStoreFloat(&Dist, t);

        return true;
    }


    //-----------------------------------------------------------------------------
    // Test if two triangles intersect.
    //
    // The final test of algorithm is based on Shen, Heng, and Tang, "A Fast
    // Triangle-Triangle Overlap Test Using Signed Distances", Journal of Graphics
    // Tools, vol. 8, no. 1, pp 17-23, 2003 and Guigue and Devillers, "Fast and
    // Robust Triangle-Triangle Overlap Test Using Orientation Predicates", Journal
    // of Graphics Tools, vol. 8, no. 1, pp 25-32, 2003.
    //
    // The final test could be considered an edge-edge separating plane test with
    // the 9 possible cases narrowed down to the only two pairs of edges that can
    // actaully result in a seperation.
    //-----------------------------------------------------------------------------
    _Use_decl_annotations_
        inline bool XM_CALLCONV Intersects(FXMVECTOR A0, FXMVECTOR A1, FXMVECTOR A2, GXMVECTOR B0, HXMVECTOR B1, HXMVECTOR B2) noexcept
    {
        static const XMVECTORU32 SelectY = { { { XM_SELECT_0, XM_SELECT_1, XM_SELECT_0, XM_SELECT_0 } } };
        static const XMVECTORU32 SelectZ = { { { XM_SELECT_0, XM_SELECT_0, XM_SELECT_1, XM_SELECT_0 } } };
        static const XMVECTORU32 Select0111 = { { { XM_SELECT_0, XM_SELECT_1, XM_SELECT_1, XM_SELECT_1 } } };
        static const XMVECTORU32 Select1011 = { { { XM_SELECT_1, XM_SELECT_0, XM_SELECT_1, XM_SELECT_1 } } };
        static const XMVECTORU32 Select1101 = { { { XM_SELECT_1, XM_SELECT_1, XM_SELECT_0, XM_SELECT_1 } } };

        XMVECTOR Zero = XMVectorZero();

        // Compute the normal of triangle A.
        XMVECTOR N1 = XMVector3Cross(XMVectorSubtract(A1, A0), XMVectorSubtract(A2, A0));

        // Assert that the triangle is not degenerate.
        assert(!XMVector3Equal(N1, Zero));

        // Test points of B against the plane of A.
        XMVECTOR BDist = XMVector3Dot(N1, XMVectorSubtract(B0, A0));
        BDist = XMVectorSelect(BDist, XMVector3Dot(N1, XMVectorSubtract(B1, A0)), SelectY);
        BDist = XMVectorSelect(BDist, XMVector3Dot(N1, XMVectorSubtract(B2, A0)), SelectZ);

        // Ensure robustness with co-planar triangles by zeroing small distances.
        uint32_t BDistIsZeroCR;
        XMVECTOR BDistIsZero = XMVectorGreaterR(&BDistIsZeroCR, g_RayEpsilon, XMVectorAbs(BDist));
        BDist = XMVectorSelect(BDist, Zero, BDistIsZero);

        uint32_t BDistIsLessCR;
        XMVECTOR BDistIsLess = XMVectorGreaterR(&BDistIsLessCR, Zero, BDist);

        uint32_t BDistIsGreaterCR;
        XMVECTOR BDistIsGreater = XMVectorGreaterR(&BDistIsGreaterCR, BDist, Zero);

        // If all the points are on the same side we don't intersect.
        if (XMComparisonAllTrue(BDistIsLessCR) || XMComparisonAllTrue(BDistIsGreaterCR))
            return false;

        // Compute the normal of triangle B.
        XMVECTOR N2 = XMVector3Cross(XMVectorSubtract(B1, B0), XMVectorSubtract(B2, B0));

        // Assert that the triangle is not degenerate.
        assert(!XMVector3Equal(N2, Zero));

        // Test points of A against the plane of B.
        XMVECTOR ADist = XMVector3Dot(N2, XMVectorSubtract(A0, B0));
        ADist = XMVectorSelect(ADist, XMVector3Dot(N2, XMVectorSubtract(A1, B0)), SelectY);
        ADist = XMVectorSelect(ADist, XMVector3Dot(N2, XMVectorSubtract(A2, B0)), SelectZ);

        // Ensure robustness with co-planar triangles by zeroing small distances.
        uint32_t ADistIsZeroCR;
        XMVECTOR ADistIsZero = XMVectorGreaterR(&ADistIsZeroCR, g_RayEpsilon, XMVectorAbs(BDist));
        ADist = XMVectorSelect(ADist, Zero, ADistIsZero);

        uint32_t ADistIsLessCR;
        XMVECTOR ADistIsLess = XMVectorGreaterR(&ADistIsLessCR, Zero, ADist);

        uint32_t ADistIsGreaterCR;
        XMVECTOR ADistIsGreater = XMVectorGreaterR(&ADistIsGreaterCR, ADist, Zero);

        // If all the points are on the same side we don't intersect.
        if (XMComparisonAllTrue(ADistIsLessCR) || XMComparisonAllTrue(ADistIsGreaterCR))
            return false;

        // Special case for co-planar triangles.
        if (XMComparisonAllTrue(ADistIsZeroCR) || XMComparisonAllTrue(BDistIsZeroCR))
        {
            XMVECTOR Axis, Dist, MinDist;

            // Compute an axis perpindicular to the edge (points out).
            Axis = XMVector3Cross(N1, XMVectorSubtract(A1, A0));
            Dist = XMVector3Dot(Axis, A0);

            // Test points of B against the axis.
            MinDist = XMVector3Dot(B0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            // Edge (A1, A2)
            Axis = XMVector3Cross(N1, XMVectorSubtract(A2, A1));
            Dist = XMVector3Dot(Axis, A1);

            MinDist = XMVector3Dot(B0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            // Edge (A2, A0)
            Axis = XMVector3Cross(N1, XMVectorSubtract(A0, A2));
            Dist = XMVector3Dot(Axis, A2);

            MinDist = XMVector3Dot(B0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(B2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            // Edge (B0, B1)
            Axis = XMVector3Cross(N2, XMVectorSubtract(B1, B0));
            Dist = XMVector3Dot(Axis, B0);

            MinDist = XMVector3Dot(A0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            // Edge (B1, B2)
            Axis = XMVector3Cross(N2, XMVectorSubtract(B2, B1));
            Dist = XMVector3Dot(Axis, B1);

            MinDist = XMVector3Dot(A0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            // Edge (B2,B0)
            Axis = XMVector3Cross(N2, XMVectorSubtract(B0, B2));
            Dist = XMVector3Dot(Axis, B2);

            MinDist = XMVector3Dot(A0, Axis);
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A1, Axis));
            MinDist = XMVectorMin(MinDist, XMVector3Dot(A2, Axis));
            if (XMVector4GreaterOrEqual(MinDist, Dist))
                return false;

            return true;
        }

        //
        // Find the single vertex of A and B (ie the vertex on the opposite side
        // of the plane from the other two) and reorder the edges so we can compute
        // the signed edge/edge distances.
        //
        // if ( (V0 >= 0 && V1 <  0 && V2 <  0) ||
        //      (V0 >  0 && V1 <= 0 && V2 <= 0) ||
        //      (V0 <= 0 && V1 >  0 && V2 >  0) ||
        //      (V0 <  0 && V1 >= 0 && V2 >= 0) ) then V0 is singular;
        //
        // If our singular vertex is not on the positive side of the plane we reverse
        // the triangle winding so that the overlap comparisons will compare the
        // correct edges with the correct signs.
        //
        XMVECTOR ADistIsLessEqual = XMVectorOrInt(ADistIsLess, ADistIsZero);
        XMVECTOR ADistIsGreaterEqual = XMVectorOrInt(ADistIsGreater, ADistIsZero);

        XMVECTOR AA0, AA1, AA2;
        bool bPositiveA;

        if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreaterEqual, ADistIsLess, Select0111)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreater, ADistIsLessEqual, Select0111)))
        {
            // A0 is singular, crossing from positive to negative.
            AA0 = A0; AA1 = A1; AA2 = A2;
            bPositiveA = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLessEqual, ADistIsGreater, Select0111)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLess, ADistIsGreaterEqual, Select0111)))
        {
            // A0 is singular, crossing from negative to positive.
            AA0 = A0; AA1 = A2; AA2 = A1;
            bPositiveA = false;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreaterEqual, ADistIsLess, Select1011)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreater, ADistIsLessEqual, Select1011)))
        {
            // A1 is singular, crossing from positive to negative.
            AA0 = A1; AA1 = A2; AA2 = A0;
            bPositiveA = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLessEqual, ADistIsGreater, Select1011)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLess, ADistIsGreaterEqual, Select1011)))
        {
            // A1 is singular, crossing from negative to positive.
            AA0 = A1; AA1 = A0; AA2 = A2;
            bPositiveA = false;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreaterEqual, ADistIsLess, Select1101)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsGreater, ADistIsLessEqual, Select1101)))
        {
            // A2 is singular, crossing from positive to negative.
            AA0 = A2; AA1 = A0; AA2 = A1;
            bPositiveA = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLessEqual, ADistIsGreater, Select1101)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(ADistIsLess, ADistIsGreaterEqual, Select1101)))
        {
            // A2 is singular, crossing from negative to positive.
            AA0 = A2; AA1 = A1; AA2 = A0;
            bPositiveA = false;
        }
        else
        {
            assert(false);
            return false;
        }

        XMVECTOR BDistIsLessEqual = XMVectorOrInt(BDistIsLess, BDistIsZero);
        XMVECTOR BDistIsGreaterEqual = XMVectorOrInt(BDistIsGreater, BDistIsZero);

        XMVECTOR BB0, BB1, BB2;
        bool bPositiveB;

        if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreaterEqual, BDistIsLess, Select0111)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreater, BDistIsLessEqual, Select0111)))
        {
            // B0 is singular, crossing from positive to negative.
            BB0 = B0; BB1 = B1; BB2 = B2;
            bPositiveB = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLessEqual, BDistIsGreater, Select0111)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLess, BDistIsGreaterEqual, Select0111)))
        {
            // B0 is singular, crossing from negative to positive.
            BB0 = B0; BB1 = B2; BB2 = B1;
            bPositiveB = false;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreaterEqual, BDistIsLess, Select1011)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreater, BDistIsLessEqual, Select1011)))
        {
            // B1 is singular, crossing from positive to negative.
            BB0 = B1; BB1 = B2; BB2 = B0;
            bPositiveB = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLessEqual, BDistIsGreater, Select1011)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLess, BDistIsGreaterEqual, Select1011)))
        {
            // B1 is singular, crossing from negative to positive.
            BB0 = B1; BB1 = B0; BB2 = B2;
            bPositiveB = false;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreaterEqual, BDistIsLess, Select1101)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsGreater, BDistIsLessEqual, Select1101)))
        {
            // B2 is singular, crossing from positive to negative.
            BB0 = B2; BB1 = B0; BB2 = B1;
            bPositiveB = true;
        }
        else if (DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLessEqual, BDistIsGreater, Select1101)) ||
            DirectX::Internal::XMVector3AllTrue(XMVectorSelect(BDistIsLess, BDistIsGreaterEqual, Select1101)))
        {
            // B2 is singular, crossing from negative to positive.
            BB0 = B2; BB1 = B1; BB2 = B0;
            bPositiveB = false;
        }
        else
        {
            assert(false);
            return false;
        }

        XMVECTOR Delta0, Delta1;

        // Reverse the direction of the test depending on whether the singular vertices are
        // the same sign or different signs.
        if (bPositiveA ^ bPositiveB)
        {
            Delta0 = XMVectorSubtract(BB0, AA0);
            Delta1 = XMVectorSubtract(AA0, BB0);
        }
        else
        {
            Delta0 = XMVectorSubtract(AA0, BB0);
            Delta1 = XMVectorSubtract(BB0, AA0);
        }

        // Check if the triangles overlap on the line of intersection between the
        // planes of the two triangles by finding the signed line distances.
        XMVECTOR Dist0 = XMVector3Dot(Delta0, XMVector3Cross(XMVectorSubtract(BB2, BB0), XMVectorSubtract(AA2, AA0)));
        if (XMVector4Greater(Dist0, Zero))
            return false;

        XMVECTOR Dist1 = XMVector3Dot(Delta1, XMVector3Cross(XMVectorSubtract(BB1, BB0), XMVectorSubtract(AA1, AA0)));
        if (XMVector4Greater(Dist1, Zero))
            return false;

        return true;
    }


    //-----------------------------------------------------------------------------
    // Ray-triangle test
    //-----------------------------------------------------------------------------
    _Use_decl_annotations_
        inline PlaneIntersectionType XM_CALLCONV Intersects(FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2, GXMVECTOR Plane) noexcept
    {
        XMVECTOR One = XMVectorSplatOne();

        assert(DirectX::Internal::XMPlaneIsUnit(Plane));

        // Set w of the points to one so we can dot4 with a plane.
        XMVECTOR TV0 = XMVectorInsert<0, 0, 0, 0, 1>(V0, One);
        XMVECTOR TV1 = XMVectorInsert<0, 0, 0, 0, 1>(V1, One);
        XMVECTOR TV2 = XMVectorInsert<0, 0, 0, 0, 1>(V2, One);

        XMVECTOR Outside, Inside;
        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane, Outside, Inside);

        // If the triangle is outside any plane it is outside.
        if (XMVector4EqualInt(Outside, XMVectorTrueInt()))
            return FRONT;

        // If the triangle is inside all planes it is inside.
        if (XMVector4EqualInt(Inside, XMVectorTrueInt()))
            return BACK;

        // The triangle is not inside all planes or outside a plane it intersects.
        return INTERSECTING;
    }


    //-----------------------------------------------------------------------------
    // Test a triangle vs 6 planes (typically forming a frustum).
    //-----------------------------------------------------------------------------
    _Use_decl_annotations_
        inline ContainmentType XM_CALLCONV ContainedBy(
            FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2,
            GXMVECTOR Plane0,
            HXMVECTOR Plane1, HXMVECTOR Plane2,
            CXMVECTOR Plane3, CXMVECTOR Plane4, CXMVECTOR Plane5) noexcept
    {
        XMVECTOR One = XMVectorSplatOne();

        // Set w of the points to one so we can dot4 with a plane.
        XMVECTOR TV0 = XMVectorInsert<0, 0, 0, 0, 1>(V0, One);
        XMVECTOR TV1 = XMVectorInsert<0, 0, 0, 0, 1>(V1, One);
        XMVECTOR TV2 = XMVectorInsert<0, 0, 0, 0, 1>(V2, One);

        XMVECTOR Outside, Inside;

        // Test against each plane.
        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane0, Outside, Inside);

        XMVECTOR AnyOutside = Outside;
        XMVECTOR AllInside = Inside;

        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane1, Outside, Inside);
        AnyOutside = XMVectorOrInt(AnyOutside, Outside);
        AllInside = XMVectorAndInt(AllInside, Inside);

        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane2, Outside, Inside);
        AnyOutside = XMVectorOrInt(AnyOutside, Outside);
        AllInside = XMVectorAndInt(AllInside, Inside);

        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane3, Outside, Inside);
        AnyOutside = XMVectorOrInt(AnyOutside, Outside);
        AllInside = XMVectorAndInt(AllInside, Inside);

        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane4, Outside, Inside);
        AnyOutside = XMVectorOrInt(AnyOutside, Outside);
        AllInside = XMVectorAndInt(AllInside, Inside);

        DirectX::Internal::FastIntersectTrianglePlane(TV0, TV1, TV2, Plane5, Outside, Inside);
        AnyOutside = XMVectorOrInt(AnyOutside, Outside);
        AllInside = XMVectorAndInt(AllInside, Inside);

        // If the triangle is outside any plane it is outside.
        if (XMVector4EqualInt(AnyOutside, XMVectorTrueInt()))
            return DISJOINT;

        // If the triangle is inside all planes it is inside.
        if (XMVector4EqualInt(AllInside, XMVectorTrueInt()))
            return CONTAINS;

        // The triangle is not inside all planes or outside a plane, it may intersect.
        return INTERSECTS;
    }

} // namespace TriangleTests