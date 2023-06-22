#include <iostream>
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include "rgeBase.h"

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LPDIRECT3DSURFACE9 backbuffer;
IDirect3DSurface9* surface;
const RECT window = { 0, 0, 640, 480 };
D3DLOCKED_RECT draw;

RGE::RGEngine* engine = new RGE::RGEngine();
RGE::RGBA* frameBuffer = 0;
float lastFps = -1.f;

enum dispMode {
    map = 0,
    render = 1
};

HWND hWnd = 0;

dispMode mode = dispMode::render;
fVec2 playerVelocity = { 0, 0 };

float frameTotalBrightness = 0.f;
float frameBrightnessAverage = 0.f;
bool overExposure = false;

fVec2 mapOffset = { 0, 0 };
fVec2 mapScale = { 1, 1 };
fVec2 mapStartPan = { 0, 0 };
fVec2 mousePos = { -1, -1 };

POINT GetMousePos(HWND hWnd)
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    ScreenToClient(hWnd, &cursorPos);
    return cursorPos;
}

//fires 60 times per second
void timerCallback(HWND unnamedParam1, UINT unnamedParam2, UINT_PTR unnamedParam3, DWORD unnamedParam4) {
    if (GetKeyState(VK_LEFT) < 0) {
        engine->plr->angle -= (pi / 2.f) / 30.f;
        if (engine->plr->angle < 0.f) {
            engine->plr->angle += (pi * 2.f);
        }
    }

    if (GetKeyState(VK_RIGHT) < 0) {
        engine->plr->angle += (pi / 2.f) / 30.f;
        if (engine->plr->angle > (pi * 2.f)) {
            engine->plr->angle -= (pi * 2.f);
        }
    }

    if (GetKeyState(VK_UP) < 0) {
        fVec2 forward = angleToVector(engine->plr->angle);
        playerVelocity.X += forward.X * 5.f;
        playerVelocity.Y += forward.Y * 5.f;
    }

    if (GetKeyState(VK_DOWN) < 0) {
        fVec2 forward = angleToVector(engine->plr->angle);
        playerVelocity.X -= forward.X * 5.f;
        playerVelocity.Y -= forward.Y * 5.f;
    }

    if (GetKeyState(0x51) < 0) {
        fVec2 left = angleToVector(engine->plr->angle - (pi / 2.f));
        playerVelocity.X += left.X * 5.f;
        playerVelocity.Y += left.Y * 5.f;
    }

    if (GetKeyState(0x45) < 0) {
        fVec2 right = angleToVector(engine->plr->angle + (pi / 2.f));
        playerVelocity.X += right.X * 5.f;
        playerVelocity.Y += right.Y * 5.f;
    }

    //this mats is broken, but oh well

    if (vectorLength(playerVelocity) > 7.f) {
        playerVelocity = normalise(playerVelocity);
        playerVelocity.X *= 7.f;
        playerVelocity.Y *= 7.f;
    }

    engine->plr->position.X += playerVelocity.X;
    engine->plr->position.Y += playerVelocity.Y;

    playerVelocity.X -= playerVelocity.X / 5.f;
    playerVelocity.Y -= playerVelocity.Y / 5.f;;

    if (overExposure && engine->plr->cameraLumens > 0.5f) {
        engine->plr->cameraLumens -= 0.075f;
    }
    else if (frameBrightnessAverage < 0.25f && engine->plr->cameraLumens < 5.f) {
        engine->plr->cameraLumens += 0.075f;
    }
}

fVec2 w2s(fVec2 world) {
    fVec2 r = { 0, 0 };
	r.X = floorf((world.X - mapOffset.X) * mapScale.X);
	r.Y = floorf((world.Y - mapOffset.Y) * mapScale.Y);
	return r;
}

fVec2 s2w(fVec2 screen) {
	fVec2 r = { 0,  0 };
	r.X = (screen.X / mapScale.X) + mapOffset.X;
	r.Y = (screen.Y / mapScale.Y) + mapOffset.Y;
	return r;
}

void drawSegmentedTest(fVec2 p1, fVec2 p2, int segmentCount, RGE::RGBA colour) {

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

    float lineSizeY = (float)max.Y - (float)min.Y;
    float segmentSizeY = lineSizeY / (float)segmentCount;

    //this->frameBufferFillRect(min, max, colours[0]);

    for (int i = 0; i < segmentCount; i++) {
        fVec2 segmentMin = { (min.X), (min.Y + (segmentSizeY * i)) };
        fVec2 segmentMax = { (max.X), (min.Y + (segmentSizeY * (i + 1))) };

        engine->frameBufferFillRect(segmentMin, segmentMax, colour);
    }
}

void renderMap() {

    fVec2 screenMin = s2w({ 0.f, 0.f });
	fVec2 screenMax = s2w({ 640.f, 480.f });

    float gridDensity = 0.0200;
    float stepSize = 1.f / gridDensity;

    int startX = std::ceil(screenMin.X / stepSize) * stepSize;
    int startY = std::ceil(screenMin.Y / stepSize) * stepSize;
    
    for (float x = startX; x < screenMax.X; x += stepSize) {
        for (float y = startY; y < screenMax.Y; y += stepSize) {
            fVec2 point = { x, y };
            fVec2 screenPoint = w2s(point);
            engine->frameBufferDrawPixel(screenPoint, RGE::RGBA(1.f, 1.f, 1.f));
        }
    }
    
    //causes lag at very zoomed in zoom levels, idk
    for (RGE::wall& w : engine->map->build()) {
        fVec2 p1Int = { w.line.p1.X, w.line.p1.Y };
        fVec2 p2Int = { w.line.p2.X, w.line.p2.Y };
        engine->frameBufferDrawLine(w2s(p1Int), w2s(p2Int), w.colour);
    }
}

int main()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RGE"), NULL };
    ::RegisterClassEx(&wc);
    hWnd = ::CreateWindow(wc.lpszClassName, _T("RGE"), WS_OVERLAPPEDWINDOW, 100, 100, 640, 480, NULL, NULL, wc.hInstance, NULL);

    engine->allocFrameBuffer({ 640, 480 });
    frameBuffer = engine->getFrameBuffer();

    engine->initTextureFromDisk("bricktexture.png", RGE::textureMode::tile, 1);
    engine->initTextureFromDisk("concrete.png", RGE::textureMode::tile, 2);
    engine->initTextureFromDisk("gobid.png", RGE::textureMode::stretch, 3);

    mapOffset = { -640 / 2, -480 / 2 };
    
    RGE::wall T;
    T.line = { {-500, -500}, {500, -500} };
    T.colour = RGE::RGBA(255, 100, 100, 255);
    T.textureID = 1;

    RGE::wall B;
    B.line = { {-500, 500}, {500, 500} };
    B.colour = RGE::RGBA(100, 255, 100, 255);
    B.textureID = 1;

    RGE::wall L;
    L.line = { {-500, -500}, {-500, 500} };
    L.colour = RGE::RGBA(100, 100, 255, 255);
    L.textureID = 1;

    RGE::wall R;
    R.line = { {500, -500}, {500, 500} };
    R.colour = RGE::RGBA(255, 255, 255, 255);
    R.textureID = 1;

    engine->map->addStaticWall(T);
    engine->map->addStaticWall(B);
    engine->map->addStaticWall(L);
    engine->map->addStaticWall(R);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hWnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    SetTimer(NULL, 1, 1000 / 60, timerCallback);

    g_pd3dDevice->CreateOffscreenPlainSurface(640, 480, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

    g_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

    while (msg.message != WM_QUIT)
    {

        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        POINT cp = GetMousePos(hWnd);
        mousePos = { (float)cp.x, (float)cp.y };

        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(100, 100, 100, 255);
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            engine->frameTimerBegin();
            
            //drawing begin

            engine->fillFrameBuffer(RGE::RGBA(0, 0, 0));

            frameTotalBrightness = 0.f;
            frameBrightnessAverage = 0.f;
            overExposure = false;

            if (mode == dispMode::render) {
                int floorSegments = 50;
                float floorSegmentHeight = (640 / 2.f) / floorSegments;

                float floorDistanceMin = 5.f;
                float floorDistanceMax = 2000.f;

                for (int i = 0; i < floorSegments; i++) {
                    fVec2 floorMin = { 0, (480 / 2) + (floorSegmentHeight * (float)i) };
                    fVec2 floorMax = { 640, (480 / 2) + (floorSegmentHeight * (float)(i + 1)) };

                    float floorDistance = floorDistanceMax - (floorDistanceMin + ((floorDistanceMax - floorDistanceMin) / floorSegments) * i);
                    float floorBrightness = (engine->plr->cameraLumens / (floorDistance * floorDistance)) * engine->plr->cameraCandella;

                    RGE::RGBA segmentColour = RGE::RGBA(floorBrightness, floorBrightness, floorBrightness);
                    engine->frameBufferFillRect(floorMin, floorMax, segmentColour);
                    engine->frameBufferFillRect({ floorMin.X, (480 / 2) - (floorSegmentHeight * (float)(i + 1)) }, { floorMax.X, (480 / 2) - (floorSegmentHeight * (float)(i)) }, segmentColour);
                }
            }

            int rayCount = 320;
            for (int i = 0; i < rayCount; i++) {

                float FOV = (pi / 2.f) / 1.75f;
                float angleStep = FOV / rayCount;
                float offsetAngle = (engine->plr->angle - (FOV / 2.f)) + (angleStep * (float)i);

                RGE::raycastResponse hinf = {};

                if (engine->castRay(engine->plr->position, offsetAngle, engine->plr->angle, engine->plr->cameraMaxDistance, &hinf)) {
                    if (mode == dispMode::map) { engine->frameBufferDrawLine(w2s(engine->plr->position), w2s(hinf.impacts.back().position), hinf.impacts.back().surfaceColour); }
                }
                else {
                    fVec2 target = { engine->plr->position.X + (cos(offsetAngle) * engine->plr->cameraMaxDistance), engine->plr->position.Y + (sin(offsetAngle) * engine->plr->cameraMaxDistance) };
                    if (mode == dispMode::map) { engine->frameBufferDrawLine(w2s(engine->plr->position), w2s(target), RGE::RGBA(100, 100, 100)); }
                }

                if (mode == dispMode::render) {
                    if (hinf.impactCount == 0) { continue; }

                    for (RGE::raycastImpact imp : hinf.impacts) {
                        float dispHeight = engine->getFrameBufferSize().Y;
                        float apparentSize = dispHeight * engine->plr->cameraFocal / imp.distanceFromOrigin;
                        float height = apparentSize * dispHeight;

                        fVec2 barMin = { (2 * i), (dispHeight / 2) - (height / 2.f) };
                        fVec2 barMax = { (2 * i) + 1, (dispHeight / 2) + (height / 2.f) };

                        float brightness = (engine->plr->cameraLumens / (imp.distanceFromOrigin * imp.distanceFromOrigin)) * engine->plr->cameraCandella;
                        brightness = fmin(brightness, 1.5f);
                        brightness = fmax(brightness, 0.1f);

                        RGE::RGBA colour = RGE::RGBA((int)(brightness * (float)imp.surfaceColour.R), (int)(brightness * (float)imp.surfaceColour.G), (int)(brightness * (float)imp.surfaceColour.B));

                        RGE::RGETexture* currentTexture = engine->textureMap[imp.surface.textureID];

                        float trueDistance = 0;
                        float unused = 0;
                        if (currentTexture->mode == RGE::textureMode::tile) {
                            trueDistance = std::modf(imp.trueDistanceFromLineOrigin / (float)(currentTexture->X * 4.f), &unused);
                        }
                        else {
                            trueDistance = imp.distanceFromLineOrigin;
                        }

                        int tpo = trueDistance * currentTexture->X;
                        int dataOffset = (tpo * currentTexture->X);


                        //drawSegmentedTest(barMin, barMax, 1, colour);
                        engine->frameBufferFillRectSegmented(barMin, barMax, (RGE::RGBA*)(currentTexture->data + dataOffset), currentTexture->X, brightness);
                        //engine->frameBufferFillRect(barMin, barMax, colour);

                        frameTotalBrightness += brightness;
                        frameBrightnessAverage = frameTotalBrightness / i;
                        if (brightness > 1.f) {
                            overExposure = true;
                        }
                    }
                }
            }

            if (mode == dispMode::map) {
                renderMap();
            }

			char fps[32];
			sprintf_s(fps, 32, "FPS: %.2f", lastFps);      
			engine->fontRendererDrawString({ 5, 5 }, fps, 1);
            
            //drawing end

            //this takes up over half of the frame time
            
            surface->LockRect(&draw, &window, D3DLOCK_DISCARD);

            memcpy(draw.pBits, frameBuffer, (640 * 480) * sizeof(RGE::RGBA));

            surface->UnlockRect();

			g_pd3dDevice->StretchRect(surface, NULL, backbuffer, NULL, D3DTEXF_LINEAR);
     
            //i must be doing something wrong for it to be this slow
            
            engine->frameTimerEnd();
            lastFps = engine->getFps();
            
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {

        }
    }

    CleanupDeviceD3D();
    ::DestroyWindow(hWnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
        return false;
    }

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) {
        return false;
    }

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}


// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_MOUSEWHEEL:
        if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
            fVec2 mousePreZoom = s2w(mousePos);
            mapScale = { mapScale.X * 1.01f, mapScale.Y * 1.01f };
            fVec2 mousePostZoom = s2w(mousePos);
            mapOffset.X += mousePreZoom.X - mousePostZoom.X;
            mapOffset.Y += mousePreZoom.Y - mousePostZoom.Y;
        }
        else {
            fVec2 mousePreZoom = s2w(mousePos);
            mapScale = { mapScale.X * 0.99f, mapScale.Y * 0.99f };
            fVec2 mousePostZoom = s2w(mousePos);
            mapOffset.X += mousePreZoom.X - mousePostZoom.X;
            mapOffset.Y += mousePreZoom.Y - mousePostZoom.Y;
        }

        return 0;

    case WM_KEYDOWN:
        if (wParam == 0x4D)
        {
            mode = mode == map ? render : map;
        }

        return 0;
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

