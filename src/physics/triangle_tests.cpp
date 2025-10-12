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

void IntersectTrianglePlane(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept {
    // Plane0
    float Dist0 = glm::dot(glm::vec4(V0, 1.0f), Plane);
    float Dist1 = glm::dot(glm::vec4(V1, 1.0f), Plane);
    float Dist2 = glm::dot(glm::vec4(V2, 1.0f), Plane);

    float MinDist = glm::min(Dist0, Dist1);
    MinDist = glm::min(MinDist, Dist2);

    float MaxDist = glm::max(Dist0, Dist1);
    MaxDist = glm::max(MaxDist, Dist2);

    // Outside the plane?
    Outside = MinDist > 0.0f;

    // Fully inside the plane?
    Inside = MaxDist < 0.0f;
}

void IntersectOrientedBoxPlane(const glm::vec3& Center, const glm::vec3& Extents, const glm::vec3& Axis0, const glm::vec3& Axis1, const glm::vec3& Axis2, const glm::vec4& Plane, bool & Outside, bool& Inside) noexcept {
        // Compute the distance to the center of the box.
        float Dist = glm::dot(glm::vec4(Center, 0.0f), Plane);

        // Project the axes of the box onto the normal of the plane.  Half the
        // length of the projection (sometime called the "radius") is equal to
        // h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
        // where h(i) are extents of the box, n is the plane normal, and b(i) are the
        // axes of the box.
        glm::vec3 Radius;
        Radius.x = glm::dot(Plane, glm::vec4(Axis0, 0.0f));
        Radius.y = glm::dot(Plane, glm::vec4(Axis1, 0.0f));
        Radius.z = glm::dot(Plane, glm::vec4(Axis2, 0.0f));

        float R = glm::dot(Extents, glm::abs(Radius));

        // Outside the plane?
        Outside = Dist > R;

        // Fully inside the plane?
        Inside = Dist < -R;
    }

void IntersectAxisAlignedBoxPlane(const glm::vec3& Center, const glm::vec3& Extents, const glm::vec4& Plane, bool& Outside, bool& Inside) noexcept {
    // Compute the distance to the center of the box.
    float Dist = glm::dot(glm::vec4(Center, 0.0f), Plane);

    // Project the axes of the box onto the normal of the plane.  Half the
    // length of the projection (sometime called the "radius") is equal to
    // h(u) * abs(n dot b(u))) + h(v) * abs(n dot b(v)) + h(w) * abs(n dot b(w))
    // where h(i) are extents of the box, n is the plane normal, and b(i) are the
    // axes of the box. In this case b(i) = [(1,0,0), (0,1,0), (0,0,1)].
    float Radius = glm::dot(Extents, glm::abs(glm::vec3(Plane)));

    // Outside the plane?
    Outside = Dist > Radius;

    // Fully inside the plane?
    Inside = Dist < -Radius;
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

bool Vector3InBounds(const glm::vec3& V, const glm::vec3& Bounds) noexcept{
    return (
        (
            (V[0] <= Bounds[0] && V[0] >= -Bounds[0]) &&
            (V[1] <= Bounds[1] && V[1] >= -Bounds[1]) &&
            (V[2] <= Bounds[2] && V[2] >= -Bounds[2])
        ) != 0
    );
}

glm::vec3 VectorSelect(const glm::vec3& V1, const glm::vec3& V2, const glm::bvec3& Control) {
    glm::vec3 Result = glm::vec3(
        Control[0] ? V2[0] : V1[0],
        Control[1] ? V2[1] : V1[1],
        Control[2] ? V2[2] : V1[2]
    );
    return Result;
}

glm::vec4 VectorSelect(const glm::vec4& V1, const glm::vec4& V2, const glm::bvec4& Control) {
    glm::vec4 Result = glm::vec4(
        Control[0] ? V2[0] : V1[0],
        Control[1] ? V2[1] : V1[1],
        Control[2] ? V2[2] : V1[2],
        Control[3] ? V2[3] : V1[3]
    );
    return Result;
}

glm::bvec3 VectorInBounds(glm::vec3 V, glm::vec3 Bounds) noexcept {

    glm::bvec3 Control = glm::bvec3(
        (V[0] <= Bounds[0] && V[0] >= -Bounds[0]) ? true : false,
        (V[1] <= Bounds[1] && V[1] >= -Bounds[1]) ? true : false,
        (V[2] <= Bounds[2] && V[2] >= -Bounds[2]) ? true : false
    );
    return Control;
}

glm::bvec4 VectorInBounds(glm::vec4 V, glm::vec4 Bounds) noexcept {

    glm::bvec4 Control = glm::bvec4(
        (V[0] <= Bounds[0] && V[0] >= -Bounds[0]) ? true : false,
        (V[1] <= Bounds[1] && V[1] >= -Bounds[1]) ? true : false,
        (V[2] <= Bounds[2] && V[2] >= -Bounds[2]) ? true : false,
        (V[3] <= Bounds[3] && V[3] >= -Bounds[3]) ? true : false
    );
    return Control;
}

bool CalculateEigenVectorsFromCovarianceMatrix(float Cxx, float Cyy, float Czz, float Cxy, float Cxz, float Cyz, glm::vec3* pV1, glm::vec3* pV2, glm::vec3* pV3) noexcept {
    // Calculate the eigenvalues by solving a cubic equation.
    float e = -(Cxx + Cyy + Czz);
    float f = Cxx * Cyy + Cyy * Czz + Czz * Cxx - Cxy * Cxy - Cxz * Cxz - Cyz * Cyz;
    float g = Cxy * Cxy * Czz + Cxz * Cxz * Cyy + Cyz * Cyz * Cxx - Cxy * Cyz * Cxz * 2.0f - Cxx * Cyy * Czz;

    float ev1, ev2, ev3;
    if (!SolveCubic(e, f, g, &ev1, &ev2, &ev3)) {
        // set them to arbitrary orthonormal basis set
        *pV1 = glm::vec3(1.0f, 0.0f, 0.0f);
        *pV2 = glm::vec3(0.0f, 1.0f, 0.0f);
        *pV3 = glm::vec3(0.0f, 0.0f, 1.0f);
        return false;
    }

    return CalculateEigenVectors(Cxx, Cxy, Cxz, Cyy, Cyz, Czz, ev1, ev2, ev3, pV1, pV2, pV3);
}

bool SolveCubic(float e, float f, float g, float* t, float* u, float* v) noexcept {
    float p, q, h, rc, d, theta, costh3, sinth3;

    p = f - e * e / 3.0f;
    q = g - e * f / 3.0f + e * e * e * 2.0f / 27.0f;
    h = q * q / 4.0f + p * p * p / 27.0f;

    if (h > 0.0f) {
        *t = *u = *v = 0.0f;
        return false; // only one real root
    }

    // all the same root
    if ((h == 0.0) && (q == 0.0)) {
        *t = -e / 3.0f;
        *u = -e / 3.0f;
        *v = -e / 3.0f;

        return true;
    }

    d = sqrtf(q * q / 4.0f - h);
    if (d < 0) {
        rc = -powf(-d, 1.0f / 3.0f);
    }
    else {
        rc = powf(d, 1.0f / 3.0f);
    }

    theta = glm::acos(-q / (2.0f * d));
    costh3 = glm::cos(theta / 3.0f);
    sinth3 = sqrtf(3.0f) * glm::sin(theta / 3.0f);
    *t = 2.0f * rc * costh3 - e / 3.0f;
    *u = -rc * (costh3 + sinth3) - e / 3.0f;
    *v = -rc * (costh3 - sinth3) - e / 3.0f;

    return true;
}

bool CalculateEigenVectors(float m11, float m12, float m13, float m22, float m23, float m33, float e1, float e2, float e3, glm::vec3* pV1, glm::vec3* pV2, glm::vec3* pV3) noexcept {
    *pV1 = CalculateEigenVector(m11, m12, m13, m22, m23, m33, e1);
    *pV2 = CalculateEigenVector(m11, m12, m13, m22, m23, m33, e2);
    *pV3 = CalculateEigenVector(m11, m12, m13, m22, m23, m33, e3);

    bool v1z = false;
    bool v2z = false;
    bool v3z = false;

    glm::vec3 Zero = glm::vec3(0.0f);

    if (glm::all(glm::equal(*pV1, Zero))) {
        v1z = true;
    }

    if (glm::all(glm::equal(*pV2, Zero))) {
        v2z = true;
    }

    if (glm::all(glm::equal(*pV3, Zero))) {
        v3z = true;
    }

    bool e12 = glm::abs((glm::dot(*pV1, *pV2))) > 0.1f; // check for non-orthogonal vectors
    bool e13 = glm::abs((glm::dot(*pV1, *pV3))) > 0.1f;
    bool e23 = glm::abs((glm::dot(*pV2, *pV3))) > 0.1f;

    // all eigenvectors are 0- any basis set
    if ((v1z && v2z && v3z) || (e12 && e13 && e23) || (e12 && v3z) || (e13 && v2z) || (e23 && v1z)) {
        *pV1 = glm::vec3(1.0f, 0.0f, 0.0f);
        *pV2 = glm::vec3(0.0f, 1.0f, 0.0f);
        *pV3 = glm::vec3(0.0f, 0.0f, 1.0f);
        return true;
    }

    if (v1z && v2z) {
        glm::vec3 vTmp = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), *pV3);
        if (glm::dot(vTmp, vTmp) < 1e-5f) {
            vTmp = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), *pV3);
        }
        *pV1 = glm::normalize(vTmp);
        *pV2 = glm::cross(*pV3, *pV1);
        return true;
    }

    if (v3z && v1z) {
        glm::vec3 vTmp = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), *pV2);
        if (glm::dot(vTmp, vTmp) < 1e-5f) {
            vTmp = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), *pV2);
        }
        *pV3 = glm::normalize(vTmp);
        *pV1 = glm::cross(*pV2, *pV3);
        return true;
    }

    if (v2z && v3z) {
        glm::vec3 vTmp = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), *pV1);
        if (glm::dot(vTmp, vTmp) < 1e-5f) {
            vTmp = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), *pV1);
        }
        *pV2 = glm::normalize(vTmp);
        *pV3 = glm::cross(*pV1, *pV2);
        return true;
    }

    if ((v1z) || e12) {
        *pV1 = glm::cross(*pV2, *pV3);
        return true;
    }

    if ((v2z) || e23) {
        *pV2 = glm::cross(*pV3, *pV1);
        return true;
    }

    if ((v3z) || e13) {
        *pV3 = glm::cross(*pV1, *pV2);
        return true;
    }

    return true;
}

glm::vec3 CalculateEigenVector(float m11, float m12, float m13, float m22, float m23, float m33, float e) noexcept {
    float fTmp[3];
    fTmp[0] = m12 * m23 - m13 * (m22 - e);
    fTmp[1] = m13 * m12 - m23 * (m11 - e);
    fTmp[2] = (m11 - e) * (m22 - e) - m12 * m12;

    glm::vec3 vTmp = glm::vec3(*reinterpret_cast<const glm::vec3*>(fTmp));

    // planar or linear
    if (glm::all(glm::equal(vTmp, glm::vec3(0.0f)))) {
        float f1, f2, f3;

        // we only have one equation - find a valid one
        if ((m11 - e != 0.0f) || (m12 != 0.0f) || (m13 != 0.0f)) {
            f1 = m11 - e; f2 = m12; f3 = m13;
        }
        else if ((m12 != 0.0f) || (m22 - e != 0.0f) || (m23 != 0.0f)) {
            f1 = m12; f2 = m22 - e; f3 = m23;
        }
        else if ((m13 != 0.0f) || (m23 != 0.0f) || (m33 - e != 0.0f)) {
            f1 = m13; f2 = m23; f3 = m33 - e;
        }
        else {
            // error, we'll just make something up - we have NO context
            f1 = 1.0f; f2 = 0.0f; f3 = 0.0f;
        }

        if (f1 == 0.0f) {
            vTmp.x = 0.0f;
        }
        else {
            vTmp.x = 1.0f;
        }

        if (f2 == 0.0f) {
            vTmp.y = 0.0f;
        }
        else {
            vTmp.y = 1.0f;
        }

        if (f3 == 0.0f) {
            vTmp.z = 0.0f;
            // recalculate y to make equation work
            if (m12 != 0.0f) {
                vTmp.y = -f1 / f2;
            }
        }
        else {
            vTmp.z = (f2 - f1) / f3;
        }
    }

    if (glm::dot(vTmp, vTmp) > 1e-5f) {
        return glm::normalize(vTmp);
    }
    else {
        // Multiply by a value large enough to make the vector non-zero.
        vTmp = vTmp * 1e5f;
        return glm::normalize(vTmp);
    }
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
    bool Intersects(const glm::vec3& Origin, const glm::vec3& Direction, const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, float& Dist) noexcept {

        glm::vec3 e1 = V1 - V0;
        glm::vec3 e2 = V2 - V0;

        // p = Direction ^ e2;
        glm::vec3 p = glm::cross(Direction, e2);

        // det = e1 * p;
        float det = glm::dot(e1, p);

        float u, v, t;

        if (det >= g_RayEpsilon.x) {
            // Determinate is positive (front side of the triangle).
            glm::vec3 s = Origin - V0;

            // u = s * p;
            u = glm::dot(s, p);

            bool NoIntersection = u < 0.0f;
            NoIntersection |= u > det;

            // q = s ^ e1;
            glm::vec3 q = glm::cross(s, e1);

            // v = Direction * q;
            v = glm::dot(Direction, q);

            NoIntersection |= v < 0.0f;
            NoIntersection |= (u + v) > det;

            // t = e2 * q;
            t = glm::dot(e2, q);

            NoIntersection |= t < 0.0f;

            if (NoIntersection) {
                Dist = 0.0f;
                return false;
            }
        }
        else if (det <= -g_RayEpsilon.x) {
            // Determinate is negative (back side of the triangle).
            glm::vec3 s = Origin - V0;

            // u = s * p;
            u = glm::dot(s, p);

            bool NoIntersection = u > 0.0f;
            NoIntersection |= u < det;

            // q = s ^ e1;
            glm::vec3 q = glm::cross(s, e1);

            // v = Direction * q;
            v = glm::dot(Direction, q);

            NoIntersection |= v > 0.0f;
            NoIntersection |= (u + v) < det;

            // t = e2 * q;
            t = glm::dot(e2, q);

            NoIntersection |= t > 0.0f;

            if (NoIntersection) {
                Dist = 0.0f;
                return false;
            }
        }
        else {
            // Parallel ray.
            Dist = 0.f;
            return false;
        }

        t /= det;

        // (u / det) and (v / dev) are the barycentric cooridinates of the intersection.

        // Store the x-component to *pDist
        Dist = t;

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
    bool Intersects(const glm::vec3& A0, const glm::vec3& A1, const glm::vec3& A2, const glm::vec3& B0, const glm::vec3& B1, const glm::vec3& B2) noexcept {

        // Compute the normal of triangle A.
        glm::vec3 N1 = glm::cross(A1 - A0, A2 - A0);

        // Test points of B against the plane of A.
        glm::vec3 BDist;
        BDist.x = glm::dot(N1, B0 - A0);
        BDist.y = glm::dot(N1, B1 - A0);
        BDist.z = glm::dot(N1, B2 - A0);

        // Ensure robustness with co-planar triangles by zeroing small distances.
        bool BDistIsZero = g_RayEpsilon.x > glm::abs(BDist.x) && g_RayEpsilon.y > glm::abs(BDist.y) && g_RayEpsilon.z > glm::abs(BDist.z);
        bool BDistIsZeroX = g_RayEpsilon.x > glm::abs(BDist.x);
        bool BDistIsZeroY = g_RayEpsilon.y > glm::abs(BDist.y);
        bool BDistIsZeroZ = g_RayEpsilon.z > glm::abs(BDist.z);
        BDist.x = g_RayEpsilon.x > glm::abs(BDist.x) ? 0.0f : BDist.x;
        BDist.y = g_RayEpsilon.y > glm::abs(BDist.y) ? 0.0f : BDist.y;
        BDist.x = g_RayEpsilon.z > glm::abs(BDist.z) ? 0.0f : BDist.z;
        
        bool BDistIsLess = 0.0f < BDist.x && 0.0f < BDist.y && 0.0f < BDist.z;
        bool BDistIsLessX = 0.0f < BDist.x;
        bool BDistIsLessY = 0.0f < BDist.y;
        bool BDistIsLessZ = 0.0f < BDist.z;
        bool BDistIsGreater = BDist.x > 0.0f && BDist.y > 0.0f && BDist.z > 0.0f;
        bool BDistIsGreaterX = BDist.x > 0.0f;
        bool BDistIsGreaterY = BDist.y > 0.0f;
        bool BDistIsGreaterZ = BDist.z > 0.0f;

        // If all the points are on the same side we don't intersect.
        if (BDistIsLess || BDistIsGreater) {
            return false;
        }

        // Compute the normal of triangle B.
        glm::vec3 N2 = glm::cross(B1 - B0, B2 - B0);

        // Test points of A against the plane of B.
        glm::vec3 ADist;
        ADist.x = glm::dot(N2, A0 - B0);
        ADist.y = glm::dot(N2, A1 - B0);
        ADist.z = glm::dot(N2, A2 - B0);

        // Ensure robustness with co-planar triangles by zeroing small distances.
        bool ADistIsZero = g_RayEpsilon.x > glm::abs(BDist.x) && g_RayEpsilon.y > glm::abs(BDist.y) && g_RayEpsilon.z > glm::abs(BDist.z);
        bool ADistIsZeroX = g_RayEpsilon.x > glm::abs(BDist.x);
        bool ADistIsZeroY = g_RayEpsilon.y > glm::abs(BDist.y);
        bool ADistIsZeroZ = g_RayEpsilon.z > glm::abs(BDist.z);
        ADist.x = g_RayEpsilon.x > glm::abs(BDist.x) ? 0.0f : ADist.x;
        ADist.y = g_RayEpsilon.y > glm::abs(BDist.y) ? 0.0f : ADist.y;
        ADist.z = g_RayEpsilon.z > glm::abs(BDist.z) ? 0.0f : ADist.z;

        bool ADistIsLess = ADist.x < 0.0f && ADist.y < 0.0f && ADist.z < 0.0f;
        bool ADistIsLessX = ADist.x < 0.0f;
        bool ADistIsLessY = ADist.y < 0.0f;
        bool ADistIsLessZ = ADist.z < 0.0f;

        bool ADistIsGreater = ADist.x > 0.0f && ADist.y > 0.0f && ADist.z > 0.0f;
        bool ADistIsGreaterX = ADist.x > 0.0;
        bool ADistIsGreaterY = ADist.y > 0.0f;
        bool ADistIsGreaterZ = ADist.z > 0.0f;

        // If all the points are on the same side we don't intersect.
        if (ADistIsLess || ADistIsGreater) {
            return false;
        }

        // Special case for co-planar triangles.
        if (ADistIsZero || BDistIsZero) {
            glm::vec3 Axis;
            float Dist;
            float MinDist;

            // Compute an axis perpindicular to the edge (points out).
            Axis = glm::cross(N1, A1 - A0);
            Dist = glm::dot(Axis, A0);

            // Test points of B against the axis.
            MinDist = glm::dot(B0, Axis);
            MinDist = glm::min(MinDist, glm::dot(B1, Axis));
            MinDist = glm::min(MinDist, glm::dot(B2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

            // Edge (A1, A2)
            Axis = glm::cross(N1, A2 - A1);
            Dist = glm::dot(Axis, A1);

            MinDist = glm::dot(B0, Axis);
            MinDist = glm::min(MinDist, glm::dot(B1, Axis));
            MinDist = glm::min(MinDist, glm::dot(B2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

            // Edge (A2, A0)
            Axis = glm::cross(N1, A0 - A2);
            Dist = glm::dot(Axis, A2);

            MinDist = glm::dot(B0, Axis);
            MinDist = glm::min(MinDist, glm::dot(B1, Axis));
            MinDist = glm::min(MinDist, glm::dot(B2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

            // Edge (B0, B1)
            Axis = glm::cross(N2, B1 - B0);
            Dist = glm::dot(Axis, B0);

            MinDist = glm::dot(A0, Axis);
            MinDist = glm::min(MinDist, glm::dot(A1, Axis));
            MinDist = glm::min(MinDist, glm::dot(A2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

            // Edge (B1, B2)
            Axis = glm::cross(N2, B2 - B1);
            Dist = glm::dot(Axis, B1);

            MinDist = glm::dot(A0, Axis);
            MinDist = glm::min(MinDist, glm::dot(A1, Axis));
            MinDist = glm::min(MinDist, glm::dot(A2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

            // Edge (B2,B0)
            Axis = glm::cross(N2, B0 - B2);
            Dist = glm::dot(Axis, B2);

            MinDist = glm::dot(A0, Axis);
            MinDist = glm::min(MinDist, glm::dot(A1, Axis));
            MinDist = glm::min(MinDist, glm::dot(A2, Axis));
            if (MinDist >= Dist) {
                return false;
            }

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
        bool ADistIsLessEqual = ADistIsLess || ADistIsZero;
        bool ADistIsLessEqualX = ADistIsLessX || ADistIsZeroX;
        bool ADistIsLessEqualY = ADistIsLessY || ADistIsZeroY;
        bool ADistIsLessEqualZ = ADistIsLessZ || ADistIsZeroZ;
        bool ADistIsGreaterEqual = ADistIsGreater || ADistIsZero;
        bool ADistIsGreaterEqualX = ADistIsGreaterX || ADistIsZeroX;
        bool ADistIsGreaterEqualY = ADistIsGreaterY || ADistIsZeroY;
        bool ADistIsGreaterEqualZ = ADistIsGreaterZ || ADistIsZeroZ;

        glm::vec3 AA0, AA1, AA2;
        bool bPositiveA;

        if ((ADistIsGreaterEqualX && ADistIsLessY && ADistIsLessZ) || (ADistIsGreaterX && ADistIsLessEqualY && ADistIsLessEqualZ)) {
            // A0 is singular, crossing from positive to negative.
            AA0 = A0; AA1 = A1; AA2 = A2;
            bPositiveA = true;
        }
        else if ((ADistIsLessEqualX && ADistIsGreaterY && ADistIsGreaterZ) || (ADistIsLessX && ADistIsGreaterEqualY && ADistIsGreaterEqualZ)) {
            // A0 is singular, crossing from negative to positive.
            AA0 = A0; AA1 = A2; AA2 = A1;
            bPositiveA = false;
        }
        else if ((ADistIsLessX && ADistIsGreaterEqualY && ADistIsLessZ) || (ADistIsLessEqualX, ADistIsGreaterY, ADistIsLessEqualZ)) {
            // A1 is singular, crossing from positive to negative.
            AA0 = A1; AA1 = A2; AA2 = A0;
            bPositiveA = true;
        }
        else if ((ADistIsGreaterX && ADistIsLessEqualY && ADistIsGreaterZ) || (ADistIsGreaterEqualX && ADistIsLessY && ADistIsGreaterEqualZ)) {
            // A1 is singular, crossing from negative to positive.
            AA0 = A1; AA1 = A0; AA2 = A2;
            bPositiveA = false;
        }
        else if ((ADistIsLessX && ADistIsLessY && ADistIsGreaterEqualZ) || (ADistIsLessEqualX && ADistIsLessEqualY && ADistIsGreaterZ)) {
            // A2 is singular, crossing from positive to negative.
            AA0 = A2; AA1 = A0; AA2 = A1;
            bPositiveA = true;
        }
        else if ((ADistIsGreaterX && ADistIsGreaterY && ADistIsLessEqualZ) || (ADistIsGreaterEqualX && ADistIsGreaterEqualY && ADistIsLessZ)) {
            // A2 is singular, crossing from negative to positive.
            AA0 = A2; AA1 = A1; AA2 = A0;
            bPositiveA = false;
        }
        else {
            return false;
        }

        bool BDistIsLessEqual = BDistIsLess || BDistIsZero;
        bool BDistIsLessEqualX = BDistIsLessX || BDistIsZeroX;
        bool BDistIsLessEqualY = BDistIsLessY || BDistIsZeroY;
        bool BDistIsLessEqualZ = BDistIsLessZ || BDistIsZeroZ;
        bool BDistIsGreaterEqual = BDistIsGreater || BDistIsZero;
        bool BDistIsGreaterEqualX = BDistIsGreaterX || BDistIsZeroX;
        bool BDistIsGreaterEqualY = BDistIsGreaterY || BDistIsZeroY;
        bool BDistIsGreaterEqualZ = BDistIsGreaterZ || BDistIsZeroZ;

        glm::vec3 BB0, BB1, BB2;
        bool bPositiveB;

        if ((BDistIsGreaterEqualX && BDistIsLessY && BDistIsLessZ) || (BDistIsGreaterX && BDistIsLessEqualY && BDistIsLessEqualZ)) {
            // B0 is singular, crossing from positive to negative.
            BB0 = B0; BB1 = B1; BB2 = B2;
            bPositiveB = true;
        }
        else if ((BDistIsLessEqualX && BDistIsGreaterY && BDistIsGreaterZ) || (BDistIsLessX && BDistIsGreaterEqualY && BDistIsGreaterEqualZ)) {
            // B0 is singular, crossing from negative to positive.
            BB0 = B0; BB1 = B2; BB2 = B1;
            bPositiveB = false;
        }
        else if ((BDistIsLessX && BDistIsGreaterEqualY && BDistIsLessZ) || (BDistIsLessEqualX && BDistIsGreaterY && BDistIsLessEqualZ)) {
            // B1 is singular, crossing from positive to negative.
            BB0 = B1; BB1 = B2; BB2 = B0;
            bPositiveB = true;
        }
        else if ((BDistIsGreaterX && BDistIsLessEqualY && BDistIsGreaterZ) || (BDistIsGreaterEqualX && BDistIsLessY && BDistIsGreaterEqualZ)) {
            // B1 is singular, crossing from negative to positive.
            BB0 = B1; BB1 = B0; BB2 = B2;
            bPositiveB = false;
        }
        else if ((BDistIsLessX && BDistIsLessY && BDistIsGreaterEqualZ) || (BDistIsLessEqualX && BDistIsLessEqualY && BDistIsGreaterZ)) {
            // B2 is singular, crossing from positive to negative.
            BB0 = B2; BB1 = B0; BB2 = B1;
            bPositiveB = true;
        }
        else if ((BDistIsGreaterX && BDistIsGreaterY && BDistIsLessEqualZ) || (BDistIsGreaterEqualX && BDistIsGreaterEqualY && BDistIsLessZ)) {
            // B2 is singular, crossing from negative to positive.
            BB0 = B2; BB1 = B1; BB2 = B0;
            bPositiveB = false;
        }
        else {
            assert(false);
            return false;
        }

        glm::vec3 Delta0, Delta1;

        // Reverse the direction of the test depending on whether the singular vertices are
        // the same sign or different signs.
        if (bPositiveA ^ bPositiveB)
        {
            Delta0 = BB0 - AA0;
            Delta1 = AA0 - BB0;
        }
        else {
            Delta0 = AA0 - BB0;
            Delta1 = BB0 - AA0;
        }

        // Check if the triangles overlap on the line of intersection between the
        // planes of the two triangles by finding the signed line distances.
        float Dist0 = glm::dot(Delta0, glm::cross(BB2 - BB0, AA2 - AA0));
        if (Dist0 > 0.0f) {
            return false;
        }

        float Dist1 = glm::dot(Delta1, glm::cross(BB1 - BB0, AA1 - AA0));
        if (Dist1 > 0.0f) {
            return false;
        }

        return true;
    }


    //-----------------------------------------------------------------------------
    // Ray-triangle test
    //-----------------------------------------------------------------------------
    PlaneIntersectionType Intersects(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane) noexcept {
        
        // Set w of the points to one so we can dot4 with a plane.
        glm::vec4 TV0 = glm::vec4(V0, 1.0f);
        glm::vec4 TV1 = glm::vec4(V1, 1.0f);
        glm::vec4 TV2 = glm::vec4(V2, 1.0f);

        bool Outside, Inside;
        IntersectTrianglePlane(TV0, TV1, TV2, Plane, Outside, Inside);

        // If the triangle is outside any plane it is outside.
        if (Outside) {
            return FRONT;
        }

        // If the triangle is inside all planes it is inside.
        if (Inside) {
            return BACK;
        }

        // The triangle is not inside all planes or outside a plane it intersects.
        return INTERSECTING;
    }


    //-----------------------------------------------------------------------------
    // Test a triangle vs 6 planes (typically forming a frustum).
    //-----------------------------------------------------------------------------
    ContainmentType ContainedBy(const glm::vec3& V0, const glm::vec3& V1, const glm::vec3& V2, const glm::vec4& Plane0, const glm::vec4& Plane1, const glm::vec4& Plane2, const glm::vec4& Plane3, const glm::vec4& Plane4, const glm::vec4& Plane5) noexcept {

        // Set w of the points to one so we can dot4 with a plane.
        glm::vec4 TV0 = glm::vec4(V0, 1.0f);
        glm::vec4 TV1 = glm::vec4(V1, 1.0f);
        glm::vec4 TV2 = glm::vec4(V2, 1.0f);

        bool Outside, Inside;

        // Test against each plane.
        IntersectTrianglePlane(TV0, TV1, TV2, Plane0, Outside, Inside);

        bool AnyOutside = Outside;
        bool AllInside = Inside;

        IntersectTrianglePlane(TV0, TV1, TV2, Plane1, Outside, Inside);
        AnyOutside |= Outside;
        AllInside &= Inside;

        IntersectTrianglePlane(TV0, TV1, TV2, Plane2, Outside, Inside);
        AnyOutside |= Outside;
        AllInside &= Inside;

        IntersectTrianglePlane(TV0, TV1, TV2, Plane3, Outside, Inside);
        AnyOutside |= Outside;
        AllInside &= Inside;

        IntersectTrianglePlane(TV0, TV1, TV2, Plane4, Outside, Inside);
        AnyOutside |= Outside;
        AllInside &= Inside;

        IntersectTrianglePlane(TV0, TV1, TV2, Plane5, Outside, Inside);
        AnyOutside |= Outside;
        AllInside &= Inside;

        // If the triangle is outside any plane it is outside.
        if (AnyOutside) {
            return DISJOINT;
        }

        // If the triangle is inside all planes it is inside.
        if (AllInside) {
            return CONTAINS;
        }

        // The triangle is not inside all planes or outside a plane, it may intersect.
        return INTERSECTS;
    }

} // namespace TriangleTests