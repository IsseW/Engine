#include<renderer/window.h>

void setup_ui(const Window* window, Renderer& renderer);

void update_ui(const Window* window, World& world);

bool ui_handle_input(HWND window, UINT msg, WPARAM wp, LPARAM lp);

void clean_up_ui();