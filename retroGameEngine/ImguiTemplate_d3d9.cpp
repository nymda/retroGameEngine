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

dispMode mode = dispMode::map;
fVec2 playerVelocity = { 0, 0 };

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
}

int main()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("RGE"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("RGE"), WS_OVERLAPPEDWINDOW, 100, 100, 640, 480, NULL, NULL, wc.hInstance, NULL);

    engine->allocFrameBuffer({ 640, 480 });
    frameBuffer = engine->getFrameBuffer();

    RGE::wall T;
    T.line = { {25, 25}, {640 - 25, 25} };
    T.colour = RGE::RGBA(255, 255, 255, 255);

    RGE::wall B;
    B.line = { {25, 480 - 25}, {640 - 25, 480 - 25} };
    B.colour = RGE::RGBA(255, 255, 255, 255);

    RGE::wall L;
    L.line = { {25, 25}, {25, 480 - 25} };
    L.colour = RGE::RGBA(255, 255, 255, 255);

    RGE::wall R;
    R.line = { {640 - 25, 25}, {640 - 25, 480 - 25} };
    R.colour = RGE::RGBA(255, 255, 255, 255);

    engine->map->addStaticWall(T);
    engine->map->addStaticWall(B);
    engine->map->addStaticWall(L);
    engine->map->addStaticWall(R);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

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

            if (mode == dispMode::map) {
                for (RGE::wall& w : engine->map->build()) {
                    iVec2 p1Int = { (int)w.line.p1.X, (int)w.line.p1.Y };
                    iVec2 p2Int = { (int)w.line.p2.X, (int)w.line.p2.Y };
                    engine->frameBufferDrawLine(p1Int, p2Int, w.colour);
                }
            }

            for (int i = 0; i < 320; i++) {

                float FOV = (pi / 2.f) / 1.75f;
                float angleStep = FOV / 320;
                float offsetAngle = (engine->plr->angle - (FOV / 2.f)) + (angleStep * (float)i);

                RGE::raycastResponse hinf = {};

                if (engine->castRay(engine->plr->position, offsetAngle, engine->plr->angle, 1000.f, &hinf)) {
                    engine->frameBufferDrawLine(engine->plr->position, hinf.impacts.front().position, hinf.impacts.front().surfaceColour);
                }
                else {
                    fVec2 target = { engine->plr->position.X + (cos(offsetAngle) * 1000.f), engine->plr->position.Y + (sin(offsetAngle) * 1000.f) };
                    engine->frameBufferDrawLine(engine->plr->position, target, RGE::RGBA(255, 0, 0));
                }
            }

            engine->frameBufferDrawRect({ 0, 0 }, { 639, 479 }, RGE::RGBA(0, 0, 0));

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
    ::DestroyWindow(hwnd);
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

