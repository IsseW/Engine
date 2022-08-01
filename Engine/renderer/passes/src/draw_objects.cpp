#include<renderer/passes/draw_objects.h>

void draw_objects(Renderer& rend, const World& world, const AssetHandler& assets, bool bind_material) {
	rend.ctx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	world.objects.values([&](const Object& obj) {
		ObjectData object_data = obj.get_data();
		rend.first_pass.object_renderer.object.update(rend.ctx.context, &object_data);

		const Mesh* mesh = assets.get_or_default(obj.mesh);
		if (mesh->binded.is_none()) {
			std::cerr << "Mesh needed for drawing isn't bound." << std::endl;
			return;
		}
		auto binded = mesh->binded.as_ptr().unwrap_unchecked();
		u32 stride = sizeof(Vertex);
		u32 offset = 0;
		rend.ctx.context->IASetVertexBuffers(0, 1, &binded->vertex_buffer, &stride, &offset);
		rend.ctx.context->IASetIndexBuffer(binded->index_buffer, DXGI_FORMAT_R16_UINT, 0);
		for (const SubMesh& sub_mesh : mesh->submeshes) {
			if (bind_material) {
				const Material* mat = assets.get_or_default(sub_mesh.material);
				auto bind_image = [&](u32 slot, Option<AId<Image>> image_handle) {
					const Image* image = assets.get_or_default(image_handle);
					image->binded.as_ptr().then_do([&](const Image::Binded* binded) {
						rend.ctx.context->PSSetShaderResources(slot, 1, &binded->srv);
						rend.ctx.context->PSSetSamplers(slot, 1, &binded->sampler_state);
					});
				};
				bind_image(0, mat->ambient.tex);
				bind_image(1, mat->diffuse.tex);
				bind_image(2, mat->specular.tex);
				bind_image(3, mat->shinyness.tex);

				MaterialData data = mat->get_data();
				rend.first_pass.object_renderer.material.update(rend.ctx.context, &data);
			}
			rend.ctx.context->DrawIndexed(sub_mesh.end_index - sub_mesh.start_index, sub_mesh.start_index, 0);
		}
		});
}