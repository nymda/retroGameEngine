#include "rgeBase.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <dinput.h>

void RGE::RGEngine::initDefaultTexture() {
	RGETexture* nt = new RGETexture;

	int defaultTextureRes = 8;

	nt->data = (RGBA*)malloc(sizeof(RGBA) * (defaultTextureRes * defaultTextureRes));
	if (!nt->data) { return; }

	RGBA p = RGBA(255, 0, 255);
	RGBA b = RGBA(0, 0, 0);

	bool swap = true;
	for (int i = 0; i < (defaultTextureRes * defaultTextureRes); i++) {
		swap = !swap;
		if (i % 32 == 0) { swap = !swap; }
		nt->data[i] = swap ? p : b;
	}

	nt->X = defaultTextureRes;
	nt->Y = defaultTextureRes;
	nt->dataSize = sizeof(RGBA) * (defaultTextureRes * defaultTextureRes);
	nt->mode = textureMode::tile;

	for (int i = 0; i < 128; i++) {
		this->textureMap[i] = nt;
	}
}

void RGE::RGEngine::initTextureFromDisk(const char* path, textureMode mode, int textureIndex) {
	if (textureIndex == 0) { return; }
	if (textureIndex > 127) { return; }

	wchar_t execPath[512] = {};
	char execPathShort[512] = {};

	GetModuleFileName(NULL, (LPWSTR)&execPath, 512);

	int lastSlashIndex = 0;
	for (int i = 0; i < 512; i++) {
		if (execPath[i] == L'\\') {
			lastSlashIndex = i + 1;
		}
	}

	FillMemory(execPath + lastSlashIndex, 512 - lastSlashIndex, 0x00);

	size_t cSize = 0;
	wcstombs_s(&cSize, (char*)execPathShort, 512, execPath, 512);

	RGETexture* nt = new RGETexture;

	char tmp[512] = {};
	sprintf_s(tmp, "%s%s", execPathShort, path);

	int tChannels = 0;
	RGB* tmpBfr = (RGB*)stbi_load(tmp, &nt->X, &nt->Y, &tChannels, 3);
	if (!tmpBfr) { return; }
	
	int pixelCount = nt->X * nt->Y;

	nt->data = new RGBA[pixelCount];
	for (int i = 0; i < pixelCount; i++) {
		nt->data[i] = RGBA(tmpBfr[i].R, tmpBfr[i].G, tmpBfr[i].B);
	}
	delete[] tmpBfr;

	nt->dataSize = sizeof(RGBA) * (nt->X * nt->Y);
	nt->mode = mode;

	this->textureMap[textureIndex] = nt;
}

void RGE::RGEngine::frameBufferFillRectSegmented(fVec2 p1, fVec2 p2, RGETexture* texture, float offset, float brightnessModifier) {
	
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
	
	int textureY = texture->Y;

	float lineSizeY = (float)max.Y - (float)min.Y;
	float segmentSizeY = lineSizeY / (float)textureY;

	for (int i = 0; i < textureY; i++) {
		fVec2 segmentMin = { (min.X), (min.Y + (segmentSizeY * (float)i)) };
		fVec2 segmentMax = { (max.X), (min.Y + (segmentSizeY * (float)(i + 1.f))) };

		float yLvl = i * (1.f / (float)textureY);
		RGBA colour = texture->fSample({ offset, yLvl });

		float newR = (float)((float)colour.R / 256.f) * brightnessModifier;
		float newG = (float)((float)colour.G / 256.f) * brightnessModifier;
		float newB = (float)((float)colour.B / 256.f) * brightnessModifier;

		frameBufferFillRect(segmentMin, segmentMax, RGBA(newR, newG, newB));
	}
}