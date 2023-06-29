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

struct line {
	fVec2 p1;
	fVec2 p2;
};

float vectorLength(fVec2 vector);
fVec2 normalise(fVec2 vector);
fVec2 angleToVector(float angle);
float vectorToAngle(fVec2 vector);
fVec2 vectorToPoint(fVec2 source, fVec2 target);
float angleToPoint(fVec2 source, fVec2 target);
float crossProduct(const fVec2& a, const fVec2& b);
float distance(fVec2 p1, fVec2 p2);
bool intersect(line* a, line* b, fVec2* out);