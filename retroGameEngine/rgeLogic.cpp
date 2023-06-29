#include "rgeBase.h"
#include <algorithm>

bool comp(RGE::raycastImpact& a, RGE::raycastImpact& b) {
    return a.distanceFromOrigin > b.distanceFromOrigin;
}


bool compSprite(RGE::RGESprite& a, RGE::RGESprite& b) {
    return a.currentDistanceFromOrigin > b.currentDistanceFromOrigin;
}

void RGE::RGEngine::recalculateSpriteDistances() {
    for (RGESprite& s : this->map->sprites) {
        s.currentDistanceFromOrigin = distance(s.position, this->plr->position);
    }
    std::sort(this->map->sprites.begin(), this->map->sprites.end(), compSprite);
}

std::vector<RGE::wall> world = {};
bool RGE::RGEngine::castRay(fVec2 origin, float angle, float correctionAngle, float maxDistance, bool refreshWorld, raycastResponse* responseData) {
    if (!responseData) { return false; }
    if (refreshWorld) { world = this->map->build(); }

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

            rci.trueDistanceFromLineOrigin = distance(hit, w.line.p1);
            rci.distanceFromLineOrigin = rci.trueDistanceFromLineOrigin / distance(w.line.p1, w.line.p2);

            responseData->impacts.push_back(rci);
            impact = true;
        }
    }

    if (responseData->impactCount > 1) {
        std::sort(responseData->impacts.begin(), responseData->impacts.end(), comp);
    }

    return impact;
}