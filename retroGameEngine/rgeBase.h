#pragma once
#include <chrono>
#include <vector>
#include "rgeTools.h"

const float pi = 3.14159265358979323846f;

/// <summary>
/// 
/// the end goal is to enable this to be easily ported to other graphics frameworks, d3d9 is not
/// a hard dependancy and anything that can take RGE->frameBuffer and show it on the screen somehow should be
/// capable of hosting it.
/// 
/// planned elements:
/// 
/// -psuedo 3D renderer
///		-texture rendering
///		-light sources
///		-realistic camera
///		-sprites, walls, dynamic objects
///		-decals
/// 
/// -basic 2D renderer
///		-primitive shapes, lines, etc
///		-single font text
/// 
/// </summary>

namespace RGE {
	struct RGB {
		unsigned char R = 0;
		unsigned char G = 0;
		unsigned char B = 0;
	};

	class RGBA {

	public:

		//the order of these can be swapped around to match the byte order in the frame buffer
		unsigned char B = 0;
		unsigned char G = 0;
		unsigned char R = 0;
		unsigned char A = 255;
		
		RGBA(int R, int G, int B, int A = 255) {
			R = R < 255 ? R : 255;
			R = R > 0 ? R : 0;
			this->R = R;

			G = G < 255 ? G : 255;
			G = G > 0 ? G : 0;
			this->G = G;

			B = B < 255 ? B : 255;
			B = B > 0 ? B : 0;
			this->B = B;

			A = A < 255 ? A : 255;
			A = A > 0 ? A : 0;;
			this->A = A;
		}

		RGBA(float R, float G, float B, float A = 1.f) {
			int iR = (int)(R * 255.f);
			iR = iR < 255 ? iR : 255;
			iR = iR > 0 ? iR : 0;
			this->R = iR;

			int iG = (int)(G * 255.f);
			iG = iG < 255 ? iG : 255;
			iG = iG > 0 ? iG : 0;
			this->G = iG;

			int iB = (int)(B * 255.f);
			iB = iB < 255 ? iB : 255;
			iB = iB > 0 ? iB : 0;
			this->B = iB;

			int iA = (int)(A * 255.f);
			iA = iA < 255 ? iA : 255;
			iA = iA > 0 ? iA : 0;;
			this->A = iA;
		}

		RGBA() {
			this->R = 0;
			this->G = 0;
			this->B = 0;
			this->A = 255;
		}
	};

	enum textureMode {
		stretch = 0,
		tile = 1
	};

	struct RGETexture {
		int textureID;
		RGBA* data;
		int dataSize;
		int X;
		int Y;
		textureMode mode = textureMode::tile;

		RGBA fSample(fVec2 pos) {
			if (pos.X < 0.f || pos.X > 1.f) { return RGBA(1.f, 0.f, 0.f); }
			if (pos.Y < 0.f || pos.Y > 1.f) { return RGBA(1.f, 0.f, 0.f); }

			int iX = (pos.X * (float)X);
			int iY = (pos.Y * (float)Y);

			return data[(iY * X) + iX];
		}
	};

	struct line {
		fVec2 p1;
		fVec2 p2;
	};

	struct wall {
		line line;
		RGBA colour;
		int textureID = 0;
	};

	struct raycastImpact {
		bool valid;
		float distanceFromOrigin;
		float distanceFromLineOrigin = -1.f;
		float trueDistanceFromLineOrigin = -1.f;
		wall surface;
		RGBA surfaceColour;
		fVec2 position;

		~raycastImpact() {
			//delete[] surfaceColour;
		}
	};

	struct raycastResponse {
		int impactCount = 0;
		std::vector<raycastImpact> impacts;
	};

	class RGEPlayer {
	public:
		float angle = pi;
		fVec2 position = { 0.f, 0.f };

		float cameraMaxDistance = 5000.f;
		float cameraFocal = 0.5f;
		float cameraLumens = 5.f;
		float cameraCandella = 20000.f;
		float cameraFov = (pi / 2.f) / 1.75f;

		bool angleWithinFov(float angle);
	};

	class RGEMap {
	private:
		std::vector<wall> staticElements = {};

	public:

		std::vector<wall> build() {
			std::vector<wall> response = {};
			for (wall& w : staticElements) {
				response.push_back(w);
			}

			return response;
		}

		void addStaticWall(wall w) {
			staticElements.push_back(w);
		}
	};

	class RGEngine {
	private:
		
		//frame buffer related
		RGBA* frameBuffer = 0;
		iVec2 frameBufferSize = { 0, 0 };

		//clock related
		std::chrono::steady_clock::time_point begin;
		std::chrono::steady_clock::time_point end;
		int frameTimeMicros = 0;

		//font renderer related
		RGBA* fontData = 0;
		RGBA white = { 255, 255, 255, 255 };
		RGBA black = { 0,   0,   0, 255 };
		iVec2 fontMapSize = { 144, 101 };
		const int charX = 8;
		const int charY = 16;
		
		void initializeFontRenderer();

	public:

		//user accessable elements
		RGEPlayer* plr = new RGEPlayer;
		RGEMap* map = new RGEMap;
		RGETexture* textureMap[128] = {};

		//basic frameBuffer drawing
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

		void frameTimerBegin() {
			begin = std::chrono::steady_clock::now();
		}
		
		void frameTimerEnd() {
			end = std::chrono::steady_clock::now();
			frameTimeMicros = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();	
		}

		float getFps() {
			return (1000.f / ((float)frameTimeMicros / 1000.f));
		}

		int getFrameTimeMicros() {
			return frameTimeMicros;
		}
		
		//primitive drawing
		void fillFrameBuffer(RGBA colour) {
			int size = frameBufferSize.X * frameBufferSize.Y;
			for (int i = 0; i < size; i++) {
				frameBuffer[i] = colour;
			}
		}

		void frameBufferDrawPixel(fVec2 location, RGBA colour);
		void frameBufferDrawLine(fVec2 p1, fVec2 p2, RGBA colour);
		void frameBufferDrawRect(fVec2 p1, fVec2 p2, RGBA colour);
		void frameBufferFillRect(fVec2 p1, fVec2 p2, RGBA colour);
		void frameBufferDrawCircle(fVec2 center, int radius, RGBA colour);
		//void frameBufferFillCircle(iVec2 center, int radius, RGBA colour);

		//font renderer drawing
		int fontRendererDrawGlyph(fVec2 position, char c, int scale);
		int fontRendererDrawString(fVec2 position, const char* text, int scale);
		int fontRendererDrawSpacer(fVec2 position, int width, int scale);
		
		//raycast functions
		bool castRay(fVec2 origin, float angle, float correctionAngle, float maxLength, raycastResponse* responseData);

		//texture functions
		void initDefaultTexture();
		void initTextureFromDisk(const char* path, textureMode mode, int textureIndex);
		void frameBufferFillRectSegmented(fVec2 p1, fVec2 p2, RGETexture* texture, float offset, float brightnessModifier);

		RGEngine() {
			initializeFontRenderer();
			initDefaultTexture();
		}
	};
}

