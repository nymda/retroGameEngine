#include "rgeDrawing.h"
#include "fontData.h"
#include <cstdlib>
#include <string.h>

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

void RGE::RGEngine::frameBufferDrawRect(iVec2 p1, iVec2 p2, RGBA colour)
{
	frameBufferDrawLine(p1, { p2.X, p1.Y }, colour);
	frameBufferDrawLine({ p2.X, p1.Y }, p2, colour);
	frameBufferDrawLine(p2, { p1.X, p2.Y }, colour);
	frameBufferDrawLine({ p1.X, p2.Y }, p1, colour);
}

void RGE::RGEngine::frameBufferFillRect(iVec2 p1, iVec2 p2, RGBA colour) {
    iVec2 min = p1;
    iVec2 max = p2;
    
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
    
	for (int y = min.Y; y <= max.Y; y++)
	{
		for (int x = min.X; x <= max.X; x++)
		{
            frameBuffer[(y * frameBufferSize.X) + x] = colour;
		}
	}
}

int RGE::RGEngine::fontRendererDrawGlyph(RGE::iVec2 position, char c, int scale) {
    int cmIndex = 0;

    for (wchar_t cm : fontMap) {
        if (cm == c) {

            //i cant remember what these do
            RGE::iVec2 map = { cmIndex % charY, cmIndex / charY };
            RGE::iVec2 mapPosExpanded = { (map.X * charX) + ((map.X * charX) / charX), (map.Y * charY) + ((map.Y * charY) / charY) };

            //pixel
            for (int y = 0; y < charY; y++) {
                for (int x = 0; x < charX; x++) {

                    //subpixel
                    for (int spY = 0; spY < scale; spY++) {
                        for (int spX = 0; spX < scale; spX++) {

                            //actual drawing
                            int canvasOffset = ((position.Y + (y * scale) + spY) * frameBufferSize.X) + (position.X + (x * scale) + spX);
                            int fontOffset = ((mapPosExpanded.Y + y) * fontMapSize.X) + (mapPosExpanded.X + x);
                            if ((canvasOffset <= (frameBufferSize.X * frameBufferSize.Y) && canvasOffset >= 0) && (fontOffset <= (fontMapSize.X * fontMapSize.Y) && fontOffset >= 0)) {
                                frameBuffer[canvasOffset] = fontData[fontOffset];
                            }
                        }
                    }
                }
            }

            break;
        }
        cmIndex++;
    }
    return charX * scale;
}

int RGE::RGEngine::fontRendererDrawSpacer(RGE::iVec2 position, int width, int scale) {

    for (int y = 0; y < charY * scale; y++) {
        for (int x = 0; x < width * scale; x++) {
            int drawIndex = ((position.Y + y) * frameBufferSize.X) + (position.X + x);
            if (drawIndex <= (frameBufferSize.X * frameBufferSize.Y) && drawIndex >= 0) {
                frameBuffer[drawIndex] = black;
            }
        }
    }

    return width * scale;
}

int RGE::RGEngine::fontRendererDrawString(RGE::iVec2 position, const char* text, int scale) {
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
            position.Y += 16 * scale;
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