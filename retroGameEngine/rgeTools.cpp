#include <limits>
#include <iostream>
#include <dinput.h>
#include "rgeTools.h"

float vectorLength(fVec2 vector) {
    return sqrt(vector.X * vector.X + vector.Y * vector.Y);
}

fVec2 normalise(fVec2 vector) {
    float l = sqrt(vector.X * vector.X + vector.Y * vector.Y);
    return { vector.X / l, vector.Y / l };
}

fVec2 angleToVector(float angle) {
    return { (float)cos(angle), (float)sin(angle) };
}

float vectorToAngle(fVec2 vector) {
    return atan2(vector.Y, vector.X);
}

fVec2 vectorToPoint(fVec2 source, fVec2 target) {
    fVec2 v = { target.X - source.X, target.Y - source.Y };
    return normalise(v);
}

float angleToPoint(fVec2 source, fVec2 target) {
    fVec2 v = vectorToPoint(source, target);
    return atan2(v.Y, v.X);
}

float crossProduct(const fVec2& a, const fVec2& b) {
    return a.X * b.Y - a.Y * b.X;
}

float dotProduct(const fVec2& a, const fVec2& b) {
	return a.X * b.X + a.Y * b.Y;
}

float distance(fVec2 p1, fVec2 p2) {
    return sqrt(pow(p2.X - p1.X, 2) + pow(p2.Y - p1.Y, 2));
}

bool intersect(line* a, line* b, fVec2* out) {
    fVec2 p = a->p1;
    fVec2 q = b->p1;
    fVec2 r = fVec2{ a->p2.X - a->p1.X, a->p2.Y - a->p1.Y };
    fVec2 s = fVec2{ b->p2.X - b->p1.X, b->p2.Y - b->p1.Y };

    float rCrossS = crossProduct(r, s);
    fVec2 qMinusP = fVec2{ q.X - p.X, q.Y - p.Y };

    // Check if the lines are parallel or coincident
    if (std::abs(crossProduct(qMinusP, r)) <= std::numeric_limits<float>::epsilon() && std::abs(crossProduct(qMinusP, s)) <= std::numeric_limits<float>::epsilon()) {
        // Check if the lines are collinear and overlapping
        if (crossProduct(qMinusP, r) <= std::numeric_limits<float>::epsilon()) {
            float t0 = (qMinusP.X * r.X + qMinusP.Y * r.Y) / (r.X * r.X + r.Y * r.Y);
            float t1 = t0 + (s.X * r.X + s.Y * r.Y) / (r.X * r.X + r.Y * r.Y);

            float tMin = min(t0, t1);
            float tMax = max(t0, t1);

            if (tMax >= 0.f && tMin <= 1.f) {
                out->X = p.X + tMin * r.X;
                out->Y = p.Y + tMin * r.Y;
                return true;
            }
        }

        return false;
    }

    float t = crossProduct(qMinusP, s) / rCrossS;
    float u = crossProduct(qMinusP, r) / rCrossS;

    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        out->X = p.X + t * r.X;
        out->Y = p.Y + t * r.Y;
        return true;
    }

    return false;
}

float calculateNormalAngle(line& l) {
    fVec2 normal = fVec2{ l.p2.X - l.p1.X, l.p2.Y - l.p1.Y };
    float angle = atan2(normal.Y, normal.X);
    angle += pi / 2.f;
    if (angle < 0) angle += pi * 2.f;
    return angle;
}