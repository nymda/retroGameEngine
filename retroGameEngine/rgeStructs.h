#pragma once
#include <chrono>

#define pi 3.14159265358979323846f

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
		char R = 0;
		char G = 0;
		char B = 0;
	};

	struct RGBA {

		//the order of these can be swapped around to match the byte order in the frame buffer
		char B = 0;
		char G = 0;
		char R = 0;
		char A = 255;
		
		RGBA(int R, int G, int B, int A = 255) {
			this->R = R;
			this->G = G;
			this->B = B;
			this->A = A;
		}

		RGBA() {
			this->R = 0;
			this->G = 0;
			this->B = 0;
			this->A = 255;
		}

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
		int charX = 8;
		int charY = 16;
		
		void initializeFontRenderer();

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
		
		void fillFrameBuffer(RGBA colour) {
			int size = frameBufferSize.X * frameBufferSize.Y;
			for (int i = 0; i < size; i++) {
				frameBuffer[i] = colour;
			}
		}

		void frameBufferDrawPixel(iVec2 location, RGBA colour);
		void frameBufferDrawLine(iVec2 p1, iVec2 p2, RGBA colour);
		void frameBufferDrawRect(iVec2 p1, iVec2 p2, RGBA colour);
		void frameBufferFillRect(iVec2 p1, iVec2 p2, RGBA colour);
		
		int fontRendererDrawGlyph(RGE::iVec2 position, char c, int scale);
		int fontRendererDrawString(RGE::iVec2 position, const char* text, int scale);
		int fontRendererDrawSpacer(RGE::iVec2 position, int width, int scale);
		
		RGEngine() {
			initializeFontRenderer();
		}
	};
}

