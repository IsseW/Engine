#pragma once
#include<renderer/renderer.h>

void draw_objects(Renderer& renderer, const World& world, const AssetHandler& assets, FirstPass::Globals globals, bool pixel_shader);