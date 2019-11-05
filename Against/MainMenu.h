#pragma once

#include <stdint.h>

int CreateMainMenuGraphics ();
int DrawMainMenu ();
void DestroyMainMenuGraphics ();

void MainMenuProcessMouseMovement (uint32_t X, uint32_t Y, uint32_t DeltaX, uint32_t DeltaY);