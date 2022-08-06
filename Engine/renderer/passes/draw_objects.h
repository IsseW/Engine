#pragma once
#include<renderer/renderer.h>

void draw_objects(Renderer& renderer, const World& world, const AssetHandler& assets, FirstPass::Globals globals, bool pixel_shader, 
	const Vec<Id<Object>>& objects_to_draw, const Vec<Id<Reflective>>& reflectives_to_draw);