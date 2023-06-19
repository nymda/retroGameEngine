#pragma once

#define pi 3.14159265358979323846f


struct RGB {
	char R = 0;
	char G = 0;
	char B = 0;
};

struct RGBA {
	char R = 0;
	char G = 0;
	char B = 0;
	char A = 255;
};

struct fVec2 {
	float X;
	float Y;
};

struct iVec2 {
	int X;
	int Y;
};

struct player {
	float angle = pi / 2.f;
	fVec2 position = { 0.f, 0.f };

	float cameraFocal = 0.5f;
	float cameraLumens = 5.f;
	float cameraCandella = 10000.f;
};

class RGE {
private:
	RGBA* frameBuffer = 0;
	iVec2 frameBufferSize = {0, 0};

public:
	player plr = {};

	bool allocFrameBuffer(iVec2 size) {
		frameBuffer = new RGBA[size.X * size.Y];
		if (!frameBuffer) { return false; }
		frameBufferSize = size;
		return true;
	}

	RGBA* getFrameBuffer() {
		return frameBuffer;
	}

	iVec2 getFrameBufferSize() {
		return frameBufferSize;
	}

	RGE() {

	}
};