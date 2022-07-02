#include<renderer/window.h>

bool capturing_keyboard();
bool capturing_mouse();

void setup_ui(const Window* window, Renderer& renderer);

void update_ui(const Window& window, World& world, AssetHandler& assets);

bool ui_handle_input(HWND window, UINT msg, WPARAM wp, LPARAM lp);

void clean_up_ui();