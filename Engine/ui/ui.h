#include<renderer/renderer.h>

void setup_ui(RendererCtx& ctx);

void update_ui(const Renderer& renderer);

bool handle_input(HWND window, UINT msg, WPARAM wp, LPARAM lp);

void clean_up_ui();