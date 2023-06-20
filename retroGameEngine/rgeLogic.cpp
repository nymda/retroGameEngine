#include "rgeBase.h"
#include <algorithm>

bool intersect(RGE::line* a, RGE::line* b, fVec2* out) {
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

            float tMin = std::min(t0, t1);
            float tMax = std::max(t0, t1);

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

bool comp(RGE::raycastImpact a, RGE::raycastImpact b) {
    return a.distanceFromOrigin < b.distanceFromOrigin;
}

bool RGE::RGEngine::castRay(fVec2 origin, float angle, float correctionAngle, float maxDistance, raycastResponse* responseData) {
    if (!responseData) { return false; }

	std::vector<wall> world = this->map->build();
    fVec2 target = { origin.X + (cos(angle) * maxDistance), origin.Y + (sin(angle) * maxDistance) };
    line ray = { origin, target };
    bool impact = false;

    for (wall& w : world) {
        fVec2 hit = { -1, -1 };
        if (intersect(&ray, &w.line, &hit)) {
            float distanceToHit = abs(sqrt(pow(origin.X - hit.X, 2) + pow(origin.Y - hit.Y, 2)));
            distanceToHit *= cos(correctionAngle - angle);
            responseData->impactCount++;

            raycastImpact rci = {};
            rci.valid = true;
            rci.distanceFromOrigin = distanceToHit;
            rci.surface = w;
            rci.surfaceColour = w.colour;
            rci.position = hit;

            responseData->impacts.push_back(rci);
            impact = true;
        }
    }

    std::sort(responseData->impacts.begin(), responseData->impacts.end(), comp);

    return impact;
}