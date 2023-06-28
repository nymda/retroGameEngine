#pragma once
#include <iostream>
#include <dinput.h>
#include <tchar.h>
#include "rgeBase.h"

enum mapState {
    hunting = 0,
    moving = 1,
    drawing = 2,
    deleting = 3
};

void drawMap(RGE::RGEngine* engine);
void handleScrollEvent(WPARAM wParam);
void handleKeyEvent(WPARAM wParam);
void updateMousePos(fVec2 newMousePos);
void mapTick();