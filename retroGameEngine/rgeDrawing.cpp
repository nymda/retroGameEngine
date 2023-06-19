#include "rgeDrawing.h"
#include <cstdlib>

void RGE::RGEngine::frameBufferDrawPixel(iVec2 location, RGBA colour)
{
	int pLocation = (location.Y * this->getFrameBufferSize().X) + location.X;
	this->getFrameBuffer()[pLocation] = colour;
}


void RGE::RGEngine::frameBufferDrawLine(iVec2 p1, iVec2 p2, RGBA colour)
{
    short w = (short)(p2.X - p1.X);
    short h = (short)(p2.Y - p1.Y);
    short dx1 = 0;
    short dy1 = 0;
    short dx2 = 0;
    short dy2 = 0;
    if (w < 0) { dx1 = -1; }
    else if (w > 0) { dx1 = 1; };
    if (h < 0) { dy1 = -1; }
    else if (h > 0) { dy1 = 1; };
    if (w < 0) { dx2 = -1; }
    else if (w > 0) { dx2 = 1; };
    short longest = abs(w);
    short shortest = abs(h);
    if (!(longest > shortest))
    {
        longest = abs(h);
        shortest = abs(w);
        if (h < 0) { dy2 = -1; }
        else if (h > 0) { dy2 = 1; };
        dx2 = 0;
    }
    int numerator = longest >> 1;
    for (int i = 0; i <= longest; i++)
    {
        frameBufferDrawPixel(p1, colour);
        numerator += shortest;
        if (!(numerator < longest))
        {
            numerator -= longest;
            p1.X += dx1;
            p1.Y += dy1;
        }
        else
        {
            p1.X += dx2;
            p1.Y += dy2;
        }
    }
}
