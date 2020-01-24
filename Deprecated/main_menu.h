#pragma once

#include <stdint.h>
#include <Windows.h>

int InitMainMenuGraphics ();
int main_menu_draw (uint64_t ElapsedTime);
void DestroyMainMenuGraphics ();

int main_menu_process_keyboard_input (WPARAM wParam, LPARAM lParam);

void MainMenuProcessMouseMovement (uint32_t X, uint32_t Y, uint32_t DeltaX, uint32_t DeltaY);
