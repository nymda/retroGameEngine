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

float distance(fVec2 p1, fVec2 p2) {
    return sqrt(pow(p2.X - p1.X, 2) + pow(p2.Y - p1.Y, 2));
}
