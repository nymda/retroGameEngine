#pragma once
#include <math.h>

struct fVec2 {
    float X;
    float Y;
};

struct iVec2 {
    int X;
    int Y;
};

float vectorLength(fVec2 vector);
fVec2 normalise(fVec2 vector);
fVec2 angleToVector(float angle);
fVec2 vectorToPoint(fVec2 source, fVec2 target);
float angleToPoint(fVec2 source, fVec2 target);
float crossProduct(const fVec2& a, const fVec2& b);
float distance(fVec2 p1, fVec2 p2);