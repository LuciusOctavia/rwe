#ifndef RWE_GEOMETRY_RAY3F_H
#define RWE_GEOMETRY_RAY3F_H

#include <rwe/math/Vector3f.h>

namespace rwe
{
    /** An infinite ray in 3D space. */
    struct Ray3f
    {
        /** The starting point at the ray, at t = 0. */
        Vector3f origin;

        /** The direction the ray is travelling. */
        Vector3f direction;

        Ray3f(const Vector3f& position, const Vector3f& direction);

        /** Returns the position along the ray at time t. */
        Vector3f pointAt(float t) const;

        /**
         * Returns true if point a is less far along the ray than point b.
         * a and b are both assumed to lie on the ray.
         */
        bool isLessFar(const Vector3f& a, const Vector3f& b) const;
    };
}

#endif