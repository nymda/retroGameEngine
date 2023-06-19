#pragma once

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
		char R = 0;
		char G = 0;
		char B = 0;
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
		RGBA* frameBuffer = 0;
		iVec2 frameBufferSize = { 0, 0 };

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

		void fillFrameBuffer(RGBA colour) {
			int size = frameBufferSize.X * frameBufferSize.Y;
			for (int i = 0; i < size; i++) {
				frameBuffer[i] = colour;
			}
		}

		void frameBufferDrawPixel(iVec2 location, RGBA colour);
		void frameBufferDrawLine(iVec2 p1, iVec2 p2, RGBA colour);

		RGEngine() {

		}
	};
}

