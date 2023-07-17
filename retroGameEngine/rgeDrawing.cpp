#include "rgeBase.h"
#include "fontData.h"
#include <cstdlib>
#include <string.h>

RGE::RGBA blendColours(RGE::RGBA lower, RGE::RGBA upper) {

	float lR = ((float)lower.R / 255.f);
	float lG = ((float)lower.G / 255.f);
	float lB = ((float)lower.B / 255.f);
    
	float uR = ((float)upper.R / 255.f);
	float uG = ((float)upper.G / 255.f);
	float uB = ((float)upper.B / 255.f);
    
    float bf = ((float)upper.A / 255.f);
    float ibf = 1.f - bf;

    float pR = (lR * ibf + uR * bf);
    float pG = (lG * ibf + uG * bf);
    float pB = (lB * ibf + uB * bf);
    float pA = fClamp(0.f, 1.f, lower.A + upper.A);

    return RGE::RGBA(pR, pG, pB, pA);
}

void RGE::RGEngine::frameBufferDrawPixel(fVec2 location, RGBA colour)
{
    iVec2 fbSize = this->getFrameBufferSize();
    if (location.X < 0 || location.Y < 0 || location.X > (fbSize.X - 1) || location.Y > (fbSize.Y - 1)) { return; }

	int pLocation = (location.Y * fbSize.X) + location.X;

    if (pLocation > (fbSize.X * fbSize.Y)) { return; }
    if (pLocation < 0) { return; }

	this->getFrameBuffer()[pLocation] = colour;
}

void RGE::RGEngine::frameBufferDrawLine(fVec2 p1, fVec2 p2, RGBA colour)
{
    p1 = { (float)floor(p1.X), (float)floor(p1.Y) };
    p2 = { (float)floor(p2.X), (float)floor(p2.Y) };

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

void RGE::RGEngine::frameBufferDrawRect(fVec2 p1, fVec2 p2, RGBA colour)
{
    p1.X = fmax(p1.X, 0.f);
    p1.Y = fmax(p1.Y, 0.f);
    p2.X = fmax(p2.X, 0.f);
    p2.Y = fmax(p2.Y, 0.f);

    p1.X = fmin(p1.X, frameBufferSize.X);
    p1.Y = fmin(p1.Y, frameBufferSize.Y);
    p2.X = fmin(p2.X, frameBufferSize.X);
    p2.Y = fmin(p2.Y, frameBufferSize.Y);

	frameBufferDrawLine(p1, { p2.X, p1.Y }, colour);
	frameBufferDrawLine({ p2.X, p1.Y }, p2, colour);
	frameBufferDrawLine(p2, { p1.X, p2.Y }, colour);
	frameBufferDrawLine({ p1.X, p2.Y }, p1, colour);
}

void RGE::RGEngine::frameBufferFillRect(fVec2 p1, fVec2 p2, RGBA colour) {
    fVec2 min = p1;
    fVec2 max = p2;
    
	if (p1.X > p2.X)
	{
		min.X = p2.X;
		max.X = p1.X;
	}
    
	if (p1.Y > p2.Y)
	{
		min.Y = p2.Y;
		max.Y = p1.Y;
	}  

	if (max.X < 0 || max.Y < 0 || min.X > frameBufferSize.X || min.Y > frameBufferSize.Y) { return; }
    
    min.X = fmax(min.X, 0.f);
    min.Y = fmax(min.Y, 0.f);
    max.X = fmax(max.X, 0.f);
    max.Y = fmax(max.Y, 0.f);

    min.X = fmin(min.X, frameBufferSize.X);
    min.Y = fmin(min.Y, frameBufferSize.Y);
    max.X = fmin(max.X, frameBufferSize.X);
    max.Y = fmin(max.Y, frameBufferSize.Y);

	for (int y = min.Y; y <= max.Y; y++)
	{
		for (int x = min.X; x <= max.X; x++)
		{
            if (colour.A < 255) {
                frameBuffer[(y * frameBufferSize.X) + x] = blendColours(frameBuffer[(y * frameBufferSize.X) + x], colour);
            }
            else {
                frameBuffer[(y * frameBufferSize.X) + x] = colour;
            }

		}
	}
}

void RGE::RGEngine::frameBufferDrawCircle(fVec2 center, int radius, RGBA colour) {
    int sides = 32;
	float theta = 0.f;
	float dTheta = (2.f * 3.14159265359f) / sides;
	for (int i = 0; i < sides; i++)
	{
        fVec2 p1 = { (center.X + (radius * cos(theta))), (center.Y + (radius * sin(theta))) };
        fVec2 p2 = { (center.X + (radius * cos(theta + dTheta))), (center.Y + (radius * sin(theta + dTheta))) };
		frameBufferDrawLine(p1, p2, colour);
		theta += dTheta;
	}
}

int RGE::RGEngine::fontRendererDrawGlyph(fVec2 position, char c, float scale) {
    int cmIndex = 0;

    float xStep = (float)charX / ((float)charX * scale);
    float yStep = (float)charY / ((float)charY * scale);

    float xSize = ((float)charX * scale);
    float ySize = ((float)charY * scale);

    for (char cm : fontMap) {
        if(cm != c){ cmIndex++; continue; }
        break;
    }
    if(cmIndex >= sizeof(fontMap)){ cmIndex = 0; }

    iVec2 map = { cmIndex % charY, cmIndex / charY };
    iVec2 mapPosExpanded = { (map.X * charX) + ((map.X * charX) / charX), (map.Y * charY) + ((map.Y * charY) / charY) };
    iVec2 mapPosEnd = { mapPosExpanded.X + charX, mapPosExpanded.Y + charY };

    fVec2 samplePos = { 0.f, 0.f };
    for (int y = 0; y < (int)(round(ySize)); y++) {
        samplePos.X = 0.f;
        for (int x = 0; x < (int)(round(xSize)); x++) {

            int sX = (mapPosExpanded.X + samplePos.X);
            int sY = (mapPosExpanded.Y + samplePos.Y);

            int fontOffset = ((sY * fontMapSize.X) + sX);

            frameBufferDrawPixel({ position.X + x, position.Y + y }, fontData[fontOffset]);

            samplePos.X += xStep;
        }
        samplePos.Y += yStep;
    }

    return charX * scale;
}

int RGE::RGEngine::fontRendererDrawSpacer(fVec2 position, int width, float scale) {

    for (int y = 0; y < (int)round(charY * scale); y++) {
        for (int x = 0; x < (int)round(width * scale); x++) {
            int drawIndex = ((position.Y + y) * frameBufferSize.X) + (position.X + x);
            if (drawIndex <= (frameBufferSize.X * frameBufferSize.Y) && drawIndex >= 0) {
                frameBuffer[drawIndex] = black;
            }
        }
    }

    return width * scale;
}

int RGE::RGEngine::fontRendererDrawString(fVec2 position, const char* text, float scale) {
    if (!frameBuffer) { return 0; }

    //current offset from the origins X
    int offset = 0;

    //width of the spaces between characters in subpixels
    int spacerWidth = 2;

    //draw the background rectangle	
    offset += fontRendererDrawSpacer(position, spacerWidth, scale);

    //draw the required characters with 1px between them
    for (int i = 0; i < strlen(text); i++) {
        if ((position.X + offset + (charX * scale)) > frameBufferSize.X || text[i] == L';') {
            position.Y += charY * scale;
            offset = fontRendererDrawSpacer(position, spacerWidth, scale);

            if (text[i] == L';') { continue; }
        }

        offset += fontRendererDrawGlyph({ position.X + offset, position.Y }, text[i], scale);
        offset += fontRendererDrawSpacer({ position.X + offset, position.Y }, spacerWidth, scale);
    }

    return 0;
}


void RGE::RGEngine::initializeFontRenderer() {
    RGBA white = { 255, 255, 255, 255 };
    RGBA black = { 0,   0,   0, 255 };
    
    size_t size = sizeof(compressedFontData);
    char* compressed = (char*)compressedFontData;
    
    fontData = new RGBA[size * 8];
    
    for (int i = 0; i < size; i++) {
        for (int b = 0; b < 8; b++) {
            fontData[(i * 8) + b] = (compressed[i] & (1 << b)) ? white : black;
        }
    }
}

bool isAngleBetween(float angle, float startAngle, float endAngle) {
    // Calculate angular distances
    float distance1 = fmod(endAngle - startAngle + 2 * pi, 2 * pi);
    float distance2 = fmod(angle - startAngle + 2 * pi, 2 * pi);

    // Compare angular distances
    return distance2 <= distance1;
}


bool RGE::RGEPlayer::angleWithinFov(float a) {
    float aNorm = fmod(a, 2 * pi);
    float angleMin = fmod(this->angle - (this->cameraFov / 2.f), 2 * pi);
    float angleMax = fmod(this->angle + (this->cameraFov / 2.f), 2 * pi);

    float distance1 = fmod(angleMax - angleMin + 2 * pi, 2 * pi);
    float distance2 = fmod(aNorm - angleMin + 2 * pi, 2 * pi);

    return distance2 <= distance1;
}