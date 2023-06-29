#include "map.h"
#include "rgeTools.h"

fVec2 mapOffset = { 0, 0 };
fVec2 mapScale = { 1, 1 };
fVec2 mapStartPan = { 0, 0 };
fVec2 mousePos = { -1, -1 };

bool mDown = false;
fVec2 panStart = mousePos;
bool mapCameraMovement = false;

fVec2 mapSelectedNode = { -1.f, -1.f };
mapState mState = mapState::hunting;
int mapTextureIndex = 0;
float mapGridDensity = 0.01f;

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

void setOffset(fVec2 newOffset) {
    mapOffset = newOffset;
}

void updateMousePos(fVec2 newMousePos) {
	mousePos = newMousePos;
}

void mapTick() {
    if (GetKeyState(VK_LSHIFT) < 0) {
        mapCameraMovement = true;
    }
    else {
        mapCameraMovement = false;
    }

    if (GetKeyState(VK_LBUTTON) < 0 && mapCameraMovement) {

        if (!mDown)
        {
            mDown = true;
            panStart = mousePos;
        }

        mapOffset.X -= (mousePos.X - panStart.X) / mapScale.X;
        mapOffset.Y -= (mousePos.Y - panStart.Y) / mapScale.Y;

        panStart.X = mousePos.X;
        panStart.Y = mousePos.Y;
    }
    else {
        mDown = false;
    }
}

void drawPlayer(RGE::RGEngine* engine) {
    float angleMin = fmod(engine->plr->angle - (engine->plr->cameraFov / 2.f), 2 * pi);
    float angleMax = fmod(engine->plr->angle + (engine->plr->cameraFov / 2.f), 2 * pi);
    
	fVec2 vectorMin = normalise(angleToVector(angleMin));
    fVec2 vectorMax = normalise(angleToVector(angleMax));

    vectorMin.X *= 250.f;
	vectorMin.Y *= 250.f;
	vectorMax.X *= 250.f;
	vectorMax.Y *= 250.f;
    
	fVec2 targetMin = { engine->plr->position.X + vectorMin.X, engine->plr->position.Y + vectorMin.Y };
	fVec2 targetMax = { engine->plr->position.X + vectorMax.X, engine->plr->position.Y + vectorMax.Y };
    
	fVec2 screenMin = w2s(targetMin);
	fVec2 screenMax = w2s(targetMax);    
	fVec2 playerPos = w2s(engine->plr->position);

	engine->frameBufferDrawLine(playerPos, screenMin, RGE::RGBA(1.f, 0.f, 0.f));
	engine->frameBufferDrawLine(playerPos, screenMax, RGE::RGBA(1.f, 0.f, 0.f));
}

void drawMap(RGE::RGEngine* engine) {
    fVec2 screenMin = s2w({ 0.f, 0.f });
    fVec2 screenMax = s2w({ (float)engine->getFrameBufferSize().X, (float)engine->getFrameBufferSize().Y});

    float stepSize = 1.f / mapGridDensity;

    int startX = ceil(screenMin.X / stepSize) * stepSize;
    int startY = ceil(screenMin.Y / stepSize) * stepSize;

    fVec2 mouseWorld = s2w(mousePos);
    fVec2 test = w2s(mouseWorld);

    fVec2 nearby = { -1, -1 };
    float distToNearby = 100000.f;
    bool hasNearby = false;

    for (float x = startX; x < screenMax.X; x += stepSize) {
        for (float y = startY; y < screenMax.Y; y += stepSize) {
            fVec2 point = { x, y };

            float distToPoint = abs(distance(mouseWorld, point));
            if (distToPoint < distToNearby) {
                distToNearby = distToPoint;
                nearby = point;
                hasNearby = true;
            }
    
            fVec2 screenPoint = w2s(point);
            engine->frameBufferDrawPixel(screenPoint, RGE::RGBA(1.f, 1.f, 1.f));
        }
    }

    if (hasNearby && !mapCameraMovement) {
        fVec2 screenPoint = w2s(nearby);
        engine->frameBufferFillRect({ screenPoint.X - 5.f, screenPoint.Y - 5.f }, { screenPoint.X + 5.f,  screenPoint.Y + 5.f }, RGE::RGBA(1.f, 0.f, 0.f));
    }

    if (!mapCameraMovement) {
        if (GetKeyState(VK_LBUTTON) < 0) {
            if (mState == mapState::hunting && hasNearby) {
                mapSelectedNode = nearby;
                mState = mapState::drawing;
            }
            else if (mState == mapState::drawing && hasNearby && !(mapSelectedNode.X == nearby.X && mapSelectedNode.Y == nearby.Y)) {

                RGE::wall newWall;
                newWall.line.p1 = mapSelectedNode;
                newWall.line.p2 = nearby;
                newWall.colour = RGE::RGBA(1.f, 1.f, 1.f);
                newWall.textureID = mapTextureIndex;
                engine->map->addStatic(newWall);

                mState = mapState::hunting;
            }
        }
    }

	if (GetKeyState(VK_ESCAPE) < 0) {
		mapSelectedNode = { -1.f, -1.f };
		mState = mapState::hunting;
	}

	if (mState == mapState::drawing) {
		fVec2 screenPoint = w2s(mapSelectedNode);
		engine->frameBufferFillRect({ screenPoint.X - 2.5f, screenPoint.Y - 2.5f }, { screenPoint.X + 2.5f,  screenPoint.Y + 2.5f }, RGE::RGBA(0.f, 0.f, 1.f));
	}

	//causes lag at very zoomed in zoom levels, idk
	for (RGE::wall& w : engine->map->build()) {
		fVec2 p1Int = { w.line.p1.X, w.line.p1.Y };
		fVec2 p2Int = { w.line.p2.X, w.line.p2.Y };
		engine->frameBufferDrawLine(w2s(p1Int), w2s(p2Int), w.colour);
	}

	for (RGE::RGESprite& s : engine->map->sprites) {
		fVec2 spritePos = w2s(s.position);
		if (spritePos.X < 0 || spritePos.X > 640 || spritePos.Y < 0 || spritePos.Y > 480) { continue; }
		engine->frameBufferFillRect({ spritePos.X - 2.5f, spritePos.Y - 2.5f }, { spritePos.X + 2.5f,  spritePos.Y + 2.5f }, RGE::RGBA(255, 153, 0));
	}

	drawPlayer(engine);

	engine->frameBufferFillRect({ mousePos.X - 2.5f, mousePos.Y - 2.5f }, { mousePos.X + 2.5f,  mousePos.Y + 2.5f }, RGE::RGBA(1.f, 1.f, 1.f));

	if (mapCameraMovement) {
		engine->fontRendererDrawString({ 5, 26 }, "PAN/ZOOM", 1);
	}
	else {
		engine->fontRendererDrawString({ 5, 26 }, "DRAW", 1);
		char texindex[32];
		sprintf_s(texindex, 32, "TEX: %i", mapTextureIndex);
		engine->fontRendererDrawString({ 5, 47 }, texindex, 1);
	}
}

void handleScrollEvent(WPARAM wParam) {
    if (mapCameraMovement) {
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
    }
}

void handleKeyEvent(WPARAM wParam) {
    if (wParam == VK_OEM_PLUS) {
        if (mapGridDensity < 1.f) { mapGridDensity += 0.01f; }

    }
    
    if (wParam == VK_OEM_MINUS) {
        if (mapGridDensity > 0.f) {
            mapGridDensity -= 0.01f;
        }
    }
    
    if (wParam == VK_OEM_6) {
        mapTextureIndex++;
        if (mapTextureIndex > 128) { mapTextureIndex -= 128; }
    }

    if (wParam == VK_OEM_4) {
        mapTextureIndex--;
        if (mapTextureIndex < 0) { mapTextureIndex += 128; }
    }
}